/**
 * @brief SiiS strategy OHLC database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "pgsql.h"
#include "pgsqlohlcdb.h"

using namespace siis;

PgSqlOhlcDb::PgSqlOhlcDb(siis::PgSql *db) :
    m_db(db)
{

}

PgSqlOhlcDb::~PgSqlOhlcDb()
{

}

Database *PgSqlOhlcDb::db()
{
    return m_db;
}

const Database *PgSqlOhlcDb::db() const
{
    return m_db;
}

o3d::Bool PgSqlOhlcDb::fetchOhlc(const o3d::String &brokerId, const o3d::String &marketId, o3d::Float timeframe, o3d::Float timestamp, Ohlc &out)
{
    return false;
}

o3d::Int32 PgSqlOhlcDb::fetchOhlcArray(const o3d::String &brokerId, const o3d::String &marketId, o3d::Float timeframe, o3d::Double from, o3d::Double to, OhlcArray &out)
{
    return 0;
}
