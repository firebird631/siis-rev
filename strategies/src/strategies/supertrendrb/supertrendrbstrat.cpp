/**
 * @brief SiiS strategy SuperTrend range-bar.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#include "supertrendrbstrat.h"
#include "supertrendrbparameters.h"

#include "siis/config/strategyconfig.h"
#include "siis/handler.h"
#include "siis/connector/traderproxy.h"

#include "siis/connector/connector.h"
#include "siis/database/database.h"
#include "siis/database/tradedb.h"
#include "siis/database/rangebardb.h"

#include "supertrendrbtrendanalyser.h"
#include "supertrendrbsiganalyser.h"
#include "supertrendrbconfanalyser.h"

using namespace siis;
using o3d::Logger;
using o3d::Debug;

extern "C"
{

SIIS_PLUGIN_API siis::Strategy* siisStrategy(Handler *handler, const o3d::String &identifier)
{
    return new siis::SuperTrendRbStrat(handler, identifier);
}

} // extern "C"

SuperTrendRbStrat::SuperTrendRbStrat(Handler *handler, const o3d::String &identifier) :
    Strategy(handler, identifier),
    m_trendAnalyser(nullptr),
    m_sigAnalyser(nullptr),
    m_confAnalyser(nullptr),
    m_lastSignal(0, 0),
    m_profitScale(0.0),
    m_riskScale(0.0),
    m_minProfit(0.0)
{

}

SuperTrendRbStrat::~SuperTrendRbStrat()
{

}

void SuperTrendRbStrat::init(Config *config)
{
    Strategy::init(config);

    // author defined strategy properties details
    setProperty("name", "supertrend-rb");
    setProperty("author", "Frederic SCHERMA (frederic.scherma@gmail.com)");
    setProperty("date", "2023-09-30");
    setProperty("revision", "1");
    setProperty("copyright", "2018-2024 Dream Overflow");
    setProperty("comment", "Crypto, indices and stocks big caps dedicaded strategy");

    // strategie parameters
    StrategyConfig conf;
    conf.parseDefaults(SuperTrendRbParameters);
    conf.parseOverrides(config);

    initBasicsParameters(conf);

    if (conf.root().isMember("tickbars")) {
        Json::Value tickbars = conf.root().get("tickbars", Json::Value());
        for (auto it = tickbars.begin(); it != tickbars.end(); ++it) {
            Json::Value tickbar = *it;

            o3d::String name = it.name().c_str();
            o3d::String mode = tickbar.get("mode", "").asString().c_str();

            o3d::Double barSize = conf.barSizeAsInt(tickbar, "tickbar");

            o3d::Int32 depth = tickbar.get("depth", 0).asInt();
            o3d::Int32 history = tickbar.get("history", 0).asInt();

            if (!tickbar.get("enabled", true).asBool()) {
                continue;
            }

            if (mode == "trend") {
                Analyser *a = new SuperTrendRbTrendAnalyser(this, name, barSize, depth, history, Price::PRICE_HLC);
                a->init(AnalyserConfig(tickbar));

                m_analysers.push_back(a);
                m_trendAnalyser = static_cast<SuperTrendRbTrendAnalyser*>(a);
            } else if (mode == "sig") {
                Analyser *a = new SuperTrendRbSigAnalyser(this, name, barSize, depth, history, Price::PRICE_HLC);
                a->init(AnalyserConfig(tickbar));

                m_analysers.push_back(a);
                m_sigAnalyser = static_cast<SuperTrendRbSigAnalyser*>(a);
            } else if (mode == "conf") {
                Analyser *a = new SuperTrendRbConfAnalyser(this, name, barSize, depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(tickbar));

                m_analysers.push_back(a);
                m_confAnalyser = static_cast<SuperTrendRbConfAnalyser*>(a);
            } else {
                // ignored, unknow mode
                O3D_WARNING(o3d::String("SuperTrendRb strategy unknow mode {0}").arg(mode));
            }
        }
    }

    if (conf.root().isMember("contexts")) {
        Json::Value contexts = conf.root().get("contexts", Json::Value());
        for (auto it = contexts.begin(); it != contexts.end(); ++it) {
            Json::Value context = *it;
            ContextConfig ctxConfig(context);

            m_minProfit = context.get("min-profit", 0.0).asDouble() * 0.01;

            // trend
            if (context.isMember("trend")) {
                Json::Value trend = context.get("trend", Json::Value());
            }

            // sig
            if (context.isMember("sig")) {
                Json::Value sig = context.get("sig", Json::Value());
            }

            // conf
            if (context.isMember("confirm")) {
                Json::Value confirm = context.get("confirm", Json::Value());

                m_profitScale = confirm.get("profit-scale", Json::Value()).asDouble();
                m_riskScale = confirm.get("risk-scale", Json::Value()).asDouble();
            }

            // entry-exits
            m_entry.init(market(), ctxConfig);
            m_stopLoss.init(market(), ctxConfig);
            m_takeProfit.init(market(), ctxConfig);
            m_breakeven.init(market(), ctxConfig);
            m_dynamicStopLoss.init(market(), ctxConfig);
            // m_dynamicTakeProfit.init(market(), ctxConfig);
        }
    }

    setInitialized();
}

void SuperTrendRbStrat::terminate(Connector *connector, Database *db)
{
    for (Analyser *analyser : m_analysers) {
        analyser->terminate();
        o3d::deletePtr(analyser);
    }

    m_trendAnalyser = nullptr;
    m_sigAnalyser = nullptr;
    m_confAnalyser = nullptr;

    m_analysers.clear();

    if (m_tradeManager) {
        if (!handler()->isBacktesting()) {
            m_tradeManager->saveTrades(handler()->database()->trade());
        }

        m_tradeManager->terminate();
        o3d::deletePtr(m_tradeManager);
    }

    setTerminated();
}

void SuperTrendRbStrat::prepareMarketData(Connector *connector, Database *db, o3d::Double fromTs, o3d::Double toTs)
{
    Ohlc::Type ohlcType = Ohlc::TYPE_MID;

    for (Analyser *analyser : m_analysers) {
        // history might be >= depth but in case of...
        o3d::Int32 depth = o3d::max(analyser->history(), analyser->depth());
        if (depth <= 0) {
            continue;
        }

        o3d::Int32 k = 0;

        o3d::Double srcTs = 0.0;  //!< range-bar are not linear then source timestamp cannot be determined
        o3d::Double dstTs = fromTs - 1.0;
        o3d::Int32 lastN = depth;

        adjustOhlcFetchRange(analyser->history(), analyser->depth(), srcTs, dstTs, lastN);

        if (lastN > 0) {
            k = handler()->database()->rangeBar()->fetchOhlcArrayLastTo(
                    analyser->strategy()->brokerId(), market()->marketId(), analyser->barSize(),
                    lastN, dstTs,
                    market()->getOhlcBuffer(ohlcType));
        } else {
            k = handler()->database()->rangeBar()->fetchOhlcArrayFromTo(
                    analyser->strategy()->brokerId(), market()->marketId(), analyser->barSize(),
                    srcTs, dstTs,
                    market()->getOhlcBuffer(ohlcType));
        }

        if (k > 0) {
            o3d::Int32 lastN = market()->getOhlcBuffer(ohlcType).getSize() - 1;

            o3d::String msg = o3d::String("Retrieved {0}/{1} range-bars with most recent at {2}").arg(k).arg(depth)
                              .arg(timestampToStr(market()->getOhlcBuffer(ohlcType).get(lastN)->timestamp()));

            log(analyser->formatUnit(), "init", msg);

            analyser->onOhlcUpdate(toTs, 0.0, market()->getOhlcBuffer(ohlcType));
        } else {
            o3d::String msg = o3d::String("No range-bars founds (0/{0})").arg(depth);
            log(analyser->formatUnit(), "init", msg);
        }
    }

    setMarketDataPrepared();
}

void SuperTrendRbStrat::finalizeMarketData(Connector *connector, Database *db)
{
    m_tradeManager = new StdTradeManager(this);
    setReady();
    setRunning();
}

void SuperTrendRbStrat::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    if (baseTimeframe() == TF_TICK) {
        for (Analyser *analyser : m_analysers) {
            analyser->onTickUpdate(timestamp, ticks);
        }
    }
}

void SuperTrendRbStrat::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, Ohlc::Type ohlcType, const OhlcArray &ohlc)
{
    /* not compatible */
}

