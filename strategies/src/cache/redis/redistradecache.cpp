/**
 * @brief SiiS strategy trade cache DAO Redis implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-08
 */

#include <hiredis/hiredis.h>

#include "redistradecache.h"
#include "rediscache.h"

#include "siis/trade/trade.h"

using namespace siis;

RedisTradeCache::RedisTradeCache(RedisCache *cache) :
    m_redisCache(cache)
{

}

RedisTradeCache::~RedisTradeCache()
{
    // delAll();
}

Cache *RedisTradeCache::cache()
{
    return m_redisCache;
}

const Cache *RedisTradeCache::cache() const
{
    return m_redisCache;
}

void RedisTradeCache::setTrade(
        const o3d::String &brokerId, const o3d::String &marketId, const o3d::String &strategyId, const Trade &trade)
{
    m_mutex.lock();

    // this way avoid memory allocation
    m_buffer = "RPUSH trade.";
    m_buffer += strategyId;
    m_buffer += '.';
    m_buffer += brokerId;
    m_buffer += '.';
    m_buffer += marketId;
    m_buffer += '.';
    m_buffer.concat(trade.id());
    m_buffer += " \"";
    m_buffer.concat(trade.entryPrice());  // @todo serialize all the members
    m_buffer += '\"';

    m_cmdList.push_back(m_buffer);

    m_mutex.unlock();
}

void RedisTradeCache::delTrade(
        const o3d::String &brokerId, const o3d::String &marketId, const o3d::String &strategyId, const Trade &trade)
{
    m_mutex.lock();
    m_mutex.unlock();
}

void RedisTradeCache::delAll(const o3d::String &brokerId, const o3d::String &strategyId)
{
    m_mutex.lock();

    // this way avoid memory allocation
//    m_buffer = "RPUSH trade.";
//    m_buffer += strategyId;
//    m_buffer += '.';
//    m_buffer += brokerId;
//    m_buffer += '.';
//    m_buffer += marketId;
//    m_buffer += '.';
//    m_buffer.concat(trade.id());
//    m_buffer += " \"";
//    m_buffer += '\"';

//    ::redisCommand(m_redisCache->redis(), "KEYS trade");

    m_mutex.unlock();
}

#include "siis/trade/assettrade.h"
void RedisTradeCache::execute()
{
    AssetTrade trade(1.0);
    trade.setId(1);
    setTrade("binance.com", "ETHUSDT", "SUP", trade);
    o3d::System::waitMs(2000);

    m_mutex.lock();

//    if (m_cmdList.isValid()) {
//        for (o3d::Int32 i = 0; i < m_cmdList.getSize(); ++i) {
//            ::redisCommand(m_redisCache->redis(), m_cmdList[i].toUtf8().getData());
//        }

//        m_cmdList.setSize(0);
//    }

    if (!m_cmdList.empty()) {
        for (auto it = m_cmdList.begin(); it != m_cmdList.end(); ++it) {
            ::redisCommand(m_redisCache->redis(), (*it).toUtf8().getData());
        }

        m_cmdList.clear();
    }

    m_mutex.unlock();
}
