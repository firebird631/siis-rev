/**
 * @brief SiiS strategy range-bar database DAO.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_PGSQLRANGEBARDB_H
#define SIIS_PGSQLRANGEBARDB_H

#include "siis/database/rangebardb.h"

namespace siis {

class PgSql;

/**
 * @brief Strategy range-bar database DAO.
 * @author Frederic Scherma
 * @date 2024-07-12
 */
class SIIS_API PgSqlRangeBarDb : public RangeBarDb
{
public:

    PgSqlRangeBarDb(siis::PgSql *db);

    virtual ~PgSqlRangeBarDb() override;

    virtual Database* db() override;
    virtual const Database* db() const override;

    virtual o3d::Bool fetchOhlc(const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize, o3d::Double timestamp,
            Ohlc &out) override;

    virtual o3d::Int32 fetchOhlcArrayFromTo(const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize,
            o3d::Double from, o3d::Double to,
            OhlcArray &out) override;

    virtual o3d::Int32 fetchOhlcArrayLast(const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize,
            o3d::Int32 lastN,
            OhlcArray &out) override;

    virtual o3d::Int32 fetchOhlcArrayLastTo(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize,
            o3d::Int32 lastN,
            o3d::Double to,
            OhlcArray &out) override;

    virtual o3d::Int32 fetchOhlcArrayFrom(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize,
            o3d::Double from,
            OhlcArray &out) override;

    virtual o3d::Bool getLastOhlc(const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize, Ohlc &out) override;

    // virtual void storeOhlc(const o3d::String &brokerId, const o3d::String &marketId, o3d::Int32 barSize, const Ohlc &ohlc) override;

private:

    PgSql *m_db;
};

} // namespace siis

#endif // SIIS_PGSQLRANGEBARDB_H