void SuperTrendRbStrat::onOrderSignal(const OrderSignal &orderSignal)
{
    m_tradeManager->onOrderSignal(orderSignal);
}

void SuperTrendRbStrat::onPositionSignal(const PositionSignal &positionSignal)
{
    m_tradeManager->onPositionSignal(positionSignal);
}

void SuperTrendRbStrat::prepare(o3d::Double timestamp)
{
    // prepare before compute
    for (Analyser *analyser : m_analysers) {
        analyser->prepare(timestamp);
    }
}

void SuperTrendRbStrat::compute(o3d::Double timestamp)
{
    for (Analyser *analyser : m_analysers) {
        analyser->process(timestamp, lastTimestamp());
    }

    m_breakeven.update(handler()->timestamp(), lastTimestamp());
    m_dynamicStopLoss.update(handler()->timestamp(), lastTimestamp());

    o3d::Int32 numClosed = 0;
    TradeSignal signal = computeSignal(timestamp);
    if (signal.type() == TradeSignal::ENTRY) {
        // do not take position over the two sides, else close before
        if (reversal() && m_tradeManager->hasTradesByDirection(-signal.direction())) {
            numClosed = m_tradeManager->closeAllByDirection(-signal.direction());
        }
    }

    // update the existing trades
    m_tradeManager->process(timestamp);

    if (signal.type() == TradeSignal::ENTRY) {
        // keep to avoid repetitions
        m_lastSignal = signal;

        o3d::Bool doOrder = true;

        if (signal.direction() < 0 && !allowShort()) {
            doOrder = false;
        }

        if (signal.estimateTakeProfitRate() < m_minProfit) {
            doOrder = false;
        }

        if (m_tradeManager->numTrades() >= maxTrades() && numClosed < 1) {
            doOrder = false;
        }

        if (hasTradingSessions()) {
            if (!allowedTradingSession(timestamp)) {
                doOrder = false;
            }
        }

        if (doOrder) {
            orderEntry(timestamp, signal.tf(), signal.d(), signal.orderType(), signal.price(), signal.tp(), signal.sl());
        }
    }
}

