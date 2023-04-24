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

#include "siis/connector/connector.h"
#include "siis/database/database.h"
#include "siis/database/tradedb.h"

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
    Strategy(handler, identifier)
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

    m_reversal = conf.root().get("reversal", true).asBool();
    m_pyramided = conf.root().get("pyramided", 0).asInt();
    m_hedging = conf.root().get("hedging", false).asBool();
    m_maxTrades = conf.root().get("max-trades", 1).asInt();
    m_tradeDelay = conf.root().get("trade-delay", 30).asDouble();
    m_needUpdate = conf.root().get("need-update", false).asBool();

    m_baseTimeframe = conf.baseTimeframe();

    // stream data sources
    if (m_baseTimeframe <= 0.0) {
        addTickDataSource();
    } else {
        addMidOhlcDataSource(m_baseTimeframe);
    }

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

            if (mode == "A" || mode == "trend") {
                Analyser *a = new MaAdxTrendAnalyser(this, tf, subTf, depth, history);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
            } else if (mode == "B" || mode == "sig") {
                Analyser *a = new MaAdxSigAnalyser(this, tf, subTf, depth, history);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
            } else if (mode == "C" || mode == "conf") {
                Analyser *a = new MaAdxConfAnalyser(this, tf, subTf, depth, history);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
            } else {
                // ignored, unknow mode
                O3D_WARNING(o3d::String("MaAdx strategy unknow mode {0}").arg(mode));
            }
        }
    }

    if (conf.root().isMember("contexts")) {
        // @todo
    }

    setInitialized();
}

void MaAdx::terminate(Connector *connector, Database *db)
{
    for (Analyser *analyser : m_analysers) {
        analyser->terminate();
        o3d::deletePtr(analyser);
    }
    m_analysers.clear();

    if (m_tradeManager) {
        // m_tradeManager->saveTrades(tradeDb);
        o3d::deletePtr(m_tradeManager);
    }

    setTerminated();
}

void MaAdx::prepareMarketData(Connector *connector, Database *db)
{
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
    if (m_baseTimeframe == TF_TICK) {
        for (Analyser *analyser : m_analysers) {
            analyser->onTickUpdate(timestamp, ticks);
        }
    }
}

void MaAdx::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, Ohlc::Type ohlcType, const OhlcArray &ohlc)
{
    if (m_baseTimeframe == timeframe) {
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

    // update the existing trades
    m_tradeManager->process(timestamp);

    if (sig) {
        Trade *trade = nullptr;
        o3d::Double maxExitTf = 0;

        // one or more potential entries or exits signals
        for (Analyser *analyser : m_analysers) {
            if (analyser->lastSignal().type() == TradeSignal::ENTRY) {
                // entry signal
                const TradeSignal &signal = analyser->lastSignal();
                o3d::Bool doOrder = true;

                // second level : entry invalidation
                // ...

                if (doOrder) {
                    // @todo problem want only in live mode, not during backtesting
                    o3d::Double height = 0.0;  // market().height(entry.timeframe, -1)

                    // @todo or trade at order book, compute the limit price from what the order book offer or could use ATR
                    o3d::Double signalPrice = signal.price() + height;

                    orderEntry(timestamp, signal.tf(), signal.d(), signalPrice, signal.tp(), signal.sl());
                }
            } else if (analyser->lastSignal().type() == TradeSignal::EXIT) {
                // exit signal
                const TradeSignal &signal = analyser->lastSignal();

                // process it on the related timeframe if we have a trade on
                trade = m_tradeManager->findTrade(signal.timeframe());
                if (trade) {
                    // o3d::Double height = 0.0;  // market().height(entry.timeframe, -1)
                    // o3d::Double signalPrice = signal.price() + height;

                    // market exit @todo look for at least a limit
                    orderExit(timestamp, trade, signal.price());
                }
            }
        }
    }
}

void MaAdx::finalize(o3d::Double timestamp)
{
    // cleanup
}

void MaAdx::orderEntry(
        o3d::Double timestamp,
        o3d::Double timeframe,
        o3d::Int32 direction,
        o3d::Double price,
        o3d::Double limitPrice,
        o3d::Double stopPrice)
{
    log(timeframe, "content", "entry");
}

void MaAdx::orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price)
{
    log(trade->tf(), "content", "exit");
}
