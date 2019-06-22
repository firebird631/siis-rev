/**
 * @brief SiiS strategy trade cache DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-08
 */

#ifndef SIIS_TRADECACHE_H
#define SIIS_TRADECACHE_H

#include "siis/trade/trade.h"

namespace siis {

class Cache;

/**
 * @brief Strategy trade cache DAO.
 * @author Frederic Scherma
 * @date 2019-06-08
 */
class SIIS_API TradeCache
{
public:

    virtual ~TradeCache() = 0;

    virtual Cache* cache() = 0;
    virtual const Cache* cache() const = 0;

    virtual void setTrade(const o3d::String &brokerId, const o3d::String &marketId,
                          const o3d::String &strategyId, const Trade &trade) = 0;

    virtual void delTrade(const o3d::String &brokerId, const o3d::String &marketId,
                          const o3d::String &strategyId, const Trade &trade) = 0;

    virtual void delAll() = 0;

    virtual void execute() = 0;
};

} // namespace siis

#endif // SIIS_TRADECACHE_H
