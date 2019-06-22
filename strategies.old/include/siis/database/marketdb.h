/**
 * @brief SiiS strategy market database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_MARKETDB_H
#define SIIS_MARKETDB_H

#include <list>

#include "siis/market.h"

#include <o3d/core/stringlist.h>

namespace siis {

class Database;

/**
 * @brief Strategy market database DAO.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API MarketDb
{
public:

    virtual ~MarketDb() = 0;

    virtual Database* db() = 0;
    virtual const Database* db() const = 0;

    virtual o3d::Bool fetchMarket(const o3d::String &brokerId, const o3d::String &marketId, Market &out) = 0;
    virtual std::list<Market*> fetchMarketList(const o3d::String &brokerId, const o3d::T_StringList &marketsId) = 0;

    // virtual void storeMarket(const o3d::String &brokerId, const Market &market) = 0;
};

} // namespace siis

#endif // SIIS_MARKETDB_H
