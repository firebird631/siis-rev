﻿/**
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
#include "maadxsessionanalyser.h"

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
    m_sessionAnalyser(nullptr),
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

            o3d::String name = it.name().c_str();
            o3d::String mode = timeframe.get("mode", "").asString().c_str();

            o3d::Double tf = conf.timeframeAsDouble(timeframe, "timeframe");

            o3d::Int32 depth = timeframe.get("depth", 0).asInt();
            o3d::Int32 history = timeframe.get("history", 0).asInt();

            if (!timeframe.get("enabled", true).asBool()) {
                continue;
            }

            if (mode == "session") {
                Analyser *a = new MaAdxSessionAnalyser(this, name, tf, baseTimeframe(), depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
                m_sessionAnalyser = static_cast<MaAdxSessionAnalyser*>(a);
            } else if (mode == "trend") {
                Analyser *a = new MaAdxTrendAnalyser(this, name, tf, baseTimeframe(), depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
                m_trendAnalyser = static_cast<MaAdxTrendAnalyser*>(a);
            } else if (mode == "sig") {
                Analyser *a = new MaAdxSigAnalyser(this, name, tf, baseTimeframe(), depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
                m_sigAnalyser = static_cast<MaAdxSigAnalyser*>(a);
            } else if (mode == "conf") {
                Analyser *a = new MaAdxConfAnalyser(this, name, tf, baseTimeframe(), depth, history, Price::PRICE_CLOSE);
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
            ContextConfig ctxConfig(context);

            m_minProfit = context.get("min-profit", 0.0).asDouble() * 0.01;

            // session
            if (context.isMember("session")) {
                Json::Value session = context.get("session", Json::Value());
            }

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

        adjustOhlcFetchRange(analyser->history(), analyser->depth(), srcTs, dstTs, lastN);

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

            log(analyser->formatUnit(), "init", msg);

            analyser->onOhlcUpdate(toTs, analyser->timeframe(), market()->getOhlcBuffer(ohlcType));
        } else {
            o3d::String msg = o3d::String("No OHLCs founds (0/{0})").arg(depth);
            log(analyser->formatUnit(), "init", msg);
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
    o3d::Double drawDownRate = 0.0;
    o3d::Double drawDown = 0.0;
    o3d::Int32 pending = 0;
    o3d::Int32 actives = 0;

    m_tradeManager->computePerformance(performance, drawDownRate, drawDown, pending, actives);

    setActiveStats(performance, drawDownRate, drawDown, pending, actives);
}

void MaAdx::orderEntry(
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
        log(m_sigAnalyser->formatUnit(), "trade-entry", msg);
    }
}

void MaAdx::orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price)
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

o3d::Bool MaAdx::checkVp(o3d::Int32 direction, o3d::Int32 vpUp, o3d::Int32 vpDn) const{
    if (direction > 0 && m_confAnalyser->lastPrice() <= m_sessionAnalyser->vp().pocPrice()) {
        return false;
    } else if (direction < 0 && m_confAnalyser->lastPrice() >= m_sessionAnalyser->vp().pocPrice()) {
        return true;
    }

    if (m_sessionAnalyser) {
        if (direction > 0) {
            return vpUp > vpDn;
        } else if (direction < 0) {
            return vpUp < vpDn;
        }
    }

    return true;
}

TradeSignal MaAdx::computeSignal(o3d::Double timestamp)
{
    TradeSignal signal(m_sigAnalyser->timeframe(), timestamp);

    o3d::Int32 vpUp = 0;
    o3d::Int32 vpDn = 0;

    // volume-profile signal
    if (m_sessionAnalyser && m_sessionAnalyser->lastPrice()) {
        o3d::Double price = m_sessionAnalyser->lastPrice();
        // o3d::Int32 n = -m_sessionAnalyser->vp().vp().size();

        for (auto const vp : m_sessionAnalyser->vp().vp()) {
            // price = m_sessionAnalyser->price().close().last();
            // ++n;

            if (price > vp->pocPrice) {
                ++vpUp;
            } else if (price < vp->pocPrice) {
                ++vpDn;
            }
        }
    }

    if (m_trendAnalyser->trend() > 0 && checkVp(1, vpUp, vpDn)) {
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
    } else if (m_trendAnalyser->trend() < 0 && checkVp(-1, vpUp, vpDn)) {
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
