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
    m_db->db()->registerQuery("get-last-ohlc",
                              R"SQL(SELECT timestamp, open, high, low, close, spread, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND timeframe = $3 ORDER BY timestamp DESC LIMIT 1)SQL");

    m_db->db()->registerQuery("get-ohlc",
                              R"SQL(SELECT timestamp, open, high, low, close, spread, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND timeframe = $3 AND timestamp = $4)SQL");

    m_db->db()->registerQuery("get-array-from-to-ohlc",
                              R"SQL(SELECT timestamp, open, high, low, close, spread, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND timeframe = $3 AND timestamp >= $4 AND timestamp <= $5 ORDER BY timestamp ASC)SQL");

    m_db->db()->registerQuery("get-array-last-ohlc",
                              R"SQL(SELECT timestamp, open, high, low, close, spread, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND timeframe = $3 ORDER BY timestamp DESC LIMIT $4)SQL");

    m_db->db()->registerQuery("get-array-last-to-ohlc",
                              R"SQL(SELECT timestamp, open, high, low, close, spread, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND timeframe = $3 AND timestamp <= $4 ORDER BY timestamp DESC LIMIT $5)SQL");

    m_db->db()->registerQuery("get-array-from-ohlc",
                              R"SQL(SELECT timestamp, open, high, low, close, spread, volume FROM ohlc
                                WHERE broker_id = $1 AND market_id = $2 AND timeframe = $3 AND timestamp >= $4 ORDER BY timestamp ASC)SQL");
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

