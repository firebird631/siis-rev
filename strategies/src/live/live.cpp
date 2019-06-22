/**
 * @brief SiiS strategy live process.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-10
 */

#include "live.h"
#include "siis/connector/zmqconnector.h"
#include "siis/connector/traderproxy.h"

#include "siis/poolworker.h"
#include "siis/strategy.h"
#include "siis/market.h"
#include "siis/collection.h"
#include "siis/display/displayer.h"

#include "siis/utils/common.h"

#include <o3d/core/debug.h>

using namespace siis;

Live::Live() :
    m_thread(this),
    m_running(false),
    m_paperMode(false),
    m_connector(nullptr),
    m_traderProxy(nullptr)
{

}

Live::~Live()
{

}

void Live::init(
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

    for (MarketConfig *mc : config->getConfiguredMarkets()) {
        Market *market = new Market(mc->marketId, mc->marketId, "", "");  // @todo fetch market from DB if exists and query from connector

        Strategy *strategy = collection->build(this, config->getStrategy(), config->getStrategyIdentifier());
        m_strategies[mc->marketId] = strategy;

        strategy->setMarket(market);

        // @todo wait results
//        strategy->prepareMarketData(m_connector, m_database);
  //      strategy->finalizeMarketData(m_connector, m_database);
    }

    // create and start a primary connector in this thread
    m_connector = new ZmqConnector(this, config->getConnectorHost() , config->getConnectorPort());

    // and a trader proxy based on this connector
    m_traderProxy = new TraderProxy(m_connector);

    // and define this proxy on the primary connector
    m_connector->setTraderProxy(m_traderProxy);

    m_poolWorker = poolWorker;
    m_database = database;
}

void Live::terminate()
{
    // delete strategies and markets
    for (auto pair : m_strategies) {
        pair.second->terminate(m_connector, m_database);
        o3d::deletePtr(pair.second);
    }
    m_strategies.clear();

    for (auto pair : m_markets) {
        o3d::deletePtr(pair.second);
    }
    m_markets.clear();

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

void Live::start()
{
    // create a decicaded thread for the timecounter
    if (!m_running) {
        m_running = true;
        m_thread.start();
        m_thread.setName("siis::live");
    }

    if (m_connector) {
        m_connector->start();
    }
}

void Live::stop()
{
    // stop the connector, delete it
    if (m_running && m_thread.isThread()) {
        m_running = false;
        m_thread.waitFinish();
    }

    if (m_connector) {
        m_connector->stop();
    }
}

void Live::sync()
{
    if (m_running) {
        // @todo
    }
}

o3d::Double Live::timestamp() const
{
    return static_cast<o3d::Double>(o3d::System::getEpochTime()) / 10000000.0;
}

const TraderProxy *Live::traderProxy() const
{
    return m_traderProxy;
}

TraderProxy *Live::traderProxy()
{
    return m_traderProxy;
}

void Live::setPaperMode(o3d::Bool active)
{
    m_paperMode = active;
}

void Live::onTick(const o3d::String &marketId, const Tick &tick)
{
    m_mutex.lock();

    auto it = m_markets.find(marketId);
    if (it != m_markets.end()) {
        Market *market = it->second;
        market->getTickBuffer().push(tick);
    }

    m_mutex.unlock();
}

void Live::onOhlc(const o3d::String &marketId, Ohlc::Type ohlcType, const Ohlc &ohlc)
{
    m_mutex.lock();

    auto it = m_markets.find(marketId);
    if (it != m_markets.end()) {
        Market *market = it->second;
        market->getOhlcBuffer(ohlcType).push(ohlc);
    }

    m_mutex.unlock();
}

Market *Live::market(const o3d::String &marketId)
{
    auto it = m_markets.find(marketId);
    if (it != m_markets.end()) {
        return it->second;
    }

    return nullptr;
}

const Market *Live::market(const o3d::String &marketId) const
{
    auto cit = m_markets.find(marketId);
    if (cit != m_markets.end()) {
        return cit->second;
    }

    return nullptr;
}

Strategy *Live::strategy(const o3d::String &marketId)
{
    auto it = m_strategies.find(marketId);
    if (it != m_strategies.end()) {
        return it->second;
    }

    return nullptr;
}

const Strategy *Live::strategy(const o3d::String &marketId) const
{
    auto cit = m_strategies.find(marketId);
    if (cit != m_strategies.cend()) {
        return cit->second;
    }

    return nullptr;
}

Database *Live::database()
{
    return m_database;
}

Cache *Live::cache()
{
    return m_cache;
}

void Live::log(o3d::Double timeframe, const o3d::String &marketId, const o3d::String &channel, const o3d::String &msg)
{
    // @todo throught the monitor send a log message
    // m_monitor.log(timestamp(), timeframe, marketId, channel, msg);
    m_displayer->display(channel, o3d::String("{0}:{1}:{2} : {3}").arg(timestamp())
                         .arg(siis::timeframeToStr(timeframe)).arg(marketId).arg(msg));
}

o3d::Int32 Live::run(void *)
{
    Strategy *strategy = nullptr;

    while (m_running) {
        if (m_traderProxy) {
            // if (m_traderProxy->hasMessages()) {
            //     std::queue<Message*> messages = m_connector->traderProxy();
            // }
        }

        // @todo take care of market or strategy mutex when adding data signal to a strategy/market
        // but if a job is running on

        // process strategies on the pool of workers. no need of count-down
        for (std::pair<o3d::String, Strategy*> pair : m_strategies) {
            strategy = pair.second;

            if (!strategy->running()) {
                continue;
            }

            m_poolWorker->addJob(strategy, timestamp());
        }

        // yield
        // o3d::System::waitMs(0);
    }

    m_running = false;
    return 0;
}