void SuperTrendRbStrat::finalize(o3d::Double timestamp)
{
    // cleanup eventually
}

void SuperTrendRbStrat::updateTrade(Trade *trade)
{
    if (trade) {
        // m_breakeven.updateΤrade(handler()->timestamp(), lastTimestamp(), trade);
        // m_dynamicStopLoss.updateΤrade(handler()->timestamp(), lastTimestamp(), trade);

        // custom dynamic stop-loss
        if (m_dynamicStopLoss.priceType() == PRICE_CUSTOM) {
            // last stop must be at minimal defined distance
            if (!m_dynamicStopLoss.checkMinDistance(trade)) {
                return;
            }

            // @todo could update once consolidated

            o3d::Double newStopPrice = m_sigAnalyser->dynamicStopLoss(
                trade->direction(), trade->stopLossPrice(), m_dynamicStopLoss.distance());

            if (newStopPrice > 0.0) {
                newStopPrice = market()->adjustPrice(newStopPrice);

                if (newStopPrice != trade->stopLossPrice()) {
                    trade->modifyStopLoss(newStopPrice, Trade::MOD_PREVIOUS);
                }
            }
        }
    }
}

void SuperTrendRbStrat::updateStats()
{
    o3d::Double performance = 0.0;
    o3d::Double drawDownRate = 0.0;
    o3d::Double drawDown = 0.0;
    o3d::Int32 pending = 0;
    o3d::Int32 actives = 0;

    m_tradeManager->computePerformance(performance, drawDownRate, drawDown, pending, actives);

    setActiveStats(performance, drawDownRate, drawDown, pending, actives);
}

