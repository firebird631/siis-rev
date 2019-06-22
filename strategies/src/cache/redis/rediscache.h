/**
 * @brief SiiS strategy cache connector Redis implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-08
 */

#ifndef SIIS_REDISCACHE_H
#define SIIS_REDISCACHE_H

#include "siis/cache/cache.h"

#include <o3d/core/thread.h>

namespace siis {

/**
 * @brief Strategy cache connector Redis implementation.
 * @author Frederic Scherma
 * @date 2019-06-08
 * @ref https://github.com/redis/hiredis
 */
class SIIS_API RedisCache : public Cache
{
public:

    RedisCache(const o3d::String &host,
            o3d::UInt32 port,
            const o3d::String &name,
            const o3d::String &user,
            const o3d::String &pwd);

    virtual ~RedisCache();

    virtual void init();
    virtual void terminate();

    struct redisContext* redis();

    virtual o3d::Int32 run(void *);

private:

    struct redisContext *m_redis;
};

} // namespace siis

#endif // SIIS_REDISCACHE_H
