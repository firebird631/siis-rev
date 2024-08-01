/**
 * @brief SiiS strategy MAADX range-bar.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadxrb.h"
#include "maadxrbparameters.h"

#include "siis/config/strategyconfig.h"
#include "siis/handler.h"
#include "siis/connector/traderproxy.h"

#include "siis/connector/connector.h"
#include "siis/database/database.h"
#include "siis/database/tradedb.h"
#include "siis/database/rangebardb.h"

#include "maadxrbtrendanalyser.h"
#include "maadxrbsiganalyser.h"
#include "maadxrbconfanalyser.h"

using namespace siis;
using o3d::Logger;
using o3d::Debug;

extern "C"
{

SIIS_PLUGIN_API siis::Strategy* siisStrategy(Handler *handler, const o3d::String &identifier)
{
    return new siis::MaAdxRb(handler, identifier);
}

} // extern "C"

MaAdxRb::MaAdxRb(Handler *handler, const o3d::String &identifier) :
    Strategy(handler, identifier),
    m_trendAnalyser(nullptr),
    m_sigAnalyser(nullptr),
    m_confAnalyser(nullptr),
    m_lastSignal(0, 0),
    m_adxSig(40),
    m_targetScale(1.0),
    m_riskReward(1.0),
    m_minProfit(0.0)
{

}

MaAdxRb::~MaAdxRb()
{

}

void MaAdxRb::init(Config *config)
{
    Strategy::init(config);

    // author defined strategy properties details
    setProperty("name", "maadx-rb");
    setProperty("author", "Frederic SCHERMA (frederic.scherma@gmail.com)");
    setProperty("date", "2023-04-24");
    setProperty("revision", "1");
    setProperty("copyright", "2018-2024 Dream Overflow");
    setProperty("comment", "Crypto, indices and stocks big caps dedicaded strategy");

    // strategie parameters
    StrategyConfig conf;
    conf.parseDefaults(MaAdxRbParameters);
    conf.parseOverrides(config);

    initBasicsParameters(conf);

    if (conf.root().isMember("tickbars")) {
        Json::Value tickbars = conf.root().get("tickbars", Json::Value());
        for (auto it = tickbars.begin(); it != tickbars.end(); ++it) {
            Json::Value tickbar = *it;

            o3d::String name = it.name().c_str();
            o3d::String mode = tickbar.get("mode", "").asString().c_str();

            o3d::Int32 barSize = conf.barSizeAsInt(tickbar, "size");

            o3d::Int32 depth = tickbar.get("depth", 0).asInt();
            o3d::Int32 history = tickbar.get("history", 0).asInt();

            if (!tickbar.get("enabled", true).asBool()) {
                continue;
            }

            if (mode == "trend") {
                Analyser *a = new MaAdxRbTrendAnalyser(this, name, barSize, depth, history, Price::PRICE_HLC);
                a->init(AnalyserConfig(tickbar));

                m_analysers.push_back(a);
                m_trendAnalyser = static_cast<MaAdxRbTrendAnalyser*>(a);
            } else if (mode == "sig") {
                Analyser *a = new MaAdxRbSigAnalyser(this, name, barSize, depth, history, Price::PRICE_HLC);
                a->init(AnalyserConfig(tickbar));

                m_analysers.push_back(a);
                m_sigAnalyser = static_cast<MaAdxRbSigAnalyser*>(a);
            } else if (mode == "conf") {
                Analyser *a = new MaAdxRbConfAnalyser(this, name, barSize, depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(tickbar));

                m_analysers.push_back(a);
                m_confAnalyser = static_cast<MaAdxRbConfAnalyser*>(a);
            } else {
                // ignored, unknow mode
                O3D_WARNING(o3d::String("MaAdxRb strategy unknow mode {0}").arg(mode));
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

                m_adxSig = sig.get("min-adx", Json::Value()).asDouble();
            }

            // conf
            if (context.isMember("confirm")) {
                Json::Value confirm = context.get("confirm", Json::Value());

                m_targetScale = confirm.get("target-scale", Json::Value()).asDouble();
                m_riskReward = confirm.get("risk-reward", Json::Value()).asDouble();
            }

            // entry-exits
            m_entry.init(market(), ctxConfig);
            m_stopLoss.init(market(), ctxConfig);
            // m_takeProfit.init(market(), ctxConfig);
            m_breakeven.init(market(), ctxConfig);
            m_dynamicStopLoss.init(market(), ctxConfig);
            // m_dynamicTakeProfit.init(market(), ctxConfig);
        }
    }

    setInitialized();
}

void MaAdxRb::terminate(Connector *connector, Database *db)
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

void MaAdxRb::prepareMarketData(Connector *connector, Database *db, o3d::Double fromTs, o3d::Double toTs)
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

            o3d::String msg = o3d::String("Retrieved {0}/{1} OHLCs with most recent at {2}").arg(k).arg(depth)
                              .arg(timestampToStr(market()->getOhlcBuffer(ohlcType).get(lastN)->timestamp()));

            log(analyser->formatUnit(), "init", msg);

            analyser->onOhlcUpdate(toTs, 0.0, market()->getOhlcBuffer(ohlcType));
        } else {
            o3d::String msg = o3d::String("No OHLCs founds (0/{0})").arg(depth);
            log(analyser->formatUnit(), "init", msg);
        }
    }

    setMarketDataPrepared();
}

void MaAdxRb::finalizeMarketData(Connector *connector, Database *db)
{
    m_tradeManager = new StdTradeManager(this);
    setReady();
    setRunning();
}

void MaAdxRb::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    if (baseTimeframe() == TF_TICK) {
        for (Analyser *analyser : m_analysers) {
            analyser->onTickUpdate(timestamp, ticks);
        }
    }
}

void MaAdxRb::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, Ohlc::Type ohlcType, const OhlcArray &ohlc)
{
    /* not compatible */
}

