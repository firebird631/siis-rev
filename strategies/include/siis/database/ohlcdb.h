/**
 * @brief SiiS strategy OHLC database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_OHLCDB_H
#define SIIS_OHLCDB_H

#include "siis/ohlc.h"
#include "siis/database/database.h"

namespace siis {

/**
 * @brief Strategy OHLC database DAO.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API OhlcDb
{
public:

    virtual ~OhlcDb() = 0;

    virtual siis::Database* db() = 0;
    virtual const siis::Database* db() const = 0;

    virtual o3d::Bool fetchOhlc(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Float timeframe, o3d::Float timestamp,
            Ohlc &out) = 0;

    virtual o3d::Int32 fetchOhlcArray(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Float timeframe,
            o3d::Double from, o3d::Double to,
            OhlcArray &out) = 0;

    // virtual void storeOhlc(const o3d::String &brokerId, const o3d::String &marketId, const Ohlc &ohlc) = 0;
};

} // namespace siis

#endif // SIIS_OHLCDB_H
