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

            if (!timeframe.get("enabled", false).asBool()) {
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
            // @todo

            // stop-loss

            // take-profit

            // breakeven

            // dynamic-stop-loss

            // dynamic-take-profit
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
        // m_tradeManager->saveTrades(tradeDb);
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
        o3d::Int32 n = o3d::max(analyser->history(), analyser->depth());
        if (n <= 0) {
            continue;
        }

//        o3d::Int32 k = handler()->database()->ohlc()->fetchOhlcArrayLastTo(
//                           analyser->strategy()->brokerId(), market()->marketId(), analyser->timeframe(), n, fromTs-1.0,
//                           market()->getOhlcBuffer(ohlcType));

        o3d::Double baseTs = fromTs - 1.0 - analyser->timeframe() * n;

        o3d::Int32 k = handler()->database()->ohlc()->fetchOhlcArrayFromTo(
                           analyser->strategy()->brokerId(), market()->marketId(), analyser->timeframe(), baseTs, fromTs-1.0,
                           market()->getOhlcBuffer(ohlcType));

        if (k > 0) {
            o3d::Int32 lastN = market()->getOhlcBuffer(ohlcType).getSize() - 1;

            o3d::String msg = o3d::String("Retrieved {0}/{1} OHLCs with most recent at {2}").arg(k).arg(n)
                              .arg(timestampToStr(market()->getOhlcBuffer(ohlcType).get(lastN)->timestamp()));

            log(analyser->timeframe(), "init", msg);

            analyser->onOhlcUpdate(toTs, analyser->timeframe(), market()->getOhlcBuffer(ohlcType));

            //        for (int i = 0; i < market()->getOhlcBuffer(ohlcType).getSize(); ++i) {
            //            o3d::System::print(market()->getOhlcBuffer(ohlcType)[i].toString(), o3d::String("{0}").arg(i));
            //        }
        } else {
            o3d::String msg = o3d::String("No OHLCs founds (0/{0})").arg(n);
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

    for (Analyser *analyser : m_analysers) {
        sig |= analyser->process(timestamp, lastTimestamp());
    }

    // compute entries
    TradeSignal signal = compteSignal(timestamp);
    if (signal.type() == TradeSignal::ENTRY) {
        // keep to avoid repetitions
        m_lastSignal = signal;

        o3d::Bool doOrder = true;
        // second level : entry invalidation
        // @todo

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
    // cleanup
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
        log(timeframe, "trade-entry", msg);
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
        log(trade->tf(), "trade-exit", msg);
    }
}

TradeSignal MaAdx::compteSignal(o3d::Double timestamp) const
{
    TradeSignal signal(m_sigAnalyser->timeframe(), timestamp);

    if (m_trendAnalyser->trend() > 0) {
        if (m_sigAnalyser->adx() > 25) {
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
        if (m_sigAnalyser->adx() > 25) {
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
