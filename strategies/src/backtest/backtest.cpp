/**
 * @brief SiiS strategy backtesting process.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-10
 */

#include "backtest.h"

#include "siis/connector/localconnector.h"
#include "siis/connector/traderproxy.h"

#include "siis/database/tickstream.h"

#include "siis/poolworker.h"

#include "siis/strategy.h"
#include "siis/market.h"
#include "siis/collection.h"
#include "siis/display/displayer.h"

#include "siis/utils/common.h"

#include "siis/database/database.h"
#include "siis/database/marketdb.h"

#include <o3d/core/debug.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

Backtest::Backtest() :
    m_thread(this),
    m_running(false),
    m_fromTs(0.0),
    m_toTs(0.0),
    m_curTs(0.0),
    m_timestep(0.0),
    m_connector(nullptr),
    m_traderProxy(nullptr)
{

}

Backtest::~Backtest()
{

}

#include "siis/database/ohlcdb.h"
void Backtest::init(
        Displayer *displayer,
        Config *config,
        StrategyCollection *collection,
        PoolWorker *poolWorker,
        Database *database,
        Cache *cache)
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

        Strategy *strategy = collection->build(this, config->getStrategy(), config->getStrategyIdentifier());
        strategy->init(config);

        StrategyElt elt;
        elt.strategy = strategy;
        elt.market = market;

        strategy->setMarket(market);
        strategy->prepareMarketData(m_connector, m_database);
        strategy->finalizeMarketData(m_connector, m_database);

        if (mc->marketTradeType > -1) {
            market->setTradeCapacities(mc->marketTradeType);
        }

        // market data from database (synchronous)
        if (!database->market()->fetchMarket(config->getBrokerId(), mc->marketId, market)) {
            O3D_ERROR(o3d::E_InvalidPrecondition(o3d::String("Unable to find market info for ") + mc->marketId));
        }

        // Ohlc ohlc = database->ohlc()->getLastOhlc(config->getBrokerId(), mc->marketId, 4*60*60);
        // o3d::System::print(ohlc.toString(), "");

        for (DataSource ds : strategy->getDataSources()) {
            // create the stream (for now only tick stream, but could offer OHLC too, but no possibility for order-book history)
            if (ds.type == DataSource::TICK) {
                TickStream *ts = new TickStream(config->getMarketsPath().getFullPathName(),
                                                config->getBrokerId(), mc->marketId, fromDt, toDt);

                elt.tickStream = ts;

            } else if (ds.type == DataSource::OHLC_MID) {
                // @todo fetch and stream ...
            } else if (ds.type == DataSource::OHLC_BID) {
                // @todo ...
            } else if (ds.type == DataSource::OHLC_ASK) {
                // @todo ...
            } else if (ds.type == DataSource::ORDER_BOOK) {
                // not possible for now
            }
        }

        m_strategies[mc->marketId] = elt;
    }

    // create and start a primary local connector in this thread
    m_connector = new LocalConnector(this);
    m_connector->init(config);

    // and a trader proxy based on this connector
    m_traderProxy = new TraderProxy(m_connector);

    // and define this proxy on the primary connector
    m_connector->setTraderProxy(m_traderProxy);

    m_poolWorker = poolWorker;
    m_database = database;
    m_cache = cache;
}

