/**
 * @brief SiiS strategy market database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_PGSQLMARKETDB_H
#define SIIS_PGSQLMARKETDB_H

#include "siis/database/marketdb.h"

namespace siis {

class PgSql;

/**
 * @brief Strategy market database DAO.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API PgSqlMarketDb : public MarketDb
{
public:

    PgSqlMarketDb(siis::PgSql *db);

    virtual ~PgSqlMarketDb();

    virtual Database* db();
    virtual const Database* db() const;

    virtual o3d::Bool fetchMarket(const o3d::String &brokerId, const o3d::String &marketId, Market &out);
    virtual std::list<Market*> fetchMarketList(const o3d::String &brokerId, const o3d::T_StringList &marketsId);

    // virtual void storeMarket(const o3d::String &brokerId, const Market &market);

private:

    PgSql *m_db;
};

} // namespace siis

#endif // SIIS_PGSQLMARKETDB_H
