/**
 * @brief SiiS strategy OHLC database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_PGSQLOHLCDB_H
#define SIIS_PGSQLOHLCDB_H

#include "siis/database/ohlcdb.h"

namespace siis {

class PgSql;

/**
 * @brief Strategy OHLC database DAO.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API PgSqlOhlcDb : public OhlcDb
{
public:

    PgSqlOhlcDb(siis::PgSql *db);

    virtual ~PgSqlOhlcDb();

    virtual Database* db();
    virtual const Database* db() const;

    virtual o3d::Bool fetchOhlc(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Float timeframe, o3d::Float timestamp,
            Ohlc &out);

    virtual o3d::Int32 fetchOhlcArray(
            const o3d::String &brokerId, const o3d::String &marketId,
            o3d::Float timeframe,
            o3d::Double from, o3d::Double to,
            OhlcArray &out);

    // virtual void storeOhlc(const o3d::String &brokerId, const o3d::String &marketId, const Ohlc &ohlc);

private:

    PgSql *m_db;
};

} // namespace siis

#endif // SIIS_PGSQLOHLCDB_H