void MaAdxRb::onOrderSignal(const OrderSignal &orderSignal)
{
    m_tradeManager->onOrderSignal(orderSignal);
}

void MaAdxRb::onPositionSignal(const PositionSignal &positionSignal)
{
    m_tradeManager->onPositionSignal(positionSignal);
}

void MaAdxRb::prepare(o3d::Double timestamp)
{
    // prepare before compute
    for (Analyser *analyser : m_analysers) {
        analyser->prepare(timestamp);
    }
}

void MaAdxRb::compute(o3d::Double timestamp)
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

//        if (signal.estimateTakeProfitRate() < m_minProfit) {
//            doOrder = false;
//        }

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

void MaAdxRb::finalize(o3d::Double timestamp)
{
    // cleanup eventually
}

void MaAdxRb::updateTrade(Trade *trade)
{
    if (trade) {
        m_breakeven.updateΤrade(handler()->timestamp(), lastTimestamp(), trade);
        m_dynamicStopLoss.updateΤrade(handler()->timestamp(), lastTimestamp(), trade);
    }
}

void MaAdxRb::updateStats()
{
    o3d::Double performance = 0.0;
    o3d::Double drawDownRate = 0.0;
    o3d::Double drawDown = 0.0;
    o3d::Int32 pending = 0;
    o3d::Int32 actives = 0;

    m_tradeManager->computePerformance(performance, drawDownRate, drawDown, pending, actives);

    setActiveStats(performance, drawDownRate, drawDown, pending, actives);
}

void MaAdxRb::orderEntry(
        o3d::Double timestamp,
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

void MaAdxRb::orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price)
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
        log(m_sigAnalyser->formatUnit()/*trade->tf()*/, "order-exit", msg);
    }
}

TradeSignal MaAdxRb::computeSignal(o3d::Double timestamp)
{
    TradeSignal signal(m_sigAnalyser->barSize(), timestamp);

    if (m_trendAnalyser->trend() > 0) {
        if (m_sigAnalyser->adx() > m_adxSig) {
            if (m_sigAnalyser->sig() > 0 && m_sigAnalyser->adx() <= ADX_MAX) {
                if (m_confAnalyser->confirmation() > 0) {
                    // keep only one signal per timeframe
                    if (m_lastSignal.timestamp() + m_lastSignal.timeframe() < timestamp) {
                        signal.setEntry();
                        signal.setLong();

                        m_entry.updateSignal(signal, market());
                        // signal.setPrice(m_confAnalyser->lastPrice());

                        signal.setTakeProfitPrice(m_sigAnalyser->takeProfit(m_targetScale));
                        signal.setStopLossPrice(m_sigAnalyser->stopLoss(m_targetScale, m_riskReward));
                    }
                }
            }
        }
    } else if (m_trendAnalyser->trend() < 0) {
        if (m_sigAnalyser->adx() > m_adxSig) {
            if (m_sigAnalyser->sig() < 0 && m_sigAnalyser->adx() <= ADX_MAX) {
                if (m_confAnalyser->confirmation() < 0) {
                    // keep only one signal per timeframe
                    if (m_lastSignal.timestamp() + m_lastSignal.timeframe() < timestamp) {
                        signal.setEntry();
                        signal.setShort();

                        m_entry.updateSignal(signal, market());
                        // signal.setPrice(m_confAnalyser->lastPrice());

                        signal.setTakeProfitPrice(m_sigAnalyser->takeProfit(m_targetScale));
                        signal.setStopLossPrice(m_sigAnalyser->stopLoss(m_targetScale, m_riskReward));
                    }
                }
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