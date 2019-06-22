/**
 * @brief SiiS strategy trade database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_TRADEDB_H
#define SIIS_TRADEDB_H

#include "siis/trade/trade.h"

namespace siis {

class Database;

/**
 * @brief Strategy trade database DAO.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API TradeDb
{
public:

    virtual ~TradeDb() = 0;

    virtual Database* db() = 0;
    virtual const Database* db() const = 0;

    virtual o3d::Bool fetchTrade(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 tradeId, Trade &out) = 0;

    virtual std::list<Trade*> fetchTradeList(const o3d::String &brokerId, const o3d::String &marketId) = 0;

    virtual void storeTrade(const o3d::String &brokerId, const o3d::String &marketId,
                            const o3d::String &strategyId, const Trade &trade) = 0;
};

} // namespace siis

#endif // SIIS_TRADEDB_H
