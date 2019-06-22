/**
 * @brief SiiS strategy database connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_DATABASE_H
#define SIIS_DATABASE_H

#include "siis/base.h"

#include <o3d/core/string.h>

namespace siis {

class OhlcDb;
class MarketDb;
class TradeDb;

/**
 * @brief Strategy database connector interface.
 * @author Frederic Scherma
 * @date 2019-03-07
 */
class SIIS_API Database
{
public:

    static Database* builder(
            const o3d::String &db,
            const o3d::String &host,
            o3d::UInt32 port,
            const o3d::String &name,
            const o3d::String &user,
            const o3d::String &pwd);

    virtual ~Database() = 0;

    virtual void terminate() = 0;

    virtual OhlcDb* buildOhlcDb() = 0;
    virtual MarketDb* buildMarketDb() = 0;
    virtual TradeDb* buildTradeDb() = 0;
};

} // namespace siis

#endif // SIIS_DATABASE_H