void SuperTrendRbStrat::orderEntry(o3d::Double timestamp,
        o3d::Int32 barSize,
        o3d::Int32 direction,
        Order::OrderType orderType,
        o3d::Double price,
        o3d::Double takeProfitPrice,
        o3d::Double stopLossPrice)
{
    Trade* trade = handler()->traderProxy()->createTrade(market(), tradeType(), barSize);
    if (trade) {
        m_tradeManager->addTrade(trade);

        o3d::Double quantity = baseQuantity();

        // query open
        trade->open(this, direction, orderType, price, quantity, takeProfitPrice, stopLossPrice);

        o3d::String msg = o3d::String("#{0} {1} at {2} sl={3} tp={4} q={5} {6}%/{7}%")
                          .arg(trade->id())
                          .arg(direction > 0 ? "LONG" : "SHORT")
                          .arg(market()->formatPrice(price))
                          .arg(market()->formatPrice(stopLossPrice))
                          .arg(market()->formatPrice(takeProfitPrice))
                          .arg(market()->formatQty(quantity))
                          .arg(trade->estimateTakeProfitRate() * 100, 2)
                          .arg(trade->estimateStopLossRate() * 100, 2);
        log(m_sigAnalyser->formatUnit(), "trade-entry", msg);
    }
}

void SuperTrendRbStrat::orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price)
{
    if (trade) {
        if (price > 0.0) {
            // if price defined, limit/stop close else market close
        } else {
            trade->close(TradeStats::REASON_CLOSE_MARKET);

            // free trade once completed @todo done by tradeManager
            handler()->traderProxy()->freeTrade(trade);
        }

        o3d::String msg = o3d::String("#{0}").arg(trade->id());
        log(m_sigAnalyser->formatUnit(), "order-exit", msg);
    }
}

TradeSignal SuperTrendRbStrat::computeSignal(o3d::Double timestamp)
{
    TradeSignal signal(m_sigAnalyser->barSize(), timestamp);

    o3d::Int32 sig = m_sigAnalyser->sig();

    if (sig != 0) {
        // printf("%i\n", sig);
    }

    if (sig > 0) {
        if (1){//m_confAnalyser->confirmation() > 0) {
            // keep only one signal per minute
            if (timestamp - m_lastSignal.timestamp() >= 60.0 ) {
                signal.setEntry();
                signal.setLong();

                m_entry.updateSignal(signal, market());
                // signal.setPrice(m_confAnalyser->lastPrice());

                signal.setTakeProfitPrice(signal.price() * (1.0 + m_takeProfit.distance()) + m_profitScale * market()->onePipMean());
                signal.setStopLossPrice(m_sigAnalyser->stopLoss(1, m_riskScale, market()->onePipMean()));
            }
        }
    } else if (sig < 0) {
        if (1){//m_confAnalyser->confirmation() < 0) {
            // keep only one signal per timeframe
            if (timestamp - m_lastSignal.timestamp() >= 60.0) {
                signal.setEntry();
                signal.setShort();

                m_entry.updateSignal(signal, market());
                // signal.setPrice(m_confAnalyser->lastPrice());

                signal.setTakeProfitPrice(signal.price() * (1.0 - m_takeProfit.distance()) - m_profitScale * market()->onePipMean());
                signal.setStopLossPrice(m_sigAnalyser->stopLoss(-1, m_riskScale, market()->onePipMean()));
            }
        }
    }

    if (signal.valid()) {
        m_stopLoss.updateSignal(signal);

        signal.setEntryTimeout(m_entry.timeout());

        if (!m_entry.checkMaxSpread(market())) {
            // cancel signal because of non typical spread
            signal.reset();
        }

        if (m_minProfit > 0.0 && signal.estimateTakeProfitRate() < m_minProfit) {
            // not enought profit
            signal.reset();
        }
    }

    return signal;
}
