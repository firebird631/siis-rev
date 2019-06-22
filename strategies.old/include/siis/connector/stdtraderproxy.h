/**
 * @brief SiiS strategy standard trader proxy implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#ifndef SIIS_STDTRADERPROXY_H
#define SIIS_STDTRADERPROXY_H

#include "traderproxy.h"

#include <o3d/core/stringmap.h>

namespace siis {

class Market;
class Trade;

/**
 * @brief Strategy asset trader proxy.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API StdTraderProxy : public TraderProxy
{
public:

    StdTraderProxy(Connector *connector);

    virtual ~StdTraderProxy() override;

    //
    // trade
    //

    /**
     * @brief createTrade Create a new StrategyTrade object according to the type of the market.
     */
    Trade* createTrade(Market *market, o3d::Double timeframe);
    Trade* createTrade(const o3d::String &marketId, o3d::Double timeframe);

    //
    //
    // orders
    //

    virtual std::list<Order*> listOrders() override;
    virtual std::list<Position*> listPositions() override;
    virtual o3d::UInt32 createOrder(const o3d::String marketId, 
                                    o3d::Int32 direction, 
                                    Order::OrderType orderType, 
                                    o3d::Double orderPrice, 
                                    o3d::Double quantity, 
                                    o3d::Double stopLossPrice,
                                    o3d::Double takeProfitPrice,
                                    o3d::Double leverage) override;
    virtual void cancelOrder(o3d::UInt32 orderId) override;
    virtual void closePosition(o3d::UInt32 positionId) override;
    virtual void modifyPosition(o3d::UInt32 positionId,
                                o3d::Double stopLossPrice,
                                o3d::Double takeProfitPrice) override;

    //
    // signals
    //

    virtual void onAccountSignal(const AccountSignal &signal) override;
    virtual void onAssetSignal(const AssetSignal &signal) override;
    virtual void onMarketSignal(const MarketSignal &signal) override;
    virtual void onOrderSignal(const OrderSignal &signal) override;
    virtual void onPositionSignal(const PositionSignal &signal) override;
    virtual void onStatusSignal(const StatusSignal &signal) override;

private:

    Connector *m_connector;
};

} // namespace siis

#endif // SIIS_STDTRADERPROXY_H