void Backtest::terminate(Config *config)
{
    GlobalStatistics globalStats;
    AccountStatistics accountStats;  // @todo from paper trader

    // delete strategies and markets
    for (auto pair : m_strategies) {
        Strategy *strategy = pair.second.strategy;

        strategy->terminate(m_connector, m_database);

        // compute final statistics
        globalStats.add(strategy->statistics());

        o3d::deletePtr(pair.second.strategy);
        o3d::deletePtr(pair.second.market);
        o3d::deletePtr(pair.second.tickStream);
    }
    m_strategies.clear();

    // if learning write final
    if (config->getLearningFilename().isValid()) {
        config->overwriteLearningFile(globalStats, accountStats);
    }

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

void Backtest::start()
{
     // create a decicaded thread for the timecounter
    if (!m_running) {
        m_running = true;
        m_thread.start();
        m_thread.setName("siis::backtest");
    }
}

void Backtest::stop()
{
    // stop the connector, delete it
    if (m_running && m_thread.isThread()) {
        m_running = false;
        m_thread.waitFinish();
    }
}

void Backtest::sync()
{
    if (m_running) {
        // @todo
    }
}

o3d::Double Backtest::timestamp() const
{
    return m_curTs;
}

o3d::Double Backtest::progress() const
{
    return o3d::clamp((m_curTs - m_fromTs) / (m_toTs - m_fromTs) * 100.0, 0.0, 100.0);
}

const TraderProxy *Backtest::traderProxy() const
{
    return m_traderProxy;
}

TraderProxy *Backtest::traderProxy()
{
    return m_traderProxy;
}

void Backtest::setPaperMode(o3d::Bool)
{
    // not available in backtest
}

void Backtest::onTick(const o3d::CString &, const Tick &)
{
    // nothing in backtesting
}

void Backtest::onOhlc(const o3d::CString &, Ohlc::Type, const Ohlc &)
{
    // nothing in backtesting
}

Market *Backtest::market(const o3d::CString &marketId)
{
//    for (auto pair : m_strategies) {
//        if (pair.second.market && pair.second.market->marketId() == marketId) {
//            return pair.second.market;
//        }
//    }

    auto it = m_strategies.find(marketId);
    if (it != m_strategies.end()) {
        return it->second.strategy->market();
    }

    return nullptr;
}

const Market *Backtest::market(const o3d::CString &marketId) const
{
//    for (auto pair : m_strategies) {
//        if (pair.second.market && pair.second.market->marketId() == marketId) {
//            return pair.second.market;
//        }
//    }

    auto cit = m_strategies.find(marketId);
    if (cit != m_strategies.cend()) {
        return cit->second.strategy->market();
    }

    return nullptr;
}

Strategy* Backtest::strategy(const o3d::CString &marketId)
{
    auto it = m_strategies.find(marketId);
    if (it != m_strategies.end()) {
        return it->second.strategy;
    }

    return nullptr;
}

const Strategy *Backtest::strategy(const o3d::CString &marketId) const
{
    auto cit = m_strategies.find(marketId);
    if (cit != m_strategies.cend()) {
        return cit->second.strategy;
    }

    return nullptr;
}

Database *Backtest::database()
{
    return m_database;
}

Cache *Backtest::cache()
{
    return m_cache;
}

void Backtest::log(o3d::Double timeframe, const o3d::String &marketId, const o3d::String &channel, const o3d::String &msg)
{
    // @todo throught the monitor send a log message
    // m_monitor.log(timestamp(), timeframe, marketId, channel, msg);
    o3d::DateTime dt;
    dt.fromTime(timestamp(), true);

    m_displayer->display(channel, o3d::String("[{0}] (tf={1}) (on={2}) : {3}").arg(dt.buildString("%Y-%m-%d %H:%M:%S"))
                         .arg(siis::timeframeToStr(timeframe)).arg(marketId).arg(msg));
}

o3d::Int32 Backtest::run(void *)
{
    Strategy *strategy = nullptr;
    Market *market = nullptr;
    o3d::Int32 n = 0;
    DataArray ticks;

    if (m_strategies.size() <= 1) {
        while (m_running) {
            if (m_curTs > m_toTs) {
                break;
            }

            for (auto pair : m_strategies) {
                strategy = pair.second.strategy;
                market = pair.second.market;

                if (!strategy->running()) {
                    continue;
                }

                if (pair.second.tickStream) {
                    // no need to acquire/release because we are always synchronous in backtesting
                    n = pair.second.tickStream->fillNext(m_curTs, market->getTickBuffer());
                   // n = pair.second.tickStream->fillNext(m_curTs, ticks);
                }

                if (n <= 0) {
                    // no ticks for this run
                    continue;
                }

//                printf("%i %f\n", market->getTickBuffer().getSize(), m_curTs);
//                if (market->getTickBuffer().getSize() > 0) {
//                    printf(">%f \n", market->getTickBuffer().getData()[0]);
//                }

                // inject ticks into the strategy
                strategy->onTickUpdate(m_curTs, market->getTickBuffer());

                // consume them
                market->setLastTick(market->getTickBuffer().last());
                market->getTickBuffer().forceSize(0);
                // ticks.destroy();

                // process one strategy iteration
                strategy->process(m_curTs);

                // update the local connector to manage orders, positions and virtual account details
                m_connector->update();
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
            countDown.count = static_cast<o3d::Int32>(m_strategies.size());

            for (auto pair : m_strategies) {
                strategy = pair.second.strategy;
                market = pair.second.market;

                if (!strategy->running()) {
                    continue;
                }

                if (pair.second.tickStream) {
                    // no need to acquire/release because we are always synchronous in backtesting
                    n = pair.second.tickStream->fillNext(m_curTs, market->getTickBuffer());
                    // n = pair.second.tickStream->fillNext(m_curTs, ticks);
                }

                if (n <= 0) {
                    // no ticks for this run
                    continue;
                }

                // inject ticks into the strategy
                strategy->onTickUpdate(m_curTs, market->getTickBuffer());

                // consume them
                market->setLastTick(market->getTickBuffer().last());
                market->getTickBuffer().forceSize(0);
                // ticks.destroy();

                // process one strategy iteration
                m_poolWorker->addJob(strategy, m_curTs, &countDown);

                // update the local connector to manage orders, positions and virtual account details
                m_connector->update();
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
