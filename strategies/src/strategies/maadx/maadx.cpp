/**
 * @brief SiiS strategy MAADX.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadx.h"
#include "maadxparameters.h"

#include "siis/config/strategyconfig.h"
#include "siis/handler.h"
#include "siis/connector/traderproxy.h"

#include "siis/connector/connector.h"
#include "siis/database/database.h"
#include "siis/database/tradedb.h"
#include "siis/database/ohlcdb.h"

#include "maadxtrendanalyser.h"
#include "maadxsiganalyser.h"
#include "maadxconfanalyser.h"

using namespace siis;
using o3d::Logger;
using o3d::Debug;

extern "C"
{

SIIS_PLUGIN_API siis::Strategy* siisStrategy(Handler *handler, const o3d::String &identifier)
{
    return new siis::MaAdx(handler, identifier);
}

} // extern "C"

MaAdx::MaAdx(Handler *handler, const o3d::String &identifier) :
    Strategy(handler, identifier),
    m_trendAnalyser(nullptr),
    m_sigAnalyser(nullptr),
    m_confAnalyser(nullptr),
    m_lastSignal(0, 0),
    m_adxSig(40),
    m_targetScale(1.0),
    m_riskReward(1.0)
{

}

MaAdx::~MaAdx()
{

}

void MaAdx::init(Config *config)
{
    Strategy::init(config);

    // author defined strategy properties details
    setProperty("name", "maadx");
    setProperty("author", "Frederic SCHERMA (frederic.scherma@gmail.com)");
    setProperty("date", "2023-04-24");
    setProperty("revision", "1");
    setProperty("copyright", "2018-2023 Dream Overflow");
    setProperty("comment", "Crypto, indices and stocks big caps dedicaded strategy");

    // strategie parameters
    StrategyConfig conf;
    conf.parseDefaults(MaAdxParameters);
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
                Analyser *a = new MaAdxTrendAnalyser(this, tf, subTf, depth, history, Price::PRICE_HLC);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
                m_trendAnalyser = static_cast<MaAdxTrendAnalyser*>(a);
            } else if (mode == "sig") {
                Analyser *a = new MaAdxSigAnalyser(this, tf, subTf, depth, history, Price::PRICE_HLC);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
                m_sigAnalyser = static_cast<MaAdxSigAnalyser*>(a);
            } else if (mode == "conf") {
                Analyser *a = new MaAdxConfAnalyser(this, tf, subTf, depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
                m_confAnalyser = static_cast<MaAdxConfAnalyser*>(a);
            } else {
                // ignored, unknow mode
                O3D_WARNING(o3d::String("MaAdx strategy unknow mode {0}").arg(mode));
            }
        }
    }

    if (conf.root().isMember("contexts")) {
        Json::Value contexts = conf.root().get("contexts", Json::Value());
        for (auto it = contexts.begin(); it != contexts.end(); ++it) {
            Json::Value context = *it;

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

            // entry
            if (context.isMember("entry")) {
                Json::Value entry = context.get("entry", Json::Value());
                ContextConfig ctxConfig(entry);
                // m_entry.init(market(), ctxConfig);
            }

            // stop-loss
            if (context.isMember("stop-loss")) {
                Json::Value stopLoss = context.get("stop-loss", Json::Value());
                ContextConfig ctxConfig(stopLoss);
                // m_stopLoss.init(market(), ctxConfig);
            }

            // take-profit
            if (context.isMember("take-profit")) {
                Json::Value takeProfit = context.get("take-profit", Json::Value());
                ContextConfig ctxConfig(takeProfit);
                // m_takeProfit.init(market(), ctxConfig);
            }

            // breakeven
            if (context.isMember("breakeven")) {
                Json::Value breakeven = context.get("breakeven", Json::Value());
                ContextConfig ctxConfig(breakeven);
                m_breakeven.init(market(), ctxConfig);
            }

            // dynamic-stop-loss
            if (context.isMember("dynamic-stop-loss")) {
                Json::Value dynamicStopLoss = context.get("dynamic-stop-loss", Json::Value());
                ContextConfig ctxConfig(dynamicStopLoss);
                m_dynamicStopLoss.init(market(), ctxConfig);
            }

            // dynamic-take-profit
            if (context.isMember("dynamic-take-profit")) {
                Json::Value dynamicTakeProfit = context.get("dynamic-take-profit", Json::Value());
                ContextConfig ctxConfig(dynamicTakeProfit);
                // m_dynamicTakeProfit.init(market(), ctxConfig);
            }
        }
    }

    setInitialized();
}

void MaAdx::terminate(Connector *connector, Database *db)
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

void MaAdx::prepareMarketData(Connector *connector, Database *db, o3d::Double fromTs, o3d::Double toTs)
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

void MaAdx::finalizeMarketData(Connector *connector, Database *db)
{
    m_tradeManager = new StdTradeManager(this);
    setReady();
    setRunning();
}

void MaAdx::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    if (baseTimeframe() == TF_TICK) {
        for (Analyser *analyser : m_analysers) {
            analyser->onTickUpdate(timestamp, ticks);
        }
    }
}

void MaAdx::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, Ohlc::Type ohlcType, const OhlcArray &ohlc)
{
    if (baseTimeframe() == timeframe) {
        for (Analyser *analyser : m_analysers) {
            analyser->onOhlcUpdate(timestamp, ohlcType, ohlc);
        }
    }
}

void MaAdx::onOrderSignal(const OrderSignal &orderSignal)
{
    m_tradeManager->onOrderSignal(orderSignal);
}

void MaAdx::onPositionSignal(const PositionSignal &positionSignal)
{
    m_tradeManager->onPositionSignal(positionSignal);
}

void MaAdx::prepare(o3d::Double timestamp)
{
    // prepare before compute
    for (Analyser *analyser : m_analysers) {
        analyser->prepare(timestamp);
    }
}

void MaAdx::compute(o3d::Double timestamp)
{
    o3d::Bool sig = false;

    m_breakeven.update(handler()->timestamp(), lastTimestamp());
    m_dynamicStopLoss.update(handler()->timestamp(), lastTimestamp());

    for (Analyser *analyser : m_analysers) {
        sig |= analyser->process(timestamp, lastTimestamp());
    }

    // compute entries
    TradeSignal signal = computeSignal(timestamp);
    if (signal.type() == TradeSignal::ENTRY) {
        // keep to avoid repetitions
        m_lastSignal = signal;

        // second level : entry invalidation
        o3d::Bool doOrder = true;

        if (m_tradeManager->numTrades() >= maxTrades()) {
            doOrder = false;
        }

        if (doOrder) {
            orderEntry(timestamp, signal.tf(), signal.d(), signal.price(), signal.tp(), signal.sl());
        }
    }

    // update the existing trades
    m_tradeManager->process(timestamp);
}

void MaAdx::finalize(o3d::Double timestamp)
{
    // cleanup eventually
}

void MaAdx::updateTrade(Trade *trade)
{
    if (trade) {
        m_breakeven.updateΤrade(handler()->timestamp(), lastTimestamp(), trade);
        m_dynamicStopLoss.updateΤrade(handler()->timestamp(), lastTimestamp(), trade);
    }
}

void MaAdx::updateStats()
{
    o3d::Double performance = 0.0;
    o3d::Double drawDown = 0.0;
    o3d::Int32 pending = 0;
    o3d::Int32 actives = 0;

    m_tradeManager->computePerformance(performance, drawDown, pending, actives);

    setActiveStats(performance, drawDown, pending, actives);
}

void MaAdx::orderEntry(
        o3d::Double timestamp,
        o3d::Double timeframe,
        o3d::Int32 direction,
        o3d::Double price,
        o3d::Double takeProfitPrice,
        o3d::Double stopLossPrice)
{
    // do not take position over the two sides, else close before
    if (reversal() && m_tradeManager->hasTradesByDirection(-direction)) {
        m_tradeManager->closeAllByDirection(-direction);
    }

    Trade* trade = handler()->traderProxy()->createTrade(market(), tradeType(), timeframe);
    if (trade) {
        m_tradeManager->addTrade(trade);

        o3d::Double quantity = 1.0;  // @todo

        // query open
        trade->open(this, direction, 0.0, quantity, takeProfitPrice, stopLossPrice);

        o3d::String msg = o3d::String("#{0} {1} at {2} sl={3} tp={4} q={5}").arg(trade->id())
                          .arg(direction > 0 ? "long" : "short").arg(formatPrice(price))
                          .arg(formatPrice(stopLossPrice)).arg(formatPrice(takeProfitPrice))
                          .arg(quantity);
        log(timeframe, "order-entry", msg);
    }
}

void MaAdx::orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price)
{
    if (trade) {
        if (price > 0.0) {
            // if price defined, limit/stop close else market close
        } else {
            trade->close();

            // free trade once completed @todo done by tradeManager
            handler()->traderProxy()->freeTrade(trade);
        }

        o3d::String msg = o3d::String("#{0}").arg(trade->id());
        log(trade->tf(), "order-exit", msg);
    }
}

TradeSignal MaAdx::computeSignal(o3d::Double timestamp) const
{
    TradeSignal signal(m_sigAnalyser->timeframe(), timestamp);

    if (m_trendAnalyser->trend() > 0) {
        if (m_sigAnalyser->adx() > m_adxSig) {
            if (m_sigAnalyser->sig() > 0 && m_sigAnalyser->sig() <= ADX_MAX) {
                if (m_confAnalyser->confirmation() > 0) {
                    // keep only one signal per timeframe
                    if (m_lastSignal.timestamp() + m_lastSignal.timeframe() < timestamp) {
                        signal.setEntry();
                        signal.setLong();
                        signal.setPrice(m_confAnalyser->lastPrice());
                        signal.setTakeProfitPrice(m_sigAnalyser->takeProfit(m_targetScale));
                        signal.setStopLossPrice(m_sigAnalyser->stopLoss(m_targetScale, m_riskReward));
                    }
                }
            }
        }
    } else if (m_trendAnalyser->trend() < 0) {
        if (m_sigAnalyser->adx() > m_adxSig) {
            if (m_sigAnalyser->sig() < 0 && m_sigAnalyser->sig() <= ADX_MAX) {
                if (m_confAnalyser->confirmation() < 0) {
                    // keep only one signal per timeframe
                    if (m_lastSignal.timestamp() + m_lastSignal.timeframe() < timestamp) {
                        signal.setEntry();
                        signal.setShort();
                        signal.setPrice(m_confAnalyser->lastPrice());
                        signal.setTakeProfitPrice(m_sigAnalyser->takeProfit(m_targetScale));
                        signal.setStopLossPrice(m_sigAnalyser->stopLoss(m_targetScale, m_riskReward));
                    }
                }
            }
        }
    }

    return signal;
}
