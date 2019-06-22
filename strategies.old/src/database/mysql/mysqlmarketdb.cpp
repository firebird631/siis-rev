/**
 * @brief SiiS strategy market database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "mysql.h"
#include "mysqlmarketdb.h"

using namespace siis;

MySqlMarketDb::MySqlMarketDb(siis::MySql *db) :
    m_db(db)
{

}

MySqlMarketDb::~MySqlMarketDb()
{

}

Database *MySqlMarketDb::db()
{
    return m_db;
}

const Database *MySqlMarketDb::db() const
{
    return m_db;
}

o3d::Bool MySqlMarketDb::fetchMarket(const o3d::String &brokerId, const o3d::String &marketId, Market &out)
{
    return false;
}

std::list<Market *> MySqlMarketDb::fetchMarketList(const o3d::String &brokerId, const o3d::T_StringList &marketsId)
{
    std::list<Market *> results;

    return results;
}
