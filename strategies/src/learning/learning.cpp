/**
 * @brief SiiS strategy machine learning process handler.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-10
 */

#include "learning.h"

#include "siis/connector/localconnector.h"
#include "siis/connector/traderproxy.h"

#include "siis/database/tickstream.h"

#include "siis/poolworker.h"

#include "siis/strategy.h"
#include "siis/market.h"
#include "siis/learning/supervisor.h"
#include "siis/collection.h"
#include "siis/display/displayer.h"

#include "siis/utils/common.h"

#include "siis/database/database.h"
#include "siis/database/marketdb.h"

#include <o3d/core/debug.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

Learning::Learning() :
    m_thread(this),
    m_running(false),
    m_fromTs(0.0),
    m_toTs(0.0),
    m_curTs(0.0),
    m_timestep(0.0),
    m_connector(nullptr),
    m_traderProxy(nullptr),
    m_monitor(nullptr),
    m_poolWorker(nullptr),
    m_database(nullptr),
    m_cache(nullptr)
{

}

Learning::~Learning()
{

}

void Learning::init(Displayer *displayer, Config *config, StrategyCollection *collection, PoolWorker *poolWorker, Database *database, Cache *cache)
{
    O3D_ASSERT(displayer != nullptr);
    O3D_ASSERT(config != nullptr);
    O3D_ASSERT(collection != nullptr);
    O3D_ASSERT(poolWorker != nullptr);
    O3D_ASSERT(database != nullptr);
    O3D_ASSERT(cache != nullptr);

    m_displayer = displayer;

    m_fromTs = config->getFromTs();
    m_toTs = config->getToTs();
    m_curTs = m_fromTs;
    m_timestep = config->getTimestep();

    if (m_fromTs <= 0.0) {
        O3D_ERROR(o3d::E_InvalidPrecondition("From datetime parameters must be valid"));
    }

    if (m_toTs <= 0.0) {
        m_toTs = static_cast<o3d::Double>(o3d::System::getTime()) / o3d::System::getTimeFrequency();
    }

    if (m_timestep <= 0.0) {
        O3D_ERROR(o3d::E_InvalidPrecondition("Timestep must be greater than 0"));
    }

    o3d::DateTime fromDt;
    fromDt.fromTime(m_fromTs, true);

    o3d::DateTime toDt;
    toDt.fromTime(m_toTs, true);

    for (MarketConfig *mc : config->getConfiguredMarkets()) {
        Market *market = new Market(mc->marketId, mc->marketId, "", "");  // @todo fetch market from DB if exists

        // Supervisor *supervisor = collection->builder(this, config->getStrategy(), config->getStrategyIdentifier());
        // supervisor->init(config);

        LearningElt elt;
        // elt.supervisor = supervisor;
        elt.market = market;

//        supervisor->setMarket(market);
//        supervisor->prepareMarketData(m_connector, m_database);
//        supervisor->finalizeMarketData(m_connector, m_database);

//        for (DataSource ds : supervisor->getDataSources()) {
//            // create the stream (for now only tick stream, but could offer OHLC too, but no possibility for order-book history)
//            if (ds.type == DataSource::TICK) {
//                TickStream *ts = new TickStream(config->getMarketsPath().getFullPathName(),
//                                                config->getBrokerId(), mc->marketId, fromDt, toDt);

//                elt.tickStream = ts;

//            } else if (ds.type == DataSource::OHLC_MID) {
//                // @todo fetch and stream ...
//            } else if (ds.type == DataSource::OHLC_BID) {
//                // @todo ...
//            } else if (ds.type == DataSource::OHLC_OFR) {
//                // @todo ...
//            } else if (ds.type == DataSource::ORDER_BOOK) {
//                // not possible for now
//            }
//        }

        m_learnings[mc->marketId] = elt;
    }

    // create and start a primary local connector in this thread
    m_connector = new LocalConnector(this);

    // and a trader proxy based on this connector
    m_traderProxy = new TraderProxy(m_connector);

    // and define this proxy on the primary connector
    m_connector->setTraderProxy(m_traderProxy);

    m_poolWorker = poolWorker;
    m_database = database;
    m_cache = cache;
}

void Learning::terminate(Config *config)
{
    // delete strategies and markets
    for (auto pair : m_learnings) {
        // pair.second.supervisor->terminate(m_connector, m_database);

        // o3d::deletePtr(pair.second.supervisor);
        o3d::deletePtr(pair.second.market);
        o3d::deletePtr(pair.second.tickStream);
    }
    m_learnings.clear();

    // delete before primary connector
    if (m_traderProxy) {
        if (m_connector) {
            m_connector->setTraderProxy(nullptr);
        }

        o3d::deletePtr(m_traderProxy);
    }

    if (m_connector) {
        o3d::deletePtr(m_connector);
    }

    m_poolWorker = nullptr;
    m_database = nullptr;
    m_cache = nullptr;
}

void Learning::start()
{
     // create a decicaded thread for the timecounter
    if (!m_running) {
        m_running = true;
        m_thread.start();
        m_thread.setName("siis::learning");
    }
}

