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

#include "maadxrbprofileanalyser.h"
#include "maadxrbsessionanalyser.h"
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
    m_profileAnalyser(nullptr),
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

    if (conf.root().isMember("analysers")) {
        Json::Value analyser = conf.root().get("analysers", Json::Value());
        for (auto it = analyser.begin(); it != analyser.end(); ++it) {
            Json::Value analyser = *it;

            o3d::String name = it.name().c_str();
            o3d::String type = analyser.get("type", "").asString().c_str();
            o3d::String mode = analyser.get("mode", "").asString().c_str();

            o3d::Double timeframe = conf.timeframeAsDouble(analyser, "timeframe");
            o3d::Int32 barSize = conf.barSizeAsInt(analyser, "size");

            o3d::Int32 depth = analyser.get("depth", 0).asInt();
            o3d::Double history = conf.timeframeAsDouble(analyser, "history");

            if (!analyser.get("enabled", true).asBool()) {
                continue;
            }

            if (mode == "profile" && type == "timeframe") {
                Analyser *a = new MaAdxRbProfileAnalyser(this, name, timeframe, baseTimeframe(), depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(analyser));

                m_analysers.push_back(a);
                m_profileAnalyser = static_cast<MaAdxRbProfileAnalyser*>(a);
            } else if (mode == "session" && type == "range-bar") {
                Analyser *a = new MaAdxRbSessionAnalyser(this, name, barSize, depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(analyser));

                m_analysers.push_back(a);
                m_sessionAnalyser = static_cast<MaAdxRbSessionAnalyser*>(a);
            } else if (mode == "trend" && type == "range-bar") {
                Analyser *a = new MaAdxRbTrendAnalyser(this, name, barSize, depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(analyser));

                m_analysers.push_back(a);
                m_trendAnalyser = static_cast<MaAdxRbTrendAnalyser*>(a);
            } else if (mode == "sig" && type == "range-bar") {
                Analyser *a = new MaAdxRbSigAnalyser(this, name, barSize, depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(analyser));

                m_analysers.push_back(a);
                m_sigAnalyser = static_cast<MaAdxRbSigAnalyser*>(a);
            } else if (mode == "conf" && type == "range-bar") {
                Analyser *a = new MaAdxRbConfAnalyser(this, name, barSize, depth, history, Price::PRICE_CLOSE);
                a->init(AnalyserConfig(analyser));

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
            m_takeProfit.init(market(), ctxConfig);
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
        if (analyser->depth() <= 0) {
            continue;
        }

        o3d::Int32 k = 0;

        o3d::Double srcTs = 0.0;
        o3d::Double dstTs = fromTs - 1.0;
        o3d::Int32 lastN = 0;

        if (analyser->history() > 0) {
            srcTs = fromTs - 1.0 - analyser->history();
        } else if (analyser->timeframe() > 0) {
            srcTs = fromTs - 1.0 - analyser->timeframe() * analyser->depth();
        } else {
            lastN = analyser->depth();
        }

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

            o3d::String msg = o3d::String("Retrieved {0}/{1} OHLCs with most recent at {2}").arg(k)
                                  .arg(analyser->depth())
                                  .arg(timestampToStr(market()->getOhlcBuffer(ohlcType).get(lastN)->timestamp()));

            log(analyser->formatUnit(), "init", msg);

            analyser->onOhlcUpdate(toTs, 0.0, market()->getOhlcBuffer(ohlcType));
        } else {
            o3d::String msg = o3d::String("No OHLCs founds (0/{0})").arg(analyser->depth());
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

        if (m_trendAnalyser->price().consolidated()) {
            if (trade->direction() > 0) {
                if (m_trendAnalyser->price().open().last() < m_trendAnalyser->lastMaLow()) {
                    trade->close(TradeStats::REASON_CLOSE_MARKET);
                }
            } else if (trade->direction() < 0) {
                if (m_trendAnalyser->price().open().last() > m_trendAnalyser->lastMaHigh()) {
                    trade->close(TradeStats::REASON_CLOSE_MARKET);
                }
            }
        }
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

o3d::Bool MaAdxRb::checkVp(o3d::Int32 direction, o3d::Int32 vpUp, o3d::Int32 vpDn) const{
    if (m_sessionAnalyser == nullptr) {
        return true;
    }

    if (direction > 0 && m_confAnalyser->lastPrice() <= m_sessionAnalyser->vp().pocPrice()) {
        return false;
    } else if (direction < 0 && m_confAnalyser->lastPrice() >= m_sessionAnalyser->vp().pocPrice()) {
        return false;
    }

    if (direction > 0) {
        //            return vpUp > 0 && vpDn == 0;
        //            return vpUp > vpDn;
        return vpDn > 0 ? o3d::Float(vpUp) / o3d::Float(vpDn) >= 1.3 : vpUp > 0;
    } else if (direction < 0) {
        //            return vpDn > 0 && vpUp == 0;
        //            return vpUp < vpDn;
        return vpUp > 0 ? o3d::Float(vpDn) / o3d::Float(vpUp) >= 1.3 : vpDn > 0;
    }

    return false;
}

o3d::Bool MaAdxRb::checkVWap(o3d::Int32 direction) const
{
    if (m_trendAnalyser == nullptr) {
        return true;
    }

    return (direction == m_trendAnalyser->vwapTrend());
}

o3d::Bool MaAdxRb::checkCvd(o3d::Int32 direction) const
{
    if (m_sigAnalyser == nullptr) {
        return true;
    }

    return (direction == m_sigAnalyser->cvdTrend());
}

o3d::Bool MaAdxRb::checkTrend(o3d::Int32 direction, o3d::Int32 vpUp, o3d::Int32 vpDn) const
{
    // 0 initial
    // return m_trendAnalyser->trend() == direction;

    // 1 initial with VP
    return m_trendAnalyser->sig() == direction && checkVp(direction, vpUp, vpDn);

    // 2 interesting
    // return checkVp(direction, vpUp, vpDn) && checkVWap(direction);

    // 3 interesting
    // return checkVp(direction, vpUp, vpDn) && checkCvd(direction);
    // return m_sigAnalyser->trend() == direction && m_sessionAnalyser->vPocBreakout() == direction;

    // 4 very interesting
    // return checkVp(direction, vpUp, vpDn) && checkVWap(direction) && checkCvd(direction);

    // 5 inefficient
    // return m_sigAnalyser->sig() == direction && checkVp(direction, vpUp, vpDn) && checkVWap(direction);

    // 6 too strict
    // return m_sigAnalyser->sig() == direction && checkVp(direction, vpUp, vpDn) && checkCvd(direction);

    // 7 (full) too strict
    // return m_sigAnalyser->sig() == direction && checkVp(direction, vpUp, vpDn) && checkVWap(direction) && checkCvd(direction);
}

TradeSignal MaAdxRb::computeSignal(o3d::Double timestamp)
{
    TradeSignal signal(m_sigAnalyser->barSize(), timestamp);

    o3d::Int32 vpUp = 0;
    o3d::Int32 vpDn = 0;

    // volume-profile signal
    if (m_sessionAnalyser && m_sessionAnalyser->lastPrice()) {
        o3d::Double price = m_confAnalyser->lastPrice();

        for (auto const vp : m_sessionAnalyser->vp().vp()) {
            if (price > vp->pocPrice) {
                ++vpUp;
            } else if (price < vp->pocPrice) {
                ++vpDn;
            }
        }
    }

    if (checkTrend(1, vpUp, vpDn)/* && !filterMa()*/) {
        if (m_sigAnalyser->adx() > m_adxSig && m_sigAnalyser->adx() <= ADX_MAX) {
            if (1){//m_sigAnalyser->trend() > 0) {
                // keep only one signal per timeframe
                if (m_lastSignal.timestamp() + m_lastSignal.timeframe() < timestamp) {
                    signal.setEntry();
                    signal.setLong();

                    m_entry.updateSignal(signal, market());
                    // signal.setPrice(m_confAnalyser->lastPrice());

                    if (m_takeProfit.adjustPolicy() == ADJ_CUSTOM) {
                        signal.setTakeProfitPrice(m_sigAnalyser->takeProfit(m_targetScale));
                    } else {
                        m_takeProfit.updateSignal(signal);
                    }

                    if (m_stopLoss.adjustPolicy() == ADJ_CUSTOM) {
                        signal.setStopLossPrice(m_sigAnalyser->stopLoss(m_targetScale, m_riskReward));
                    } else {
                        m_stopLoss.updateSignal(signal);
                    }
                }
            }
        }
    } else if (checkTrend(-1, vpUp, vpDn)/* && !filterMa()*/) {
        if (m_sigAnalyser->adx() > m_adxSig && m_sigAnalyser->adx() <= ADX_MAX) {
            if (1){//m_sigAnalyser->trend() < 0) {
                // keep only one signal per timeframe
                if (m_lastSignal.timestamp() + m_lastSignal.timeframe() < timestamp) {
                    signal.setEntry();
                    signal.setShort();

                    m_entry.updateSignal(signal, market());
                    // signal.setPrice(m_confAnalyser->lastPrice());

                    if (m_takeProfit.adjustPolicy() == ADJ_CUSTOM) {
                        signal.setTakeProfitPrice(m_sigAnalyser->takeProfit(m_targetScale));
                    } else {
                        m_takeProfit.updateSignal(signal);
                    }

                    if (m_stopLoss.adjustPolicy() == ADJ_CUSTOM) {
                        signal.setStopLossPrice(m_sigAnalyser->stopLoss(m_targetScale, m_riskReward));
                    } else {
                        m_stopLoss.updateSignal(signal);
                    }
                }
            }
        }
    }

    if (signal.valid()) {
        // default stop-loss only if missing
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

o3d::Bool MaAdxRb::filterMa() const
{
    if (m_sigAnalyser->lastMaHigh() >= m_trendAnalyser->lastMaLow() && m_sigAnalyser->lastMaHigh() <= m_trendAnalyser->lastMaHigh()) {
        return true;
    }

    if (m_sigAnalyser->lastMaLow() >= m_trendAnalyser->lastMaLow() && m_sigAnalyser->lastMaLow() <= m_trendAnalyser->lastMaHigh()) {
        return true;
    }

    if (m_trendAnalyser->lastMaHigh() >= m_sigAnalyser->lastMaLow() && m_trendAnalyser->lastMaHigh() <= m_sigAnalyser->lastMaHigh()) {
        return true;
    }

    if (m_trendAnalyser->lastMaLow() >= m_sigAnalyser->lastMaLow() && m_trendAnalyser->lastMaLow() <= m_sigAnalyser->lastMaHigh()) {
        return true;
    }

    return false;
}
