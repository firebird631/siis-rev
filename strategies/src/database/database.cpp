/**
 * @brief SiiS strategy database connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/database/database.h"
#include <o3d/core/error.h>

#include "mysql/mysql.h"
#include "pgsql/pgsql.h"

using namespace siis;
using o3d::Debug;
using o3d::Logger;

Database *Database::builder(
        const o3d::String &db,
        const o3d::String &host,
        o3d::UInt32 port,
        const o3d::String &name,
        const o3d::String &user,
        const o3d::String &pwd)
{
    if (db == "mysql") {
        return new siis::MySql(host, port, name, user, pwd);
    } else if (db == "postgresql") {
        return new siis::PgSql(host, port, name, user, pwd);
    } else {
        O3D_ERROR(o3d::E_InvalidParameter("Unsupported DB type"));
    }
}

Database::Database() :
    m_thread(this),
    m_running(false),
    m_ohlc(nullptr),
    m_market(nullptr),
    m_trade(nullptr)
{

}

void Database::start()
{
    if (!m_running) {
        m_running = true;
        m_thread.start();
        m_thread.setName("siis::db");
    }
}

void Database::stop()
{
    if (m_running) {
        m_running = false;
        m_thread.waitFinish();
    }
}

OhlcDb *Database::ohlc()
{
    return m_ohlc;
}

MarketDb *Database::market()
{
    return m_market;
}

TradeDb *Database::trade()
{
    return m_trade;
}

Database::~Database()
{

}
