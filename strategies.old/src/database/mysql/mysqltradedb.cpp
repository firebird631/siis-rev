/**
 * @brief SiiS strategy trade database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "mysql.h"
#include "mysqltradedb.h"

using namespace siis;

MySqlTradeDb::MySqlTradeDb(siis::MySql *db) :
    m_db(db)
{

}

MySqlTradeDb::~MySqlTradeDb()
{

}

Database *MySqlTradeDb::db()
{
    return m_db;
}

const Database *MySqlTradeDb::db() const
{
    return m_db;
}

o3d::Bool MySqlTradeDb::fetchTrade(const o3d::String &brokerId, const o3d::String &marketId, o3d::Int32 tradeId, Trade &out)
{
    return false;
}

std::list<Trade *> MySqlTradeDb::fetchTradeList(const o3d::String &brokerId, const o3d::String &marketId)
{
    std::list<Trade *> results;

    return results;
}

void MySqlTradeDb::storeTrade(const o3d::String &brokerId, const o3d::String &marketId, const o3d::String &strategyId, const Trade &trade)
{

}
