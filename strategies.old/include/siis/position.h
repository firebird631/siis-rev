/**
 * @brief SiiS strategy position signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_POSITION_H
#define SIIS_POSITION_H

#include "constants.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief Strategy position .
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API Position
{
public:

    static constexpr o3d::Double TIMESTAMP_UNDEFINED = -1.0;
    static constexpr o3d::Double PRICE_UNDEFINED = -1.0;
    static constexpr o3d::Double QUANTITY_UNDEFINED = -1.0;
    static constexpr o3d::Double RATE_UNDEFINED = -1.0;
    static constexpr o3d::Int8 FLAG_UNDEFINED = -1;
    static constexpr o3d::Int8 VALUE_UNDEFINED = -1;

    Position() :
        timestamp(TIMESTAMP_UNDEFINED),
        id(-1),
        direction(UNDEFINED),
        quantity(QUANTITY_UNDEFINED),
        avgPrice(PRICE_UNDEFINED),
        execPrice(PRICE_UNDEFINED),
        stopLossPrice(PRICE_UNDEFINED),
        limitPrice(PRICE_UNDEFINED),
        profitLoss(RATE_UNDEFINED),
        filled(QUANTITY_UNDEFINED),
        cumulativeFilled(QUANTITY_UNDEFINED),
        liquidationPrice(PRICE_UNDEFINED),
        commission(RATE_UNDEFINED)
    {
    }

    o3d::Double timestamp;   //!< operation timestamp

    o3d::Int32 id;           //!< internal integer unique id

    o3d::String positionId;  //!< must always be valid
    o3d::String marketId;    //!< empty means not defined

    o3d::Int32 direction;    //!< -1 or 1, 0 means not defined
    o3d::Double quantity;    //!< always positive or 0

    o3d::Double avgPrice;    //!< average price or -1 if not defined
    o3d::Double execPrice;   //!< executed price or -1 if not defined

    o3d::Double stopLossPrice;    //! -1 if not defined
    o3d::Double limitPrice;       //! -1 if not defined

    o3d::String profitCurrency;   //!< currency symbol
    o3d::Double profitLoss;  //!< unrealized profit/loss in profit currency

    o3d::Double filled;      //!< filled qty or neg if not defined
    o3d::Double cumulativeFilled;   //!< cumulative filled qty or neg if not defined

    o3d::Double liquidationPrice;   //!< liquidation price, neg if not defined
    o3d::Double commission;  //!< commission rate realized on the position
};

} // namespace siis

#endif // SIIS_POSITION_H
