/**
 * @brief SiiS strategy OHLC database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "pgsql.h"
#include "pgsqlrangebardb.h"

using namespace siis;

PgSqlRangeBarDb::PgSqlRangeBarDb(siis::PgSql *db) :
    m_db(db)
{
    m_db->db()->registerQuery("get-last-range-bar",
                              R"SQL(SELECT timestamp, duration, open, high, low, close, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND size = $3 ORDER BY timestamp DESC LIMIT 1)SQL");

    m_db->db()->registerQuery("get-range-bar",
                              R"SQL(SELECT timestamp, duration, open, high, low, close, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND size = $3 AND timestamp = $4)SQL");

    m_db->db()->registerQuery("get-array-from-to-range-bar",
                              R"SQL(SELECT timestamp, duration, open, high, low, close, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND size = $3 AND timestamp >= $4 AND timestamp <= $5 ORDER BY timestamp ASC)SQL");

    m_db->db()->registerQuery("get-array-last-range-bar",
                              R"SQL(SELECT timestamp, duration, open, high, low, close, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND size = $3 ORDER BY timestamp DESC LIMIT $4)SQL");

    m_db->db()->registerQuery("get-array-last-to-range-bar",
                              R"SQL(SELECT timestamp, duration, open, high, low, close, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND size = $3 AND timestamp <= $4 ORDER BY timestamp DESC LIMIT $5)SQL");

    m_db->db()->registerQuery("get-array-from-range-bar",
                              R"SQL(SELECT timestamp, duration, open, high, low, close, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND size = $3 AND timestamp >= $4 ORDER BY timestamp ASC)SQL");
}

PgSqlRangeBarDb::~PgSqlRangeBarDb()
{

}

Database *PgSqlRangeBarDb::db()
{
    return m_db;
}

const Database *PgSqlRangeBarDb::db() const
{
    return m_db;
}

o3d::Bool PgSqlRangeBarDb::fetchOhlc(const o3d::String &brokerId,
                                     const o3d::String &marketId,
                                     o3d::Int32 barSize,
                                     o3d::Double timestamp,
                                     Ohlc &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-range-bar");
    if (!query) {
        return false;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setInt64(2, barSize);
    query->setInt64(3, static_cast<o3d::Int64>(timestamp * 1000.0));

    query->execute();
    query->fetch();

    if (query->getNumRows() != 1) {
        return false;
    }

    out.setTimestamp(timestamp);
    out.setDuration(query->getOut("duration").toDouble() * 0.001);

    out.setO(query->getOut("open").toDouble());
    out.setH(query->getOut("high").toDouble());
    out.setL(query->getOut("low").toDouble());
    out.setC(query->getOut("close").toDouble());

    out.setVolume(query->getOut("volume").toDouble());

    if (out.absHeight() >= static_cast<o3d::Double>(barSize)) {
        out.setConsolidated();
    }

    return true;
}

o3d::Int32 PgSqlRangeBarDb::fetchOhlcArrayFromTo(const o3d::String &brokerId,
                                                 const o3d::String &marketId,
                                                 o3d::Int32 barSize,
                                                 o3d::Double from,
                                                 o3d::Double to,
                                                 OhlcArray &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-array-from-to-range-bar");
    if (!query) {
        return 0;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setInt64(2, barSize);
    query->setInt64(3, static_cast<o3d::Int64>(from * 1000.0));
    query->setInt64(4, static_cast<o3d::Int64>(to * 1000.0));

    query->execute();

    o3d::Int32 n = query->getNumRows();
    o3d::Int32 m = 0;

    if (n <= 0) {
        return 0;
    }

    while (query->fetch()) {
        Ohlc ohlc;

        ohlc.setTimestamp(query->getOut("timestamp").toDouble() * 0.001);
        ohlc.setDuration(query->getOut("duration").toDouble() * 0.001);

        ohlc.setO(query->getOut("open").toDouble());
        ohlc.setH(query->getOut("high").toDouble());
        ohlc.setL(query->getOut("low").toDouble());
        ohlc.setC(query->getOut("close").toDouble());

        ohlc.setVolume(query->getOut("volume").toDouble());

        if (ohlc.absHeight() >= static_cast<o3d::Double>(barSize)) {
            ohlc.setConsolidated();
        }

        out.push(ohlc);

        ++m;
    }

    O3D_ASSERT(n == m);
    return n;
}

o3d::Int32 PgSqlRangeBarDb::fetchOhlcArrayLast(const o3d::String &brokerId,
                                               const o3d::String &marketId,
                                               o3d::Int32 barSize,
                                               o3d::Int32 lastN,
                                               OhlcArray &out)
{
    // take care inverted results
    o3d::DbQuery *query = m_db->db()->findQuery("get-array-last-range-bar");
    if (!query) {
        return 0;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setInt64(2, barSize);
    query->setInt64(3, lastN);

    query->execute();

    o3d::Int32 n = query->getNumRows();
    o3d::Int32 m = 0;

    if (n <= 0) {
        return 0;
    }

    std::list<Ohlc> reversed;

    while (query->fetch()) {
        Ohlc ohlc;

        ohlc.setTimestamp(query->getOut("timestamp").toDouble() * 0.001);
        ohlc.setDuration(query->getOut("duration").toDouble() * 0.001);

        ohlc.setO(query->getOut("open").toDouble());
        ohlc.setH(query->getOut("high").toDouble());
        ohlc.setL(query->getOut("low").toDouble());
        ohlc.setC(query->getOut("close").toDouble());

        ohlc.setVolume(query->getOut("volume").toDouble());

        if (ohlc.absHeight() >= static_cast<o3d::Double>(barSize)) {
            ohlc.setConsolidated();
        }

        reversed.push_front(ohlc);

        ++m;
    }

    for (auto it = reversed.begin(); it != reversed.end(); ++it) {
        out.push(*it);
    }

    O3D_ASSERT(n == m);
    return n;
}

o3d::Int32 PgSqlRangeBarDb::fetchOhlcArrayLastTo(const o3d::String &brokerId,
                                                 const o3d::String &marketId,
                                                 o3d::Int32 barSize,
                                                 o3d::Int32 lastN,
                                                 o3d::Double to,
                                                 OhlcArray &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-array-last-to-range-bar");
    if (!query) {
        return 0;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setInt64(2, barSize);
    query->setInt64(3, static_cast<o3d::Int64>(to * 1000.0));
    query->setInt64(4, lastN);

    query->execute();

    o3d::Int32 n = query->getNumRows();
    o3d::Int32 m = 0;

    if (n <= 0) {
        return 0;
    }

    std::list<Ohlc> reversed;

    while (query->fetch()) {
        Ohlc ohlc;

        ohlc.setTimestamp(query->getOut("timestamp").toDouble() * 0.001);
        ohlc.setDuration(query->getOut("duration").toDouble() * 0.001);

        ohlc.setO(query->getOut("open").toDouble());
        ohlc.setH(query->getOut("high").toDouble());
        ohlc.setL(query->getOut("low").toDouble());
        ohlc.setC(query->getOut("close").toDouble());

        ohlc.setVolume(query->getOut("volume").toDouble());

        if (ohlc.absHeight() >= static_cast<o3d::Double>(barSize)) {
            ohlc.setConsolidated();
        }

        reversed.push_front(ohlc);

        ++m;
    }

    for (auto it = reversed.begin(); it != reversed.end(); ++it) {
        out.push(*it);
    }

    O3D_ASSERT(n == m);
    return n;
}

o3d::Int32 PgSqlRangeBarDb::fetchOhlcArrayFrom(const o3d::String &brokerId,
                                               const o3d::String &marketId,
                                               o3d::Int32 barSize,
                                               o3d::Double from,
                                               OhlcArray &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-array-from-range-bar");
    if (!query) {
        return 0;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setInt64(2, barSize);
    query->setInt64(3, static_cast<o3d::Int64>(from * 1000.0));

    query->execute();

    o3d::Int32 n = query->getNumRows();
    o3d::Int32 m = 0;

    if (n <= 0) {
        return 0;
    }

    while (query->fetch()) {
        Ohlc ohlc;

        ohlc.setTimestamp(query->getOut("timestamp").toDouble() * 0.001);
        ohlc.setDuration(query->getOut("duration").toDouble() * 0.001);

        ohlc.setO(query->getOut("open").toDouble());
        ohlc.setH(query->getOut("high").toDouble());
        ohlc.setL(query->getOut("low").toDouble());
        ohlc.setC(query->getOut("close").toDouble());

        ohlc.setVolume(query->getOut("volume").toDouble());

        if (ohlc.absHeight() >= static_cast<o3d::Double>(barSize)) {
            ohlc.setConsolidated();
        }

        out.push(ohlc);

        ++m;
    }

    O3D_ASSERT(n == m);
    return n;
}

o3d::Bool PgSqlRangeBarDb::getLastOhlc(
        const o3d::String &brokerId,
        const o3d::String &marketId,
        o3d::Int32 barSize,
        Ohlc &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-last-range-bar");
    if (!query) {
        return false;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setInt64(2, barSize);

    query->execute();
    query->fetch();

    if (query->getNumRows() != 1) {
        return false;
    }

    Ohlc ohlc;

    o3d::Double timestamp = query->getOut("timestamp").toDouble() * 0.001;  // as second
    out.setTimestamp(timestamp);
    out.setDuration(query->getOut("duration").toDouble() * 0.001);

    out.setO(query->getOut("open").toDouble());
    out.setH(query->getOut("high").toDouble());
    out.setL(query->getOut("low").toDouble());
    out.setC(query->getOut("close").toDouble());

    out.setVolume(query->getOut("volume").toDouble());

    if (out.absHeight() >= static_cast<o3d::Double>(barSize)) {
        out.setConsolidated();
    }

    return true;
}
