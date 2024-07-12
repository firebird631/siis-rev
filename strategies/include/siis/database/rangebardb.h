/**
 * @brief SiiS strategy range-bar database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-12
 */

#ifndef SIIS_RANGEBARDB_H
#define SIIS_RANGEBARDB_H

#include "siis/ohlc.h"
#include "siis/database/database.h"

namespace siis {

/**
 * @brief Strategy OHLC range-bar database DAO.
 * @author Frederic Scherma
 * @date 2024-07-12
 */
class SIIS_API RangeBarDb
{
public:

    virtual ~RangeBarDb() = 0;

    virtual siis::Database* db() = 0;
    virtual const siis::Database* db() const = 0;

    virtual o3d::Bool fetchOhlc(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize, o3d::Double timestamp,
            Ohlc &out) = 0;

    virtual o3d::Int32 fetchOhlcArrayFromTo(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize,
            o3d::Double from, o3d::Double to,
            OhlcArray &out) = 0;

    virtual o3d::Int32 fetchOhlcArrayLast(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize,
            o3d::Int32 lastN,
            OhlcArray &out) = 0;

    virtual o3d::Int32 fetchOhlcArrayLastTo(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize,
            o3d::Int32 lastN, o3d::Double to,
            OhlcArray &out) = 0;

    virtual o3d::Int32 fetchOhlcArrayFrom(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize,
            o3d::Double from,
            OhlcArray &out) = 0;

    virtual o3d::Bool getLastOhlc(const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Int32 barSize, Ohlc &out) = 0;

    // virtual void storeOhlc(const o3d::String &brokerId, const o3d::String &marketId, const Ohlc &ohlc) = 0;
};

} // namespace siis

#endif // SIIS_RANGEBARDB_H
