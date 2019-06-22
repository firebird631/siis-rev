/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#ifndef SIIS_CONNECTOR_H
#define SIIS_CONNECTOR_H

#include "subscription.h"
#include "../order.h"
#include "../position.h"

#include <o3d/core/string.h>

namespace siis {

class TraderProxy;
class Handler;
class Order;
class Position;

/**
 * @brief SiiS strategy connector interface.
 * @author Frederic Scherma
 * @date 2019-03-07
 * At connection it will default subscribe account blance, order, position and asset balance updates.
 * Additionals subscriptions are for ticks and OHLC prices.
 */
class SIIS_API Connector
{
public:

    virtual ~Connector() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void connect() = 0;
    virtual void disconnect() = 0;

    /**
     * @brief subscribe Subscribe to a market data (tick, ohlc, order book).
     * @note sub.id is modified.
     */
    virtual void subscribe(Subscription &sub) = 0;

    /**
     * @brief unsubscribe Retrieve the subscription id according to the type and other details and unsubscribe.
     */
    virtual void unsubscribe(Subscription &sub) = 0;

    virtual void unsubscribeAll() = 0;

    virtual TraderProxy* traderProxy() = 0;
    virtual const TraderProxy* traderProxy() const = 0;

    virtual Handler* handler() = 0;
    virtual const Handler* handler() const = 0;

   //
    // orders functions
    //

    virtual std::list<Order*> listOrders() = 0;
    virtual std::list<Position*> listPositions() = 0;
    virtual o3d::UInt32 createOrder(const o3d::String marketId, 
                                    o3d::Int32 direction, 
                                    Order::OrderType orderType, 
                                    o3d::Double orderPrice, 
                                    o3d::Double quantity, 
                                    o3d::Double stopLossPrice,
                                    o3d::Double takeProfitPrice,
                                    o3d::Double leverage = 1.0) = 0;
    virtual void cancelOrder(o3d::UInt32 orderId) = 0;
    virtual void closePosition(o3d::UInt32 positionId) = 0;
    virtual void modifyPosition(o3d::UInt32 positionId,
                                o3d::Double stopLossPrice,
                                o3d::Double takeProfitPrice) = 0;


};

} // namespace siis

#endif // SIIS_CONNECTOR_H
