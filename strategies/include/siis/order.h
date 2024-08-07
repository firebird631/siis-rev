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

class TraderProxy;
class Strategy;

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

    /**
     * @brief The Flags enum 5 bits, cannot exceed 7 bits, -1 means undefined
     */
    enum Flags {
        F_POST_ONLY = 1,      //!< cannot only be executed has maker (Participate Do Not Initiate)
        F_REDUCE_ONLY = 2,    //!< only allowed to reduce the size of the position
        F_CLOSE_ONLY = 4,     //!< close and reduce position size only allowed
        F_HEDGING = 8,        //!< force position mean heding
        F_MARGIN = 16         //!< create a margin order when spot and margin are on the same instrument
    };

    static constexpr o3d::Double TIMESTAMP_UNDEFINED = -1.0;
    static constexpr o3d::Double PRICE_UNDEFINED = -1.0;
    static constexpr o3d::Double QUANTITY_UNDEFINED = -1.0;
    static constexpr o3d::Double FEE_UNDEFINED = O3D_MIN_DOUBLE;
    static constexpr o3d::Double RATE_UNDEFINED = O3D_MIN_DOUBLE;
    static constexpr o3d::Int8 FLAG_UNDEFINED = -1;
    static constexpr o3d::Int8 VALUE_UNDEFINED = -1;

    enum ReturnCode {
        RET_OK = 1,                     //!< Order successfully open
        RET_UNDEFINED = 0,              //!< Undefined
        RET_INSUFFICIENT_FUNDS = -1,    //!< Insufficient asset quantity to open order (permanent)
        RET_INSUFFICIENT_MARGIN = -2,   //!< Insufficient margin to open order (permanent)
        RET_ERROR = -3,                 //!< General error or unspecified (permanent)
        RET_INVALID_ARGS = -4,          //!< Invalid order arguments (permanent)
        RET_DENIED = -5,                //!< User or API key or sign not allowed (permanent)
        RET_UNREACHABLE_SERVICE = -32,  //!< Service is currently or permanently unreachable (temporary)
        RET_RATE_LIMIT = -33,           //!< API rate limit exceeded (temporary)
        RET_ORDER_LIMIT = -34,          //!< Number of order limit exceeded (temporary)
        RET_POSITION_LIMIT = -35,       //!< Number of position limit exceeded (temporary)
        RET_INVALID_NONCE = -36,        //!< Wrong nonce value (temporary)
        RET_CANCEL_ONLY = -37,          //!< Cancel only mode (temporary)
        RET_POST_ONLY = -38             //!< Post-only mode (temporary)
    };

    Order() :
        proxy(nullptr),
        strategy(nullptr),
        created(TIMESTAMP_UNDEFINED),
        executed(TIMESTAMP_UNDEFINED),
        id(-1),
        orderType(ORDER_UNDEFINED),
        direction(UNDEFINED),
        orderQuantity(QUANTITY_UNDEFINED),
        orderPrice(PRICE_UNDEFINED),
        orderStopPrice(PRICE_UNDEFINED),
        limitPrice(PRICE_UNDEFINED),
        stopPrice(PRICE_UNDEFINED),
        timeInForce(TIME_IN_FORCE_UNDEFINED),
        priceType(PRICE_TYPE_UNDEFINED),
        flags(FLAG_UNDEFINED),
        execPrice(PRICE_UNDEFINED),
        avgPrice(PRICE_UNDEFINED),
        filled(QUANTITY_UNDEFINED),
        cumulativeFilled(QUANTITY_UNDEFINED),
        quoteTransacted(QUANTITY_UNDEFINED),
        commissionAmount(FEE_UNDEFINED),
        cumulativeCommissionAmount(FEE_UNDEFINED),
        maker(VALUE_UNDEFINED)
    {
    }

    void reset()
    {
        strategy = nullptr;
        created = TIMESTAMP_UNDEFINED;
        executed = TIMESTAMP_UNDEFINED;
        id = -1;
        orderId = "";
        refId = "";
        positionId = "";
        marketId = "";
        orderType = ORDER_UNDEFINED;
        direction = UNDEFINED;
        orderQuantity = QUANTITY_UNDEFINED;
        orderPrice = PRICE_UNDEFINED;
        orderStopPrice = PRICE_UNDEFINED;
        limitPrice = PRICE_UNDEFINED;
        stopPrice = PRICE_UNDEFINED;
        timeInForce = TIME_IN_FORCE_UNDEFINED;
        priceType = PRICE_TYPE_UNDEFINED;
        flags = FLAG_UNDEFINED;
        tradeId = "";
        execPrice = PRICE_UNDEFINED;
        avgPrice = PRICE_UNDEFINED;
        filled = QUANTITY_UNDEFINED;
        cumulativeFilled = QUANTITY_UNDEFINED;
        quoteTransacted = QUANTITY_UNDEFINED;
        commissionAmount = FEE_UNDEFINED;
        cumulativeCommissionAmount = FEE_UNDEFINED;
        commissionAsset = "";
        maker = VALUE_UNDEFINED;
    }

    inline void defineFlag(o3d::Int8 v) { flags = flags < 0 ? v : flags | v; }

    inline void setPostOnly() { defineFlag(F_POST_ONLY); }
    inline void setReduceOnly() { defineFlag(F_REDUCE_ONLY); }
    inline void setCloseOnly() { defineFlag(F_CLOSE_ONLY); }
    inline void setHedging() { defineFlag(F_HEDGING); }
    inline void setMargin() { defineFlag(F_MARGIN); }

    o3d::Bool postOnly() const { return flags > 0 && flags & F_POST_ONLY; }
    o3d::Bool reduceOnly() const { return flags > 0 && flags & F_REDUCE_ONLY; }
    o3d::Bool closeOnly() const { return flags > 0 && flags & F_CLOSE_ONLY; }
    o3d::Bool hedging() const { return flags > 0 && flags & F_HEDGING; }
    o3d::Bool margin() const { return flags > 0 && flags & F_MARGIN; }

    TraderProxy *proxy;      //!< must be valid
    Strategy *strategy;      //!< must be valid

    o3d::Double created;     //!< timestamp of the creation
    o3d::Double executed;    //!< timestamp of the last execution

    o3d::Int32 id;           //!< internal integer unique id

    o3d::CString orderId;    //!< to retrieve it from the distant exchange
    o3d::CString refId;      //!< id of the order that as referenced this one
    o3d::CString positionId; //!< created position id

    o3d::CString marketId;   //!< empty means not defined

    OrderType orderType;
    o3d::Int32 direction;

    o3d::Double orderQuantity;   //!< initial order quantity
    o3d::Double orderPrice;      //!< initial order price (for limit or )
    o3d::Double orderStopPrice;  //!< initial order price (for stop limit, take profit...)

    o3d::Double limitPrice;      //!< for position creation order, initial exit limit price, 0 or -1 if not defined
    o3d::Double stopPrice;       //!< for position creation order, initial exit stop price, 0 or -1 if not defined

    TimeInForce timeInForce;
    PriceType priceType;         //!< used price (last: default, index, mark)
    o3d::Int8 flags;             //!< contains post-only, close-reduce, reduce-only, hedging, margin flags

    o3d::CString tradeId;

    o3d::Double execPrice;
    o3d::Double avgPrice;

    o3d::Double filled;
    o3d::Double cumulativeFilled;
    o3d::Double quoteTransacted;

    o3d::Double commissionAmount;
    o3d::Double cumulativeCommissionAmount;
    o3d::CString commissionAsset;

    o3d::Int8 maker;        //!< maker or taker fee (-1 if not defined)
};

} // namespace siis

#endif // SIIS_ORDER_H
