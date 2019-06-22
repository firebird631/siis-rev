/**
 * @brief SiiS strategy cache connector Redis implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-08
 */

#include <hiredis/hiredis.h>

#include "rediscache.h"
#include <o3d/core/debug.h>

#include "redistradecache.h"

using namespace siis;
using o3d::Debug;
using o3d::Logger;

siis::RedisCache::RedisCache(
            const o3d::String &host,
            o3d::UInt32 port,
            const o3d::String &/*name*/,
            const o3d::String &/*user*/,
            const o3d::String &/*pwd*/) :
    m_redis(nullptr)
{
    if (host.startsWith("unix://")) {
        m_redis = ::redisConnectUnix(host.sub(7).toUtf8().getData());
    } else if (host.startsWith("tcp://")) {
        m_redis = ::redisConnect(host.sub(6).toUtf8().getData(), static_cast<o3d::Int32>(port));
    } else {
        m_redis = ::redisConnect(host.toUtf8().getData(), static_cast<o3d::Int32>(port));
    }

    if (m_redis == nullptr) {
        O3D_ERROR(o3d::E_InvalidParameter("Couldn't connect to redis server"));
    } else if (m_redis->err) {
        O3D_ERROR(o3d::E_InvalidResult(m_redis->errstr));
    }

    // @todo auth
}

siis::RedisCache::~RedisCache()
{
    terminate();
}

void RedisCache::init()
{
    if (m_redis) {
        m_trade = new RedisTradeCache(this);
    }
}

void siis::RedisCache::terminate()
{
    if (m_redis) {
        o3d::deletePtr(m_trade);

        ::redisFree(m_redis);
        m_redis = nullptr;
    }
}

redisContext *RedisCache::redis()
{
    return m_redis;
}

o3d::Int32 RedisCache::run(void *)
{
    while (m_running) {
        // redisReconnect
        // reply = redisCommand(context, "SET key:%s %s", myid, value);
        // reply = redisCommand(context, "SET foo bar");
        // ::redisCommand(m_redis, "SET toto titi");
        // ::redisCommand(m_redis, "EXPIRE toto 2");

        m_trade->execute();

        o3d::System::waitMs(0);
    }

    return 0;
}