o3d::Bool PgSqlOhlcDb::fetchOhlc(const o3d::String &brokerId,
                                 const o3d::String &marketId,
                                 o3d::Float timeframe,
                                 o3d::Float timestamp,
                                 Ohlc &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-ohlc");
    if (!query) {
        return false;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setDouble(2, timeframe);
    query->setInt64(3, static_cast<o3d::Int64>(timestamp * 1000.0));

    query->execute();
    query->fetch();

    if (query->getNumRows() != 1) {
        return false;
    }

    out.setTimestamp(timestamp);
    out.setTimeframe(timeframe);

    out.setO(query->getOut("open").toDouble());
    out.setH(query->getOut("high").toDouble());
    out.setL(query->getOut("low").toDouble());
    out.setC(query->getOut("close").toDouble());

    // out.setSpread(query->getOut("spread").toDouble());
    out.setVolume(query->getOut("volume").toDouble());

    // @todo
    // if (out.timestamp() < Instrument.basetime(timeframe, time.time()){
    // }
    out.setConsolidated();

    return true;
}

o3d::Int32 PgSqlOhlcDb::fetchOhlcArray(const o3d::String &brokerId,
                                       const o3d::String &marketId,
                                       o3d::Float timeframe,
                                       o3d::Double from,
                                       o3d::Double to,
                                       OhlcArray &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-array-ohlc");
    if (!query) {
        return 0;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setDouble(2, timeframe);
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
        ohlc.setTimeframe(timeframe);

        ohlc.setO(query->getOut("open").toDouble());
        ohlc.setH(query->getOut("high").toDouble());
        ohlc.setL(query->getOut("low").toDouble());
        ohlc.setC(query->getOut("close").toDouble());

        // ohlc.setSpread(query->getOut("spread").toDouble());
        ohlc.setVolume(query->getOut("volume").toDouble());

        // @todo
        // if (ohlc.timestamp() < Instrument.basetime(timeframe, time.time()){
        // }
        ohlc.setConsolidated();

        out.push(ohlc);

        ++m;
    }

    O3D_ASSERT(n == m);
    return n;
}

o3d::Int32 PgSqlOhlcDb::fetchOhlcArray(const o3d::String &brokerId,
                                       const o3d::String &marketId,
                                       o3d::Float timeframe,
                                       o3d::Int32 lastN,
                                       OhlcArray &out)
{
    // take care inverted results
    o3d::DbQuery *query = m_db->db()->findQuery("get-array-last-ohlc");
    if (!query) {
        return 0;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setDouble(2, timeframe);
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
        ohlc.setTimeframe(timeframe);

        ohlc.setO(query->getOut("open").toDouble());
        ohlc.setH(query->getOut("high").toDouble());
        ohlc.setL(query->getOut("low").toDouble());
        ohlc.setC(query->getOut("close").toDouble());

        // ohlc.setSpread(query->getOut("spread").toDouble());
        ohlc.setVolume(query->getOut("volume").toDouble());

        // @todo
        // if (ohlc.timestamp() < Instrument.basetime(timeframe, time.time()){
        // }
        ohlc.setConsolidated();

        reversed.push_front(ohlc);

        ++m;
    }

    for (auto it = reversed.begin(); it != reversed.end(); ++it) {
        out.push(*it);
    }

    O3D_ASSERT(n == m);
    return n;
}

o3d::Int32 PgSqlOhlcDb::fetchOhlcArray(const o3d::String &brokerId,
                                       const o3d::String &marketId,
                                       o3d::Float timeframe,
                                       o3d::Int32 lastN,
                                       o3d::Double to,
                                       OhlcArray &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-array-last-to-ohlc");
    if (!query) {
        return 0;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setDouble(2, timeframe);
    query->setInt64(3, lastN);
    query->setInt64(4, static_cast<o3d::Int64>(to * 1000.0));

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
        ohlc.setTimeframe(timeframe);

        ohlc.setO(query->getOut("open").toDouble());
        ohlc.setH(query->getOut("high").toDouble());
        ohlc.setL(query->getOut("low").toDouble());
        ohlc.setC(query->getOut("close").toDouble());

        // out.setSpread(query->getOut("spread").toDouble());
        ohlc.setVolume(query->getOut("volume").toDouble());

        // @todo
        // if (out.timestamp() < Instrument.basetime(timeframe, time.time()){
        // }
        ohlc.setConsolidated();

        reversed.push_front(ohlc);

        ++m;
    }

    for (auto it = reversed.begin(); it != reversed.end(); ++it) {
        out.push(*it);
    }

    O3D_ASSERT(n == m);
    return n;
}

o3d::Int32 PgSqlOhlcDb::fetchOhlcArrayFrom(const o3d::String &brokerId,
                                       const o3d::String &marketId,
                                        o3d::Float timeframe,
                                        o3d::Double from,
                                        OhlcArray &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-array-from-ohlc");
    if (!query) {
        return 0;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setDouble(2, timeframe);
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
        ohlc.setTimeframe(timeframe);

        ohlc.setO(query->getOut("open").toDouble());
        ohlc.setH(query->getOut("high").toDouble());
        ohlc.setL(query->getOut("low").toDouble());
        ohlc.setC(query->getOut("close").toDouble());

        // out.setSpread(query->getOut("spread").toDouble());
        ohlc.setVolume(query->getOut("volume").toDouble());

        // @todo
        // if (out.timestamp() < Instrument.basetime(timeframe, time.time()){
        // }
        ohlc.setConsolidated();
        out.push(ohlc);

        ++m;
    }

    O3D_ASSERT(n == m);
    return n;
}

o3d::Bool PgSqlOhlcDb::getLastOhlc(
        const o3d::String &brokerId,
        const o3d::String &marketId,
        o3d::Float timeframe,
        Ohlc &out)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-last-ohlc");
    if (!query) {
        return false;
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setDouble(2, timeframe);

    query->execute();
    query->fetch();

    if (query->getNumRows() != 1) {
        return false;
    }

    Ohlc ohlc;

    o3d::Double timestamp = query->getOut("timestamp").toDouble() * 0.001;  // as second
    out.setTimestamp(timestamp);
    out.setTimeframe(timeframe);

    out.setO(query->getOut("open").toDouble());
    out.setH(query->getOut("high").toDouble());
    out.setL(query->getOut("low").toDouble());
    out.setC(query->getOut("close").toDouble());

    // out.setSpread(query->getOut("spread").toDouble());
    out.setVolume(query->getOut("volume").toDouble());

    // @todo
    // if (out.timestamp() < Instrument.basetime(timeframe, time.time()){
    // }
    out.setConsolidated();

    return true;
}
