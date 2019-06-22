/**
 * @brief SiiS strategy standard trader proxy implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-24
 */

#include "siis/connector/connector.h"
#include "siis/market.h"
#include "siis/handler.h"

#include "siis/connector/stdtraderproxy.h"

#include "siis/trade/assettrade.h"
#include "siis/trade/margintrade.h"
#include "siis/trade/indmargintrade.h"

using namespace siis;

StdTraderProxy::StdTraderProxy(Connector *connector) :
    m_connector(connector)
{

}

StdTraderProxy::~StdTraderProxy()
{

}

Trade* StdTraderProxy::createTrade(Market *market, o3d::Double timeframe)
{
    Trade *trade = nullptr;

    if (market) {
        if (market->mode() == Market::MODE_BUY_SELL) {
            return new AssetTrade(timeframe);
        } else if (market->mode() == Market::MODE_MARGIN) {
            return new MarginTrade(timeframe);
        } else if (market->mode() == Market::MODE_IND_MARGIN) {
            return new IndMarginTrade(timeframe);
        }
    }

    return trade;
}

Trade* StdTraderProxy::createTrade(const o3d::String &marketId, o3d::Double timeframe)
{
    Trade *trade = nullptr;
    Market *market = m_connector->handler()->market(marketId);

    if (market) {
        if (market->mode() == Market::MODE_BUY_SELL) {
            return new AssetTrade(timeframe);
        } else if (market->mode() == Market::MODE_MARGIN) {
            return new MarginTrade(timeframe);
        } else if (market->mode() == Market::MODE_IND_MARGIN) {
            return new IndMarginTrade(timeframe);
        }
    }

    return trade;
}


std::list<Order*> StdTraderProxy::listOrders()
{
    return m_connector->listOrders();
}

std::list<Position*> StdTraderProxy::listPositions()
{
    return m_connector->listPositions();
}

o3d::UInt32 StdTraderProxy::createOrder(const o3d::String marketId, 
                                        o3d::Int32 direction, 
                                        Order::OrderType orderType, 
                                        o3d::Double orderPrice, 
                                        o3d::Double quantity, 
                                        o3d::Double stopLossPrice,
                                        o3d::Double takeProfitPrice,
                                        o3d::Double leverage)
{
    return m_connector->createOrder(marketId, direction, orderType, orderPrice, quantity, stopLossPrice, takeProfitPrice, leverage);
}

void StdTraderProxy::cancelOrder(o3d::UInt32 orderId)
{
    m_connector->cancelOrder(orderId);
}

void StdTraderProxy::closePosition(o3d::UInt32 positionId)
{
    m_connector->closePosition(positionId);
}

void StdTraderProxy::modifyPosition(o3d::UInt32 positionId,
                                    o3d::Double stopLossPrice,
                                    o3d::Double takeProfitPrice)
{
    m_connector->modifyPosition(positionId, stopLossPrice, takeProfitPrice);
}

void StdTraderProxy::onAccountSignal(const AccountSignal &signal)
{
 // Do nothing because the connector call the headler
}

void StdTraderProxy::onAssetSignal(const AssetSignal &signal)
{
 // Do nothing because the connector call the headler
}

void StdTraderProxy::onMarketSignal(const MarketSignal &signal)
{
 // Do nothing because the connector call the headler
}

void StdTraderProxy::onOrderSignal(const OrderSignal &signal)
{
 // Do nothing because the connector call the headler
}

void StdTraderProxy::onPositionSignal(const PositionSignal &signal)
{
 // Do nothing because the connector call the headler
}

void StdTraderProxy::onStatusSignal(const StatusSignal &signal)
{
 // Do nothing because the connector call the headler
}
