/**
 * @brief SiiS strategy range-bar database DAO.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-12
 */

#include "mysql.h"
#include "mysqlrangebardb.h"

using namespace siis;

MySqlRangeBarDb::MySqlRangeBarDb(siis::MySql *db) :
    m_db(db)
{

}

MySqlRangeBarDb::~MySqlRangeBarDb()
{

}

siis::Database *MySqlRangeBarDb::db()
{
    return m_db;
}

const siis::Database *MySqlRangeBarDb::db() const
{
    return m_db;
}

o3d::Bool MySqlRangeBarDb::fetchOhlc(const o3d::String &brokerId,
                                     const o3d::String &marketId,
                                     o3d::Int32 barSize,
                                     o3d::Double timestamp,
                                     Ohlc &out)
{
    return false;
}

o3d::Int32 MySqlRangeBarDb::fetchOhlcArrayFromTo(const o3d::String &brokerId,
                                                 const o3d::String &marketId,
                                                 o3d::Int32 barSize,
                                                 o3d::Double from,
                                                 o3d::Double to,
                                                 OhlcArray &out)
{
    return 0;
}

o3d::Int32 MySqlRangeBarDb::fetchOhlcArrayLast(const o3d::String &brokerId,
                                       const o3d::String &marketId,
                                       o3d::Int32 barSize,
                                       o3d::Int32 lastN,
                                       OhlcArray &out)
{
    return 0;
}

o3d::Int32 MySqlRangeBarDb::fetchOhlcArrayLastTo(const o3d::String &brokerId,
                                       const o3d::String &marketId,
                                       o3d::Int32 barSize,
                                       o3d::Int32 lastN,
                                       o3d::Double to,
                                       OhlcArray &out)
{
    return 0;
}

o3d::Int32 MySqlRangeBarDb::fetchOhlcArrayFrom(const o3d::String &brokerId,
                                       const o3d::String &marketId,
                                       o3d::Int32 barSize,
                                       o3d::Double from,
                                       OhlcArray &out)
{
    return 0;
}

o3d::Bool MySqlRangeBarDb::getLastOhlc(const o3d::String &brokerId,
                                   const o3d::String &marketId,
                                   o3d::Int32 barSize,
                                   Ohlc &out)
{
    return false;
}
