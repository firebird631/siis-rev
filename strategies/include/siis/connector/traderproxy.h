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
 * By default pre-allocate 100 orders, 100 trade of each type.
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
     * @brief createTrade Create a new trade object according to the type of the market.
     */
    Trade* createTrade(Market *market, Trade::Type tradeType, o3d::Double timeframe);

    /**
     * @brief createTrade Not the prefered way because it have to lookup for the market trought the handler.
     */
    Trade* createTrade(const o3d::CString &marketId, Trade::Type tradeType, o3d::Double timeframe);

    /**
     * @brief freeTrade Once a trade is processed, closed release with this method.
     * @param order Valid trade to release.
     */
    void freeTrade(Trade *trade);

    //
    // order
    //

    /**
     * @brief createOrder Send an order to the connector. It can be a local connector.
     * @param order Valid order instance.
     * @return Return code from the synchronous part of the request.
     */
    o3d::Int32 createOrder(Order *order);

    /**
     * @brief cancelOrder Send a cancel order to the connector. It can be a local connector.
     * @param orderId Valid order unique identifier.
     * @return Return code from the synchronous part of the request.
     */
    o3d::Int32 cancelOrder(const o3d::CString &orderId);

    /**
     * @brief newOrder Returns a new free order to be used with createOrder.
     */
    Order* newOrder(Strategy *strategy);

    /**
     * @brief freeOrder Once an order message or local execution is processed, release the order with this method.
     * @param order Valid order to release.
     */
    void freeOrder(Order *order);

    /**
     * @brief findOrder Return the local version of an order previously fetched/updated
     */
    Order* getOrder(const o3d::CString &orderId);

    //
    // position
    //

    /**
     * @brief newPosition Returns a new free position to be used with createPosition.
     */
    Position* newPosition(Strategy *strategy);

    /**
     * @brief freePosition Once a position message or local executions is processed, release the order with this method.
     * @param order Valid position to release.
     */
    void freePosition(Position *position);

    /**
     * @brief closePosition Close an existing position at limit or market.
     * @param positionId
     * @param direction Necessary, direction of the position to close.
     * @param quantity Quantity to close, can be a partial (reduce) quantity.
     * @param taker If true (default) close a market (in taker) else a limit price must be provided.
     * @param limitPrice Limit price to close as maker if taker is set to false.
     * @return
     */
    o3d::Int32 closePosition(const o3d::CString &positionId, o3d::Int32 direction, o3d::Double quantity,
                             o3d::Bool taker = true, o3d::Double limitPrice = 0.0);

    /**
     * @brief modifyPosition Modifit the stop and or limit price of an existing position.
     * @param positionId
     * @param stopPrice If zero will query for remove the stop price of the position. Set a negative value to
     *  ignore its change.
     * @param limitPrice If zero will query for remove the limit price of the position. Set a negative value to
     *  ignore its change.
     * @return
     */
    o3d::Int32 modifyPosition(const o3d::CString &positionId, o3d::Double stopPrice, o3d::Double limitPrice);

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
    o3d::Double freeAssetQuantity(const o3d::CString &assetId) const;

    /**
     * @brief freeAssetQuantity Get asset locked quantity.
     */
    o3d::Double lockedAssetQuantity(const o3d::CString &assetId) const;

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

    o3d::Int32 m_nextTradeId;
    o3d::Int32 m_nextOrderId;
    o3d::Int32 m_nextPositionId;

    o3d::Double m_freeMargin;
    o3d::Double m_reservedMargin;
    o3d::Double m_marginFactor;

    o3d::CStringMap<Asset> m_assets;

    o3d::TemplateArray<Trade*> m_freeTrades[Trade::NUM_TYPES];
    o3d::TemplateArray<Order*> m_freeOrders;
    o3d::TemplateArray<Position*> m_freePositions;
};

} // namespace siis

#endif // SIIS_TRADERPROXY_H
