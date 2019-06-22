/**
 * @brief SiiS strategy market database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "pgsql.h"
#include "pgsqlmarketdb.h"

using namespace siis;

PgSqlMarketDb::PgSqlMarketDb(siis::PgSql *db) :
    m_db(db)
{

}

PgSqlMarketDb::~PgSqlMarketDb()
{

}

Database *PgSqlMarketDb::db()
{
    return m_db;
}

const Database *PgSqlMarketDb::db() const
{
    return m_db;
}

o3d::Bool PgSqlMarketDb::fetchMarket(const o3d::String &brokerId, const o3d::String &marketId, Market &out)
{
    return false;
}

std::list<Market *> PgSqlMarketDb::fetchMarketList(const o3d::String &brokerId, const o3d::T_StringList &marketsId)
{
    std::list<Market *> results;

    return results;
}