void Learning::stop()
{
    // stop the connector, delete it
    if (m_running && m_thread.isThread()) {
        m_running = false;
        m_thread.waitFinish();
    }
}

void Learning::sync()
{
    if (m_running) {
        // @todo
    }
}

o3d::Double Learning::timestamp() const
{
    return m_curTs;
}

o3d::Double Learning::progress() const
{
    return o3d::clamp((m_curTs - m_fromTs) / (m_toTs - m_fromTs) * 100.0, 0.0, 100.0);
}

const TraderProxy *Learning::traderProxy() const
{
    return m_traderProxy;
}

TraderProxy *Learning::traderProxy()
{
    return m_traderProxy;
}

void Learning::setPaperMode(o3d::Bool)
{
    // not available in backtest
}

void Learning::onTick(const o3d::CString &, const Tick &)
{
    // nothing in backtesting
}

void Learning::onOhlc(const o3d::CString &, Ohlc::Type, const Ohlc &)
{
    // nothing in backtesting
}

Market *Learning::market(const o3d::CString &marketId)
{
    auto it = m_learnings.find(marketId);
    if (it != m_learnings.end()) {
        return it->second.supervisor->market();
    }

    return nullptr;
}

const Market *Learning::market(const o3d::CString &marketId) const
{
    auto cit = m_learnings.find(marketId);
    if (cit != m_learnings.cend()) {
        return cit->second.supervisor->market();
    }

    return nullptr;
}

Strategy *Learning::strategy(const o3d::CString &marketId)
{
    auto it = m_learnings.find(marketId);
    if (it != m_learnings.end()) {
        return it->second.supervisor;
    }

    return nullptr;
}

const Strategy *Learning::strategy(const o3d::CString &marketId) const
{
    auto cit = m_learnings.find(marketId);
    if (cit != m_learnings.cend()) {
        return cit->second.supervisor;
    }

    return nullptr;
}

Database *Learning::database()
{
    return m_database;
}

Cache *Learning::cache()
{
    return m_cache;
}

void Learning::log(o3d::Double timeframe, const o3d::String &marketId, const o3d::String &channel, const o3d::String &msg)
{
    // @todo throught the monitor send a log message
    // m_monitor.log(timestamp(), timeframe, marketId, channel, msg);
    m_displayer->display(channel, o3d::String("[{0}] (tf={1}) (on={2}) : {3}").arg(timestamp())
                         .arg(siis::timeframeToStr(timeframe)).arg(marketId).arg(msg));
}

o3d::Int32 Learning::run(void *)
{
    Supervisor *supervisor = nullptr;
    Market *market = nullptr;
    o3d::Int32 n = 0;
    // DataArray ticks;

    if (m_learnings.size() <= 1) {
        while (m_running) {
            if (m_curTs > m_toTs) {
                break;
            }

            for (auto pair : m_learnings) {
                supervisor = pair.second.supervisor;
                market = pair.second.market;

                if (pair.second.tickStream) {
                    // no need to acquire/release because we are always synchronous in backtesting
                    n = pair.second.tickStream->fillNext(m_curTs, market->getTickBuffer());
                    // n = pair.second.tickStream->fillNext(m_curTs, ticks);
                }

                if (n <= 0) {
                    // no ticks for this run
                    continue;
                }

                // inject the tick to the strategy
//                supervisor->onTickUpdate(m_curTs, market->getTickBuffer());

                // consume them
                market->setLastTick(market->getTickBuffer().last());
                market->getTickBuffer().forceSize(0);
                // ticks.destroy();

                // process one strategy iteration
//                supervisor->process(m_curTs);

                // from strategy trades, once a trade is active follow and compare with the bests entry/exit prices
                // supervisor->learnFromStrategy(strategy->tradeManager());

                //
            }

            m_curTs += m_timestep;

            // yield
            //o3d::System::waitMs(0);
        }
    } else {
        // using PoolWorker and synchronization
        PoolWorker::CountDown countDown;

        while (m_running) {
            if (m_curTs > m_toTs) {
                break;
            }

            // reset the countdown
            countDown.count = static_cast<o3d::Int32>(m_learnings.size());

            for (auto pair : m_learnings) {
                supervisor = pair.second.supervisor;
                market = pair.second.market;

                if (pair.second.tickStream) {
                    // no need to acquire/release because we are always synchronous in backtesting
                    n = pair.second.tickStream->fillNext(m_curTs, market->getTickBuffer());
                    // n = pair.second.tickStream->fillNext(m_curTs, ticks);
                }

                if (n <= 0) {
                    // no ticks for this run
                    continue;
                }

                // inject the tick to the strategy
                // supervisor->onTickUpdate(m_curTs, market->getTickBuffer());

                // consume them
                market->setLastTick(market->getTickBuffer().last());
                market->getTickBuffer().forceSize(0);
                // ticks.destroy();

                // process one strategy iteration
                // m_poolWorker->addJob(supervisor, m_curTs, &countDown);
            }

            // sync before continue
            countDown.wait();

            m_curTs += m_timestep;

            // yield
            //o3d::System::waitMs(0);
        }
    }

    m_running = false;
    return 0;
}
