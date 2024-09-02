/**
 * @brief SiiS strategy machine learning supervisor interface.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-28
 */

#include "siis/learning/stdsupervisor.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

StdSupervisor::StdSupervisor(Handler *handler, const o3d::String &identifier) :
    Supervisor(handler, identifier),
    m_needUpdate(false),
    m_baseTimeframe(0.0),
    m_minTradedTimeframe(0.0),
    m_maxTradedTimeframe(0.0),
    m_numEpoch(0),
    m_batchSize(0),
    m_tradeManager(nullptr)
{

}

StdSupervisor::~StdSupervisor()
{

}

void StdSupervisor::init(Config *config)
{
    Strategy::init(config);

    setProperty("name", "stdsupervisor");
    setProperty("author", "Frederic SCHERMA (frederic.scherma@gmail.com)");
    setProperty("date", "2019-03-28");
    setProperty("revision", "1");
    setProperty("copyright", "2018-2019 Dream Overflow");
    setProperty("comment", "Std supervisor");

    // strategie parameters
    StrategyConfig conf;
    conf.parseDefaults("{}");
    conf.parseStrategyOverrides(config->getStrategiesPath(), config->getStrategyFilename());

    m_needUpdate = conf.root().get("need-update", false).asBool();
    m_baseTimeframe = conf.baseTimeframe();
    m_minTradedTimeframe = conf.minTradedTimeframe();
    m_maxTradedTimeframe = conf.maxTradedTimeframe();

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

            o3d::Int32 depth = timeframe.get("depth", 0).asInt();
            o3d::Double history = conf.timeframeAsDouble(timeframe, "history");

            if (!timeframe.get("enabled", true).asBool()) {
                continue;
            }

//            if (mode == "A") {
//                Analyser *a = new Std...Analyser(this, tf, baseTimeframe(), depth);
//                a->init(AnalyserConfig(timeframe));

//                m_analysers.push_back(a);
//            } else if (mode == "B") {
//                Analyser *b = new FaBAnalyser(this, tf, baseTimeframe(), depth);
//                b->init(AnalyserConfig(timeframe));

//                m_analysers.push_back(b);
//            } else if (mode == "C") {
//                Analyser *c = new FaCAnalyser(this, tf, baseTimeframe(), depth);
//                c->init(AnalyserConfig(timeframe));

//                m_analysers.push_back(c);
//            } else if (mode == "D") {
//                Analyser *d = new FaDAnalyser(this, tf, baseTimeframe(), depth);
//                d->init(AnalyserConfig(timeframe));

//                m_analysers.push_back(d);
//            } else {
//                // ignored, unknow mode
//                O3D_WARNING(o3d::String("Standard supervisor unknow mode {0}").arg(mode));
//            }
        }
    }

    setInitialized();
}

void StdSupervisor::terminate(Connector *connector, Database *db)
{
//    for (Analyser *analyser : m_analysers) {
//        analyser->terminate();
//        o3d::deletePtr(analyser);
//    }
//    m_analysers.clear();

    if (m_tradeManager) {
        // m_tradeManager->saveTrades(tradeDb);
        o3d::deletePtr(m_tradeManager);
    }

    setTerminated();
}

void StdSupervisor::prepareMarketData(Connector *connector, Database *db, o3d::Double fromTs, o3d::Double toTs)
{
    setMarketDataPrepared();
}

void StdSupervisor::finalizeMarketData(Connector *connector, Database *db)
{
    m_tradeManager = new StdTradeManager(this);
    setReady();
}

void StdSupervisor::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    if (m_baseTimeframe == TF_TICK) {
//        for (Analyser *analyser : m_analysers) {
//            analyser->onTickUpdate(timestamp, ticks);
//        }
    }
}

void StdSupervisor::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, Ohlc::Type ohlcType, const OhlcArray &ohlc)
{
    if (m_baseTimeframe == timeframe) {
//        for (Analyser *analyser : m_analysers) {
//            analyser->onOhlcUpdate(timestamp, ohlcType, ohlc);
//        }
    }
}

void StdSupervisor::prepare(o3d::Double timestamp)
{

}

void StdSupervisor::compute(o3d::Double timestamp)
{

}

void StdSupervisor::finalize(o3d::Double timestamp)
{
    // cleanup
}
