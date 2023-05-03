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
    return false;
}

o3d::Int32 PgSqlOhlcDb::fetchOhlcArray(const o3d::String &brokerId,
                                       const o3d::String &marketId,
                                       o3d::Float timeframe,
                                       o3d::Double from,
                                       o3d::Double to,
                                       OhlcArray &out)
{
    return 0;
}

Ohlc PgSqlOhlcDb::getLastOhlc(const o3d::String &brokerId, const o3d::String &marketId, o3d::Float timeframe)
{
    o3d::DbQuery *query = m_db->db()->findQuery("get-last-ohlc");
    if (!query) {
        return Ohlc();
    }

    query->setCString(0, brokerId.toUtf8());
    query->setCString(1, marketId.toUtf8());
    query->setDouble(2, timeframe);

    query->execute();
    if (query->fetch()) {
        // fetch one row
    }

    if (query->getNumRows() != 1) {
        return Ohlc();
    }

    Ohlc ohlc;

    o3d::Double timestamp = query->getOut("timestamp").toDouble() * 0.001;  // as second
    ohlc.setTimestamp(timestamp);
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

    return ohlc;
}
