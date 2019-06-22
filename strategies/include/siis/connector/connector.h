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

    /**
     * @brief setTraderProxy Define the trader proxy related to this connector or nullptr.
     * @param traderProxy Valid connector proxy, will not be deleted, or nullptr if no trader proxy for this connector.
     */
    virtual void setTraderProxy(TraderProxy *traderProxy) = 0;

    virtual TraderProxy* traderProxy() = 0;
    virtual const TraderProxy* traderProxy() const = 0;

    virtual Handler* handler() = 0;
    virtual const Handler* handler() const = 0;

    //
    // order
    //

    virtual void fetchAnyOrders() = 0;
    virtual void fetchOrder(const o3d::String& marketId) = 0;

    virtual o3d::Int32 createOrder(Order *order) = 0;

    virtual o3d::Int32 cancelOrder(const o3d::String &orderId) = 0;

    //
    // position
    //

    virtual void fetchAnyPositions() = 0;
    virtual void fetchPositions(const o3d::String& marketId) = 0;

    virtual o3d::Int32 closePosition(const o3d::String &positionId) = 0;

    virtual o3d::Int32 modifyPosition(
            const o3d::String &positionId,
            o3d::Double stopLossPrice,
            o3d::Double takeProfitPrice) = 0;

    //
    // account
    //

    virtual void fetchAccount() = 0;

    //
    // asset
    //

    virtual void fetchAnyAssets() = 0;

    virtual void fetchAssets(const o3d::String& assetId) = 0;
};

} // namespace siis

#endif // SIIS_CONNECTOR_H
