/**
 * @brief SiiS strategy OHLC database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "mysql.h"
#include "mysqlohlcdb.h"

using namespace siis;

MySqlOhlcDb::MySqlOhlcDb(siis::MySql *db) :
    m_db(db)
{

}

MySqlOhlcDb::~MySqlOhlcDb()
{

}

siis::Database *MySqlOhlcDb::db()
{
    return m_db;
}

const siis::Database *MySqlOhlcDb::db() const
{
    return m_db;
}

o3d::Bool MySqlOhlcDb::fetchOhlc(const o3d::String &brokerId, const o3d::String &marketId, o3d::Float timeframe, o3d::Float timestamp, Ohlc &out)
{
    return false;
}

o3d::Int32 MySqlOhlcDb::fetchOhlcArray(const o3d::String &brokerId, const o3d::String &marketId, o3d::Float timeframe, o3d::Double from, o3d::Double to, OhlcArray &out)
{
    return 0;
}
