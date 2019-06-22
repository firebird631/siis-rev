/**
 * @brief SiiS strategy trade cache DAO Redis implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-08
 */

#ifndef SIIS_REDISTRADECACHE_H
#define SIIS_REDISTRADECACHE_H

#include "siis/cache/tradecache.h"

#include <o3d/core/templatearray.h>
#include <o3d/core/string.h>
#include <o3d/core/mutex.h>

namespace siis {

class RedisCache;

/**
 * @brief Strategy trade cache DAO Redis implementation.
 * @author Frederic Scherma
 * @date 2019-06-08
 */
class SIIS_API RedisTradeCache : public TradeCache
{
public:

    RedisTradeCache(siis::RedisCache* cache);

    virtual ~RedisTradeCache() override;

    virtual Cache* cache() override;
    virtual const Cache* cache() const override;

    virtual void setTrade(const o3d::String &brokerId, const o3d::String &marketId,
                          const o3d::String &strategyId, const Trade &trade) override;

    virtual void delTrade(const o3d::String &brokerId, const o3d::String &marketId,
                          const o3d::String &strategyId, const Trade &trade) override;

    virtual void delAll() override;

    virtual void execute() override;

private:

    RedisCache *m_redisCache;
    // o3d::TemplateArray<o3d::String> m_cmdList;   //!< this structure avoid lot of memory reallocations
    std::list<o3d::String> m_cmdList;

    o3d::String m_buffer;
    o3d::FastMutex m_mutex;
};

} // namespace siis

#endif // SIIS_REDISTRADECACHE_H
