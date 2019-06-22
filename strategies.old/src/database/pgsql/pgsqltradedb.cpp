/**
 * @brief SiiS strategy trade database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "pgsql.h"
#include "pgsqltradedb.h"

using namespace siis;

PgSqlTradeDb::PgSqlTradeDb(siis::PgSql *db) :
    m_db(db)
{

}

PgSqlTradeDb::~PgSqlTradeDb()
{

}

Database *PgSqlTradeDb::db()
{
    return m_db;
}

const Database *PgSqlTradeDb::db() const
{
    return m_db;
}

o3d::Bool PgSqlTradeDb::fetchTrade(const o3d::String &brokerId, const o3d::String &marketId, o3d::Int32 tradeId, Trade &out)
{
    return false;
}

std::list<Trade *> PgSqlTradeDb::fetchTradeList(const o3d::String &brokerId, const o3d::String &marketId)
{
    std::list<Trade *> results;

    return results;
}

void PgSqlTradeDb::storeTrade(const o3d::String &brokerId, const o3d::String &marketId, const o3d::String &strategyId, const Trade &trade)
{

}
