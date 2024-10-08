/**
 * @brief SiiS strategy OHLC database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_MYSQLOHLCDB_H
#define SIIS_MYSQLOHLCDB_H

#include "siis/database/ohlcdb.h"

namespace siis {

class MySql;

/**
 * @brief Strategy OHLC database DAO.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API MySqlOhlcDb : public OhlcDb
{
public:

    MySqlOhlcDb(siis::MySql *db);

    virtual ~MySqlOhlcDb() override;

    virtual Database* db() override;
    virtual const Database* db() const override;

    virtual o3d::Bool fetchOhlc(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Double timeframe, o3d::Double timestamp,
            Ohlc &out) override;

    virtual o3d::Int32 fetchOhlcArrayFromTo(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Double timeframe,
            o3d::Double from, o3d::Double to,
            OhlcArray &out) override;

    virtual o3d::Int32 fetchOhlcArrayLast(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Double timeframe,
            o3d::Int32 lastN,
            OhlcArray &out) override;

    virtual o3d::Int32 fetchOhlcArrayLastTo(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Double timeframe,
            o3d::Int32 lastN,
            o3d::Double to,
            OhlcArray &out) override;

    virtual o3d::Int32 fetchOhlcArrayFrom(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Double timeframe,
            o3d::Double from,
            OhlcArray &out) override;

    virtual o3d::Bool getLastOhlc(const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Double timeframe, Ohlc &out) override;

    // virtual void storeOhlc(const o3d::String &brokerId, const o3d::String &marketId, const Ohlc &ohlc);

private:

    MySql *m_db;
};

} // namespace siis

#endif // SIIS_MYSQLOHLCDB_H
