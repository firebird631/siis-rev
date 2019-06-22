/**
 * @brief SiiS strategy trader proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_TRADERPROXY_H
#define SIIS_TRADERPROXY_H

#include "../base.h"
#include "../order.h"

#include <list>
#include <o3d/core/base.h>

namespace siis {

class Connector;
class StatusSignal;
class AccountSignal;
class AssetSignal;
class MarketSignal;
class OrderSignal;
class PositionSignal;
class Position;

/**
 * @brief Strategy trader proxy.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API TraderProxy
{
public:

    enum Type {
        TYPE_STD = 0
    };

    static TraderProxy* build(Type type, Connector *connector);

    virtual ~TraderProxy() = 0;

    // @todo createOrder, cancelOrder, modifyOrder | getOrder, getAllOrders
    // @todo closePosition, modifyPosition | getPosition, getAllPositions
    // @todo getAccount
    // @todo getAsset | getAllAssets

    //
    // orders
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
    
    

    //
    // signals
    //

    virtual void onAccountSignal(const AccountSignal &signal) = 0;
    virtual void onAssetSignal(const AssetSignal &signal) = 0;
    virtual void onMarketSignal(const MarketSignal &signal) = 0;
    virtual void onOrderSignal(const OrderSignal &signal) = 0;
    virtual void onPositionSignal(const PositionSignal &signal) = 0;
    virtual void onStatusSignal(const StatusSignal &signal) = 0;
};

} // namespace siis

#endif // SIIS_TRADERPROXY_H
