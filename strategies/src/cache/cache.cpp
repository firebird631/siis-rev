/**
 * @brief SiiS strategy cache connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-08
 */

#include "siis/cache/cache.h"
#include <o3d/core/error.h>

#include "redis/rediscache.h"
#include "redis/redistradecache.h"

using namespace siis;

Cache *Cache::builder(
        const o3d::String &db,
        const o3d::String &host,
        o3d::UInt32 port,
        const o3d::String &name,
        const o3d::String &user,
        const o3d::String &pwd)
{
    if (db == "redis") {
        return new siis::RedisCache(host, port, name, user, pwd);
    } else {
        throw o3d::E_InvalidParameter("Unsupported cache type");
    }
}

Cache::Cache() :
    m_thread(this),
    m_running(false),
    m_trade(nullptr)
{

}

void Cache::start()
{
    if (!m_running) {
        m_running = true;
        m_thread.start();
        m_thread.setName("siis::cache");
    }
}

void Cache::stop()
{
    if (m_running) {
        m_running = false;
        m_thread.waitFinish();
    }
}

TradeCache *Cache::trade()
{
    return m_trade;
}

Cache::~Cache()
{

}

TradeCache::~TradeCache()
{

}
