/**
 * @brief SiiS strategy indicealpha.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#include "indicealpha.h"
#include "iaparameters.h"

#include "siis/config/strategyconfig.h"

#include "siis/handler.h"

#include "siis/connector/connector.h"
#include "siis/connector/traderproxy.h"

#include "siis/database/database.h"
#include "siis/database/tradedb.h"

#include "iaaanalyser.h"
#include "iabanalyser.h"
#include "iacanalyser.h"
#include "iadanalyser.h"

using namespace siis;
using o3d::Logger;
using o3d::Debug;

extern "C"
{

SIIS_PLUGIN_API siis::Strategy* siisStrategy(Handler *handler, const o3d::String &identifier)
{
    return new siis::IndiceAlpha(handler, identifier);
}

} // extern "C"

IndiceAlpha::IndiceAlpha(Handler *handler, const o3d::String &identifier) :
    Strategy(handler, identifier)
{

}

IndiceAlpha::~IndiceAlpha()
{

}

void IndiceAlpha::init(Config *config)
{
    Strategy::init(config);

    // author defined strategy properties details
    setProperty("name", "indicealpha");
    setProperty("author", "Frederic SCHERMA (frederic.scherma@gmail.com)");
    setProperty("date", "2019-06-03");
    setProperty("revision", "1");
    setProperty("copyright", "2018-2019 Dream Overflow");
    setProperty("comment", "Indice (DAX,SPX,DJI) dedicaded strategy");

    // strategie parameters
    StrategyConfig conf;
    conf.parseDefaults(IndiceAlphaParameters);
    conf.parseOverrides(config->getStrategiesPath(), config->getStrategyFilename());

    m_reversal = conf.root().get("reversal", true).asBool();
    m_pyramided = conf.root().get("pyramided", 0).asInt();
    m_hedging = conf.root().get("hedging", false).asBool();
    m_maxTrades = conf.root().get("maxTrades", 1).asInt();
    m_tradeDelay = conf.root().get("tradeDelay", 30).asDouble();
    m_needUpdate = conf.root().get("needUpdate", false).asBool();
    m_minVol24h = conf.root().get("minVol24h", 0).asDouble();
    m_minPrice = conf.root().get("minPrice", 0).asDouble();
    m_minTimeframe = conf.minTimeframe();

    // stream data sources
    if (m_minTimeframe <= 0.0) {
        addTickDataSource();
    } else {
        addMidOhlcDataSource(m_minTimeframe);
    }

    if (conf.root().isMember("timeframes")) {
        Json::Value timeframes = conf.root().get("timeframes", Json::Value());
        for (auto it = timeframes.begin(); it != timeframes.end(); ++it) {
            Json::Value timeframe = *it;

            o3d::String mode = timeframe.get("mode", "").asString().c_str();

            o3d::Double tf = conf.timeframeAsDouble(timeframe, "timeframe");
            o3d::Double subTf = conf.timeframeAsDouble(timeframe, "subTimeframe");

            o3d::Int32 depth = timeframe.get("depth", 0).asInt();
            o3d::Int32 history = timeframe.get("history", 0).asInt();

            if (!timeframe.get("enabled", false).asBool()) {
                continue;
            }

            if (mode == "A") {
                Analyser *a = new IaAAnalyser(this, tf, subTf, depth, history);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
            } else if (mode == "B") {
                Analyser *b = new IaBAnalyser(this, tf, subTf, depth, history);
                b->init(AnalyserConfig(timeframe));

                m_analysers.push_back(b);
            } else if (mode == "C") {
                Analyser *c = new IaCAnalyser(this, tf, subTf, depth, history);
                c->init(AnalyserConfig(timeframe));

                m_analysers.push_back(c);
            } else if (mode == "D") {
                Analyser *d = new IaDAnalyser(this, tf, subTf, depth, history);
                d->init(AnalyserConfig(timeframe));

                m_analysers.push_back(d);
            } else {
                // ignored, unknow mode
                O3D_WARNING(o3d::String("ForexAlpha strategy unknow mode {0}").arg(mode));
            }
        }
    }

    setInitialized();
}

void IndiceAlpha::terminate(Connector *connector, Database *db)
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

void IndiceAlpha::prepareMarketData(Connector *connector, Database *db)
{
    setMarketDataPrepared();
}

void IndiceAlpha::finalizeMarketData(Connector *connector, Database *db)
{
    m_tradeManager = new StdTradeManager(this);
    setReady();
}

void IndiceAlpha::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    if (m_minTimeframe == TF_TICK) {
        for (Analyser *analyser : m_analysers) {
            analyser->onTickUpdate(timestamp, ticks);
        }
    }
}

void IndiceAlpha::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, Ohlc::Type ohlcType, const OhlcArray &ohlc)
{
    if (m_minTimeframe == timeframe) {
        for (Analyser *analyser : m_analysers) {
            analyser->onOhlcUpdate(timestamp, ohlcType, ohlc);
        }
    }
}

void IndiceAlpha::onOrderSignal(const OrderSignal &orderSignal)
{
    m_tradeManager->onOrderSignal(orderSignal);
}

void IndiceAlpha::onPositionSignal(const PositionSignal &positionSignal)
{
    m_tradeManager->onPositionSignal(positionSignal);
}

void IndiceAlpha::prepare(o3d::Double timestamp)
{
    // prepare before compute
    for (Analyser *analyser : m_analysers) {
        analyser->prepare(timestamp);
    }
}

void IndiceAlpha::compute(o3d::Double timestamp)
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

void IndiceAlpha::finalize(o3d::Double timestamp)
{
    // cleanup
}

void IndiceAlpha::orderEntry(
        o3d::Double timestamp,
        o3d::Double timeframe,
        o3d::Int32 direction,
        o3d::Double price,
        o3d::Double limitPrice,
        o3d::Double stopPrice)
{
    Trade* trade = handler()->traderProxy()->createTrade(market(), timeframe);
    if (trade) {
        m_tradeManager->addTrade(trade);

        o3d::Double quantity = 1.0;  // @todo

        // query open
        trade->open(handler()->traderProxy(), market(), direction, Trade::ORDER_CREATE, price, quantity, stopPrice, limitPrice);

        log(timeframe, "content", "entry");
    }
}

void IndiceAlpha::orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price)
{
    if (trade) {
        if (price > 0.0) {
            // if price defined, limit close else market close
        } else {
            trade->close(handler()->traderProxy(), market());
        }

        log(trade->tf(), "content", "exit");
    }
}
