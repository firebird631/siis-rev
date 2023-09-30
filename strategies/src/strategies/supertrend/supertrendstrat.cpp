/**
 * @brief SiiS strategy SuperTrend.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#include "supertrendstrat.h"
#include "supertrendparameters.h"

#include "siis/config/strategyconfig.h"
#include "siis/handler.h"
#include "siis/connector/traderproxy.h"

#include "siis/connector/connector.h"
#include "siis/database/database.h"
#include "siis/database/tradedb.h"
#include "siis/database/ohlcdb.h"

#include "supertrendtrendanalyser.h"
#include "supertrendsiganalyser.h"
#include "supertrendconfanalyser.h"

using namespace siis;
using o3d::Logger;
using o3d::Debug;

extern "C"
{

SIIS_PLUGIN_API siis::Strategy* siisStrategy(Handler *handler, const o3d::String &identifier)
{
    return new siis::SuperTrendStrat(handler, identifier);
}

} // extern "C"

SuperTrendStrat::SuperTrendStrat(Handler *handler, const o3d::String &identifier) :
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

SuperTrendStrat::~SuperTrendStrat()
{

}

void SuperTrendStrat::init(Config *config)
{
    Strategy::init(config);

    // author defined strategy properties details
    setProperty("name", "supertrend");
    setProperty("author", "Frederic SCHERMA (frederic.scherma@gmail.com)");
    setProperty("date", "2023-09-30");
    setProperty("revision", "1");
    setProperty("copyright", "2018-2023 Dream Overflow");
    setProperty("comment", "Crypto, indices and stocks big caps dedicaded strategy");

    // strategie parameters
    StrategyConfig conf;
    conf.parseDefaults(SuperTrendParameters);
    conf.parseOverrides(config);

    initBasicsParameters(conf);

    if (conf.root().isMember("timeframes")) {
        Json::Value timeframes = conf.root().get("timeframes", Json::Value());
        for (auto it = timeframes.begin(); it != timeframes.end(); ++it) {
            Json::Value timeframe = *it;

            o3d::String mode = timeframe.get("mode", "").asString().c_str();

            o3d::Double tf = conf.timeframeAsDouble(timeframe, "timeframe");
            o3d::Double subTf = conf.timeframeAsDouble(timeframe, "sub-timeframe");

            o3d::Int32 depth = timeframe.get("depth", 0).asInt();
            o3d::Int32 history = timeframe.get("history", 0).asInt();

            if (!timeframe.get("enabled", true).asBool()) {
                continue;
            }

            if (mode == "trend") {
                Analyser *a = new SuperTrendTrendAnalyser(this, tf, subTf, depth, history, Price::PRICE_HLC);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
                m_trendAnalyser = static_cast<SuperTrendTrendAnalyser*>(a);
            } else if (mode == "sig") {
                Analyser *a = new SuperTrendSigAnalyser(this, tf, subTf, depth, history, Price::PRICE_HLC);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
                m_sigAnalyser = static_cast<SuperTrendSigAnalyser*>(a);
            } else if (mode == "conf") {
                Analyser *a = new SuperTrendConfAnalyser(this, tf, subTf, depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
                m_confAnalyser = static_cast<SuperTrendConfAnalyser*>(a);
            } else {
                // ignored, unknow mode
                O3D_WARNING(o3d::String("SuperTrend strategy unknow mode {0}").arg(mode));
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

void SuperTrendStrat::terminate(Connector *connector, Database *db)
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

void SuperTrendStrat::prepareMarketData(Connector *connector, Database *db, o3d::Double fromTs, o3d::Double toTs)
{
    Ohlc::Type ohlcType = Ohlc::TYPE_MID;

    for (Analyser *analyser : m_analysers) {
        // history might be >= depth but in case of...
        o3d::Int32 depth = o3d::max(analyser->history(), analyser->depth());
        if (depth <= 0) {
            continue;
        }

        o3d::Int32 k = 0;

        o3d::Double srcTs = fromTs - 1.0 - analyser->timeframe() * depth;
        o3d::Double dstTs = fromTs - 1.0;
        o3d::Int32 lastN = 0;

        adjustOhlcFetchRange(depth, srcTs, dstTs, lastN);

        if (lastN > 0) {
            k = handler()->database()->ohlc()->fetchOhlcArrayLastTo(
                    analyser->strategy()->brokerId(), market()->marketId(), analyser->timeframe(),
                    lastN, dstTs,
                    market()->getOhlcBuffer(ohlcType));
        } else {
            k = handler()->database()->ohlc()->fetchOhlcArrayFromTo(
                    analyser->strategy()->brokerId(), market()->marketId(), analyser->timeframe(),
                    srcTs, dstTs,
                    market()->getOhlcBuffer(ohlcType));
        }

        if (k > 0) {
            o3d::Int32 lastN = market()->getOhlcBuffer(ohlcType).getSize() - 1;

            o3d::String msg = o3d::String("Retrieved {0}/{1} OHLCs with most recent at {2}").arg(k).arg(depth)
                              .arg(timestampToStr(market()->getOhlcBuffer(ohlcType).get(lastN)->timestamp()));

            log(analyser->timeframe(), "init", msg);

            analyser->onOhlcUpdate(toTs, analyser->timeframe(), market()->getOhlcBuffer(ohlcType));
        } else {
            o3d::String msg = o3d::String("No OHLCs founds (0/{0})").arg(depth);
            log(analyser->timeframe(), "init", msg);
        }
    }

    setMarketDataPrepared();
}

void SuperTrendStrat::finalizeMarketData(Connector *connector, Database *db)
{
    m_tradeManager = new StdTradeManager(this);
    setReady();
    setRunning();
}

void SuperTrendStrat::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    if (baseTimeframe() == TF_TICK) {
        for (Analyser *analyser : m_analysers) {
            analyser->onTickUpdate(timestamp, ticks);
        }
    }
}

void SuperTrendStrat::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, Ohlc::Type ohlcType, const OhlcArray &ohlc)
{
    if (baseTimeframe() == timeframe) {
        for (Analyser *analyser : m_analysers) {
            analyser->onOhlcUpdate(timestamp, ohlcType, ohlc);
        }
    }
}

void SuperTrendStrat::onOrderSignal(const OrderSignal &orderSignal)
{
    m_tradeManager->onOrderSignal(orderSignal);
}

void SuperTrendStrat::onPositionSignal(const PositionSignal &positionSignal)
{
    m_tradeManager->onPositionSignal(positionSignal);
}

void SuperTrendStrat::prepare(o3d::Double timestamp)
{
    // prepare before compute
    for (Analyser *analyser : m_analysers) {
        analyser->prepare(timestamp);
    }
}

void SuperTrendStrat::compute(o3d::Double timestamp)
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

        if (doOrder) {
            orderEntry(timestamp, signal.tf(), signal.d(), signal.orderType(), signal.price(), signal.tp(), signal.sl());
        }
    }
}

void SuperTrendStrat::finalize(o3d::Double timestamp)
{
    // cleanup eventually
}

void SuperTrendStrat::updateTrade(Trade *trade)
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

void SuperTrendStrat::updateStats()
{
    o3d::Double performance = 0.0;
    o3d::Double drawDown = 0.0;
    o3d::Int32 pending = 0;
    o3d::Int32 actives = 0;

    m_tradeManager->computePerformance(performance, drawDown, pending, actives);

    setActiveStats(performance, drawDown, pending, actives);
}

void SuperTrendStrat::orderEntry(
        o3d::Double timestamp,
        o3d::Double timeframe,
        o3d::Int32 direction,
        Order::OrderType orderType,
        o3d::Double price,
        o3d::Double takeProfitPrice,
        o3d::Double stopLossPrice)
{
    Trade* trade = handler()->traderProxy()->createTrade(market(), tradeType(), timeframe);
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
        log(timeframe, "trade-entry", msg);
    }
}

void SuperTrendStrat::orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price)
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
        log(trade->tf(), "order-exit", msg);
    }
}

TradeSignal SuperTrendStrat::computeSignal(o3d::Double timestamp)
{
    TradeSignal signal(m_sigAnalyser->timeframe(), timestamp);

    o3d::Int32 sig = m_sigAnalyser->sig();

    if (sig != 0) {
        // printf("%i\n", sig);
    }

    if (sig > 0) {
        if (1){//m_confAnalyser->confirmation() > 0) {
            // keep only one signal per timeframe
            if (m_lastSignal.timestamp() + m_lastSignal.timeframe() < timestamp) {
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
            if (m_lastSignal.timestamp() + m_lastSignal.timeframe() < timestamp) {
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

        if (m_minProfit > 0.0 && signal.estimateTakeProfitRate() < m_minProfit) {
            signal.reset();
        }
    }

    return signal;
}
