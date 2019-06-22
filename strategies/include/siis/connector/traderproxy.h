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
#include "../position.h"
#include "../asset.h"

#include "../trade/trade.h"

#include <list>

#include <o3d/core/stringmap.h>
#include <o3d/core/templatearray.h>

namespace siis {

class Connector;
class StatusSignal;
class AccountSignal;
class AssetSignal;
class MarketSignal;
class OrderSignal;
class PositionSignal;

class Market;

/**
 * @brief Strategy trader proxy.
 * @author Frederic Scherma
 * @date 2019-03-17
 * By default pre-allocate 100 orders, 100 trade of each type (asset, margin, ind-margin).
 */
class SIIS_API TraderProxy
{
public:

    static constexpr o3d::Int32 BUCKET_SIZE = 100;

    TraderProxy(Connector *connector);

    ~TraderProxy();

    //
    // global
    //

    o3d::Bool alive() const;

    //
    // trade
    //

    /**
     * @brief createTrade Create a new StrategyTrade object according to the type of the market.
     */
    Trade* createTrade(Market *market, o3d::Double timeframe);

    /**
     * @brief createTrade Not the prefered way because it have to lookup for the market trought the handler.
     */
    Trade* createTrade(const o3d::String &marketId, o3d::Double timeframe);

    //
    // order
    //

    o3d::Int32 createOrder(Order *order);

    void cancelOrder(const o3d::String &orderId);

    /**
     * @brief newOrder Returns a new free order to be used with createOrder.
     */
    Order* newOrder();

    /**
     * @brief freeOrder Once an order message is process release the order with this method.
     * @param order Valid order to release.
     */
    void freeOrder(Order *order);

    /**
     * @brief findOrder Return the local version of an order previously fetched/updated
     */
    Order* getOrder(const o3d::String &orderId);

    //
    // position
    //

    o3d::Int32 closePosition(const o3d::String &positionId);

    o3d::Int32 modifyPosition(
            const o3d::String &positionId,
            o3d::Double stopLossPrice,
            o3d::Double takeProfitPrice);

    /**
     * @brief findOrder Return the local version of a position previously fetched/updated
     */
    Position* getPosition(const o3d::String &positionId);

    //
    // account
    //

    /**
     * @brief freeMargin Get account free margin.
     */
    o3d::Double freeMargin() const;

    //
    // asset
    //

    /**
     * @brief freeAssetQuantity Get asset free quantity.
     */
    o3d::Double freeAssetQuantity(const o3d::String &assetId) const;

    /**
     * @brief freeAssetQuantity Get asset locked quantity.
     */
    o3d::Double lockedAssetQuantity(const o3d::String &assetId) const;

    //
    // signals
    //

    void onAccountSignal(const AccountSignal &signal);
    void onAssetSignal(const AssetSignal &signal);
    void onMarketSignal(const MarketSignal &signal);
    void onOrderSignal(const OrderSignal &signal);
    void onPositionSignal(const PositionSignal &signal);
    void onStatusSignal(const StatusSignal &signal);

protected:

    Connector *m_connector;

    o3d::Bool m_alive;  //!< true if connection on the client connector is alive

    o3d::Double m_freeMargin;
    o3d::Double m_reservedMargin;
    o3d::Double m_marginFactor;

    o3d::StringMap<Asset> m_assets;

    o3d::TemplateArray<Trade*> m_freeTrades[Trade::NUM_TYPES];
    o3d::TemplateArray<Order*> m_freeOrders;
};

} // namespace siis

#endif // SIIS_TRADERPROXY_H
