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

#include "siis/trade/tradesignal.h"

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
    conf.parseStrategyOverrides(config->getStrategiesPath(), config->getStrategyFilename());

    initBasicsParameters(conf);

    m_minVol24h = conf.root().get("min-vol-24h", 0).asDouble();
    m_minPrice = conf.root().get("min-price", 0).asDouble();

    m_minTradedTimeframe = conf.minTradedTimeframe();
    m_maxTradedTimeframe = conf.maxTradedTimeframe();

    if (conf.root().isMember("timeframes")) {
        Json::Value timeframes = conf.root().get("timeframes", Json::Value());
        for (auto it = timeframes.begin(); it != timeframes.end(); ++it) {
            Json::Value timeframe = *it;

            o3d::String name = it.name().c_str();
            o3d::String mode = timeframe.get("mode", "").asString().c_str();

            o3d::Double tf = conf.timeframeAsDouble(timeframe, "timeframe");

            o3d::Int32 depth = timeframe.get("depth", 0).asInt();
            o3d::Double history = conf.timeframeAsDouble(timeframe, "history");

            if (!timeframe.get("enabled", true).asBool()) {
                continue;
            }

            if (mode == "A") {
                IaAnalyser *a = new IaAAnalyser(this, name, tf, baseTimeframe(), depth, history);
                a->init(AnalyserConfig(timeframe));

                m_analysers.push_back(a);
            } else if (mode == "B") {
                IaAnalyser *b = new IaBAnalyser(this, name, tf, baseTimeframe(), depth, history);
                b->init(AnalyserConfig(timeframe));

                m_analysers.push_back(b);
            } else if (mode == "C") {
                IaAnalyser *c = new IaCAnalyser(this, name, tf, baseTimeframe(), depth, history);
                c->init(AnalyserConfig(timeframe));

                m_analysers.push_back(c);
            } else if (mode == "D") {
                IaAnalyser *d = new IaDAnalyser(this, name, tf, baseTimeframe(), depth, history);
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

void IndiceAlpha::prepareMarketData(Connector *connector, Database *db, o3d::Double fromTs, o3d::Double toTs)
{
    setMarketDataPrepared();
}

void IndiceAlpha::finalizeMarketData(Connector *connector, Database *db)
{
    m_tradeManager = new StdTradeManager(this);
    setReady();
    setRunning();
}

void IndiceAlpha::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    if (baseTimeframe() == TF_TICK) {
        for (Analyser *analyser : m_analysers) {
            analyser->onTickUpdate(timestamp, ticks);
        }
    }
}

void IndiceAlpha::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, Ohlc::Type ohlcType, const OhlcArray &ohlc)
{
    if (baseTimeframe() == timeframe) {
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

    for (IaAnalyser *analyser : m_analysers) {
        analyser->process(timestamp, lastTimestamp());
        if (analyser->lastSignal().valid()) {
            sig |= true;
        }
    }

    // update the existing trades
    m_tradeManager->process(timestamp);

    if (sig) {
        Trade *trade = nullptr;
        o3d::Double maxExitTf = 0;

        // one or more potential entries or exits signals
        for (IaAnalyser *analyser : m_analysers) {
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

                    orderEntry(timestamp, signal.tf(), signal.d(), signal.orderType(), signalPrice, signal.tp(), signal.sl());
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

void IndiceAlpha::updateTrade(Trade *trade)
{

}

void IndiceAlpha::updateStats()
{

}

void IndiceAlpha::orderEntry(
        o3d::Double timestamp,
        o3d::Double timeframe,
        o3d::Int32 direction,
        Order::OrderType orderType,
        o3d::Double price,
        o3d::Double limitPrice,
        o3d::Double stopPrice)
{
    Trade* trade = handler()->traderProxy()->createTrade(market(), tradeType(), timeframe);
    if (trade) {
        m_tradeManager->addTrade(trade);

        o3d::Double quantity = baseQuantity();

        // query open
        trade->open(this, direction, orderType, price, quantity, stopPrice, limitPrice);

        log(timeframeToStr(timeframe), "content", "entry");
    }
}

void IndiceAlpha::orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price)
{
    if (trade) {
        if (price > 0.0) {
            // if price defined, limit close else market close
        } else {
            trade->close(TradeStats::REASON_CLOSE_MARKET);
        }

        // @todo trade manager removeTrader and it must also freeTrade from tradeproxy

        log(timeframeToStr(trade->tf()), "content", "exit");
    }
}
