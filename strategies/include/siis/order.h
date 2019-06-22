/**
 * @brief SiiS strategy order signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_ORDER_H
#define SIIS_ORDER_H

#include "constants.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief Strategy order.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API Order
{
public:

    enum OrderType {
        ORDER_UNDEFINED = -1,        //!< undefined value
        ORDER_MARKET = 0,            //!< market order
        ORDER_LIMIT = 1,             //!< entry or exit limit
        ORDER_STOP = 2,              //!< stop at market
        ORDER_STOP_LIMIT = 3,        //!<  stop limit order
        ORDER_TAKE_PROFIT = 4,       //!< take profit market
        ORDER_TAKE_PROFIT_LIMIT = 5  //!<  take profit limit
    };

    enum TimeInForce {
        TIME_IN_FORCE_UNDEFINED = -1,
        TIME_IN_FORCE_GTC = 0,  //!< Good till cancelled
        TIME_IN_FORCE_IOC = 1,  //!< Immediate or cancel
        TIME_IN_FORCE_FOK = 2   //!< Fill or kill
    };

    enum PriceType {
        PRICE_TYPE_UNDEFINED = -1,
        PRICE_TYPE_LAST = 0,    //!< Use last market price
        PRICE_TYPE_INDEX = 1,   //!< Use last index price
        PRICE_TYPE_MARK = 2     //!< Use last mark price
    };

    static constexpr o3d::Double TIMESTAMP_UNDEFINED = -1.0;
    static constexpr o3d::Double PRICE_UNDEFINED = -1.0;
    static constexpr o3d::Double QUANTITY_UNDEFINED = -1.0;
    static constexpr o3d::Double RATE_UNDEFINED = -1.0;
    static constexpr o3d::Int8 FLAG_UNDEFINED = -1;
    static constexpr o3d::Int8 VALUE_UNDEFINED = -1;

    Order() :
        timestamp(TIMESTAMP_UNDEFINED),
        id(-1),
        orderType(ORDER_UNDEFINED),
        direction(UNDEFINED),
        orderQuantity(QUANTITY_UNDEFINED),
        orderPrice(PRICE_UNDEFINED),
        stopLossPrice(PRICE_UNDEFINED),
        limitPrice(PRICE_UNDEFINED),
        timeInForce(TIME_IN_FORCE_UNDEFINED),
        priceType(PRICE_TYPE_UNDEFINED),
        postOnly(FLAG_UNDEFINED),
        closeOnly(FLAG_UNDEFINED),
        execPrice(PRICE_UNDEFINED),
        avgPrice(PRICE_UNDEFINED),
        filled(QUANTITY_UNDEFINED),
        cumulativeFilled(QUANTITY_UNDEFINED),
        commissionAmount(QUANTITY_UNDEFINED)
    {
    }

    o3d::Double timestamp;   //!< timestamp of the operation

    o3d::Int32 id;           //!< internal integer unique id

    o3d::String orderId;     //!< must always be valid
    o3d::String orderRefId;  //!< id of the order that as referenced this one

    o3d::String marketId;    //!< empty means not defined

    OrderType orderType;
    o3d::Int32 direction;

    o3d::Double orderQuantity;   //!< initial order quantity
    o3d::Double orderPrice;      //!< initial order price (for limit)

    o3d::Double stopLossPrice;   //!< for position creation order, 0 or -1 if not defined
    o3d::Double limitPrice;      //!< for position creation order, 0 or -1 if not defined

    TimeInForce timeInForce;
    PriceType priceType;

    o3d::Int8 postOnly;    //!< means maker fee only, else invalidate the trade
    o3d::Int8 closeOnly;   //!< mean only reduce an existing position, cannot increase or create

    o3d::String tradeId;

    o3d::Double execPrice;
    o3d::Double avgPrice;

    o3d::Double filled;
    o3d::Double cumulativeFilled;
    o3d::Double quoteTransacted;

    o3d::Double commissionAmount;
    o3d::String commissionAsset;
};

} // namespace siis

#endif // SIIS_ORDER_H
