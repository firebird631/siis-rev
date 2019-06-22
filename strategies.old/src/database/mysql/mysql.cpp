/**
 * @brief SiiS strategy database connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "mysql.h"
#include <o3d/core/error.h>

#include <o3d/mysql/mysqldb.h>

#include "mysqlohlcdb.h"
#include "mysqlmarketdb.h"
#include "mysqltradedb.h"

using namespace siis;

siis::MySql::MySql(
            const o3d::String &host,
            o3d::UInt32 port,
            const o3d::String &name,
            const o3d::String &user,
            const o3d::String &pwd) :
    m_db(nullptr)
{
    o3d::mysql::MySql::init();

    m_db = new o3d::mysql::MySqlDb();
    m_db->connect(host, port, name, user, pwd, true);

//    m_db->registerQuery("fetch-market", "");
//    m_db->registerQuery("fetch-trades", "");
//    m_db->registerQuery("fetch-candles", "");
//    m_db->registerQuery("store-trade", "");
}

siis::MySql::~MySql()
{
    terminate();
}

void siis::MySql::terminate()
{
    if (m_db) {
        m_db->disconnect();
        o3d::deletePtr(m_db);
    }

    o3d::mysql::MySql::quit();
}

OhlcDb *siis::MySql::buildOhlcDb()
{
    return new MySqlOhlcDb(this);
}

MarketDb *siis::MySql::buildMarketDb()
{
    return new MySqlMarketDb(this);
}

TradeDb *MySql::buildTradeDb()
{
    return new MySqlTradeDb(this);
}
