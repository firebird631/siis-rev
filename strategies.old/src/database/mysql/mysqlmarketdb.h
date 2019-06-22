/**
 * @brief SiiS strategy market database DAO.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_MYSQLMARKETDB_H
#define SIIS_MYSQLMARKETDB_H

#include "siis/database/marketdb.h"

namespace siis {

class MySql;

/**
 * @brief Strategy market database DAO.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API MySqlMarketDb : public MarketDb
{
public:

    MySqlMarketDb(siis::MySql *db);

    virtual ~MySqlMarketDb();

    virtual Database* db();
    virtual const Database* db() const;

    virtual o3d::Bool fetchMarket(const o3d::String &brokerId, const o3d::String &marketId, Market &out);
    virtual std::list<Market*> fetchMarketList(const o3d::String &brokerId, const o3d::T_StringList &marketsId);

    // virtual void storeMarket(const o3d::String &brokerId, const Market &market);

private:

    MySql *m_db;
};

} // namespace siis

#endif // SIIS_MYSQLMARKETDB_H
