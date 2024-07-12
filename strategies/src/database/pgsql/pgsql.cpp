/**
 * @brief SiiS strategy database connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "pgsql.h"
#include <o3d/core/error.h>
#include <o3d/pgsql/pgsqldb.h>

#include <postgresql/libpq-fe.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "pgsqlohlcdb.h"
#include "pgsqlrangebardb.h"
#include "pgsqlmarketdb.h"
#include "pgsqltradedb.h"

using namespace siis;

siis::PgSql::PgSql(
            const o3d::String &host,
            o3d::UInt32 port,
            const o3d::String &name,
            const o3d::String &user,
            const o3d::String &pwd)
{
    o3d::pgsql::PgSql::init();

    m_db = new o3d::pgsql::PgSqlDb();
    m_db->connect(host, port, name, user, pwd, true);
}

siis::PgSql::~PgSql()
{
    terminate();
}

void siis::PgSql::init()
{
    if (m_db) {
        m_ohlc = new PgSqlOhlcDb(this);
        m_rangeBar = new PgSqlRangeBarDb(this);
        m_market = new PgSqlMarketDb(this);
        m_trade = new PgSqlTradeDb(this);
    }
}

void siis::PgSql::terminate()
{
    if (m_db) {
        o3d::deletePtr(m_ohlc);
        o3d::deletePtr(m_rangeBar);
        o3d::deletePtr(m_market);
        o3d::deletePtr(m_trade);

        m_db->unregisterAll();
        m_db->disconnect();

        o3d::deletePtr(m_db);
    }

    o3d::pgsql::PgSql::quit();
}

o3d::Int32 PgSql::run(void *)
{
    while (m_running) {
        o3d::System::waitMs(1);
    }

    return 0;
}

o3d::Database *PgSql::db()
{
    return m_db;
}
