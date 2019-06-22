/**
 * @brief SiiS strategy trade database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_MYSQLTRADEDB_H
#define SIIS_MYSQLTRADEDB_H

#include "siis/database/tradedb.h"

namespace siis {

class MySql;

/**
 * @brief Strategy trade database DAO.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API MySqlTradeDb : public TradeDb
{
public:

    MySqlTradeDb(siis::MySql *db);

    virtual ~MySqlTradeDb();

    virtual Database* db();
    virtual const Database* db() const;

    virtual o3d::Bool fetchTrade(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 tradeId, Trade &out);

    virtual std::list<Trade*> fetchTradeList(const o3d::String &brokerId, const o3d::String &marketId);

    virtual void storeTrade(const o3d::String &brokerId, const o3d::String &marketId,
                            const o3d::String &strategyId, const Trade &trade);

private:

    MySql *m_db;
};

} // namespace siis

#endif // SIIS_MYSQLTRADEDB_H
