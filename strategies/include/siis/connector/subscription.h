/**
 * @brief SiiS strategy subscription.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#ifndef SIIS_SUBSCRIPTION_H
#define SIIS_SUBSCRIPTION_H

#include "../base.h"
#include <o3d/core/string.h>

namespace siis {

/**
 * @brief Siis strategy connector subscription model.
 */
class SIIS_API Subscription
{
public:

    enum Type {
        TYPE_TICK = 0,
        TICK_OHLC_MID = 1,
        TICK_OHLC_BID = 2,
        TICK_OHLC_OFR = 3,
        TICK_ORDER_BOOK = 4
    };

    Subscription() :
        id(-1),
        type(TYPE_TICK),
        marketId(),
        timeframe(0),
        orderBookdepth(0)
    {
    }

    Subscription(const Subscription &dup) :
        id(dup.id),
        type(dup.type),
        marketId(dup.marketId),
        timeframe(dup.timeframe),
        orderBookdepth(dup.orderBookdepth)
    {
    }

    o3d::Int32 id;  //!< leave it blank, defined by the connector

    Type type;
    o3d::String marketId;
    o3d::Double timeframe;
    o3d::Int32 orderBookdepth;
};

} // namespace siis

#endif // SIIS_SUBSCRIPTION_H
