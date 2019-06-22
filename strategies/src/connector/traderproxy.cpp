/**
 * @brief SiiS strategy trader proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/connector/traderproxy.h"

#include "siis/connector/connector.h"

#include "siis/market.h"
#include "siis/handler.h"
#include "siis/asset.h"
#include "siis/strategy.h"

#include "siis/trade/assettrade.h"
#include "siis/trade/margintrade.h"
#include "siis/trade/indmargintrade.h"

#include "siis/connector/statussignal.h"
#include "siis/connector/accountsignal.h"
#include "siis/connector/assetsignal.h"
#include "siis/connector/ordersignal.h"
#include "siis/connector/marketsignal.h"
#include "siis/connector/positionsignal.h"

using namespace siis;

TraderProxy::TraderProxy(Connector *connector) :
    m_connector(connector),
    m_freeTrades{{BUCKET_SIZE, BUCKET_SIZE}, {BUCKET_SIZE, BUCKET_SIZE}, {BUCKET_SIZE, BUCKET_SIZE}},
    m_freeOrders(BUCKET_SIZE, BUCKET_SIZE)
{
    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freeOrders[i] = new Order();
    }

    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freeTrades[Trade::TYPE_BUY_SELL][i] = new AssetTrade();
    }

    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freeTrades[Trade::TYPE_MARGIN][i] = new MarginTrade();
    }

    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freeTrades[Trade::TYPE_IND_MARGIN][i] = new IndMarginTrade();
    }
}

TraderProxy::~TraderProxy()
{
    for (o3d::Int32 i = 0; i < m_freeOrders.getMaxSize(); ++i) {
        o3d::deletePtr(m_freeOrders[i]);
    }

    for (o3d::Int32 i = 0; i < m_freeTrades[Trade::TYPE_BUY_SELL].getMaxSize(); ++i) {
        o3d::deletePtr(m_freeTrades[Trade::TYPE_BUY_SELL][i]);
    }

    for (o3d::Int32 i = 0; i < m_freeTrades[Trade::TYPE_MARGIN].getMaxSize(); ++i) {
        o3d::deletePtr(m_freeTrades[Trade::TYPE_MARGIN][i]);
    }

    for (o3d::Int32 i = 0; i < m_freeTrades[Trade::TYPE_IND_MARGIN].getMaxSize(); ++i) {
        o3d::deletePtr(m_freeTrades[Trade::TYPE_IND_MARGIN][i]);
    }
}

o3d::Bool TraderProxy::alive() const
{
    return m_alive;
}

Trade* TraderProxy::createTrade(Market *market, o3d::Double timeframe)
{
    Trade *trade = nullptr;

    if (market) {
        if (market->mode() == Market::MODE_BUY_SELL) {
            if (m_freeTrades[Trade::TYPE_BUY_SELL].getSize() == 0) {
                // empty allocator, make some news
                for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
                    m_freeTrades[Trade::TYPE_BUY_SELL][i] = new AssetTrade();
                }

                // get the last free and pop it
                trade = m_freeTrades[Trade::TYPE_BUY_SELL].getLast();
                m_freeTrades[Trade::TYPE_BUY_SELL].pop();
            }

            trade->init(timeframe);
        } else if (market->mode() == Market::MODE_MARGIN) {
            if (m_freeTrades[Trade::TYPE_MARGIN].getSize() == 0) {
                // empty allocator, make some news
                for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
                    m_freeTrades[Trade::TYPE_MARGIN][i] = new MarginTrade();
                }

                // get the last free and pop it
                trade = m_freeTrades[Trade::TYPE_MARGIN].getLast();
                m_freeTrades[Trade::TYPE_MARGIN].pop();
            }

            trade->init(timeframe);
        } else if (market->mode() == Market::MODE_IND_MARGIN) {
            if (m_freeTrades[Trade::TYPE_IND_MARGIN].getSize() == 0) {
                // empty allocator, make some news
                for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
                    m_freeTrades[Trade::TYPE_IND_MARGIN][i] = new IndMarginTrade();
                }

                // get the last free and pop it
                trade = m_freeTrades[Trade::TYPE_IND_MARGIN].getLast();
                m_freeTrades[Trade::TYPE_IND_MARGIN].pop();
            }

            trade->init(timeframe);
        }
    }

    return trade;
}

Trade* TraderProxy::createTrade(const o3d::String &marketId, o3d::Double timeframe)
{
    Market *market = m_connector->handler()->market(marketId);
    return createTrade(market, timeframe);
}

o3d::Int32 TraderProxy::createOrder(Order *order)
{
    if (m_connector) {
        return m_connector->createOrder(order);
    }

    return -1;
}

void TraderProxy::cancelOrder(const o3d::String &orderId)
{
    if (m_connector) {
        m_connector->cancelOrder(orderId);
    }
}

Order *TraderProxy::newOrder()
{
    Order* order = nullptr;

    if (m_freeOrders.getSize() == 0) {
        // empty allocator, make some news
        for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
            m_freeOrders[i] = new Order();
        }

        // get the last free and pop it
        order = m_freeOrders.getLast();
        m_freeOrders.pop();
    }

    return order;
}

void TraderProxy::freeOrder(Order *order)
{
    if (order) {
        m_freeOrders.push(order);
    }
}

o3d::Int32 TraderProxy::closePosition(const o3d::String &positionId)
{
    if (m_connector) {
        return m_connector->closePosition(positionId);
    }

    return -1;
}

o3d::Int32 TraderProxy::modifyPosition(const o3d::String &positionId, o3d::Double stopLossPrice, o3d::Double takeProfitPrice)
{
    if (m_connector) {
        return m_connector->modifyPosition(positionId, stopLossPrice, takeProfitPrice);
    }

    return -1;
}

o3d::Double TraderProxy::freeMargin() const
{
    return m_freeMargin;
}

o3d::Double TraderProxy::freeAssetQuantity(const o3d::String &assetId) const
{
    auto cit = m_assets.find(assetId);
    if (cit != m_assets.cend()) {
        return cit->second.freeQuantity();
    }

    return 0.0;
}

o3d::Double TraderProxy::lockedAssetQuantity(const o3d::String &assetId) const
{
    auto cit = m_assets.find(assetId);
    if (cit != m_assets.cend()) {
        return cit->second.lockedQuantity();
    }

    return 0.0;
}

void TraderProxy::onAccountSignal(const AccountSignal &signal)
{
    m_freeMargin = signal.freeMargin;
    m_reservedMargin = signal.reservedMargin;
    m_marginFactor = signal.marginFactor;
}

void TraderProxy::onAssetSignal(const AssetSignal &signal)
{
    auto it = m_assets.find(signal.symbol);
    if (it != m_assets.end()) {
        it->second.setQuantity(signal.freeQuantity, signal.lockedQuantity, signal.timestamp);
    } else {
        // append new
        m_assets[signal.symbol] = Asset(signal.symbol);
        m_assets[signal.symbol].setQuantity(signal.freeQuantity, signal.lockedQuantity, signal.timestamp);
    }
}

void TraderProxy::onMarketSignal(const MarketSignal &signal)
{
    // only for pre-defined markets
    Market *market = m_connector->handler()->market(signal.marketId);
    if (market) {
        if (signal.open > 0 && !market->tradeable()) {
            // market reopen, send a signal to the strategy
            market->setState(signal.baseExchangeRate, true);
        } else if (signal.open <= 0 && market->tradeable()) {
            // market closed, send a signal to the strategy
            market->setState(signal.baseExchangeRate, false);
        }

        if (signal.contractSize > 0.0) {
            market->setDetails(signal.contractSize, signal.lotSize, signal.valuePerPip, signal.onePipMeans, signal.hedging);
        }

        if (signal.bid >= 0.0) {
            market->setPrice(signal.bid, signal.ofr, signal.timestamp);
        }

        if (signal.fees[MarketSignal::MAKER].commission >= 0.0) {
            market->setMakerFee(signal.fees[MarketSignal::MAKER].rate, signal.fees[MarketSignal::MAKER].commission, signal.fees[MarketSignal::MAKER].limits);
        }

        if (signal.fees[MarketSignal::TAKER].commission >= 0.0) {
            market->setMakerFee(signal.fees[MarketSignal::TAKER].rate, signal.fees[MarketSignal::TAKER].commission, signal.fees[MarketSignal::TAKER].limits);
        }

        if (signal.marketType != MarketSignal::TYPE_UNDEFINED) {
            market->setType(static_cast<Market::Type>(signal.marketType), static_cast<Market::Contract>(signal.marketContract), static_cast<Market::Unit>(signal.marketUnit));
        }

        if (signal.priceFilter[0] >= 0.0) {
            market->setPriceFilter(signal.priceFilter);
        }

        if (signal.qtyFilter[0] >= 0.0) {
            market->setQtyFilter(signal.qtyFilter);
        }

        if (signal.notionalFilter[0] >= 0.0) {
            market->setNotionalFilter(signal.notionalFilter);
        }

        if (signal.mode != MarketSignal::MODE_UNDEFINED) {
            market->setModeAndOrders(static_cast<Market::Mode>(signal.mode), static_cast<Market::OrderCapacity>(signal.orderCaps));
        }

        if (signal.base.symbol.isValid()) {
            market->setBaseInfo(signal.base.symbol, signal.base.precision);
        }

        if (signal.quote.symbol.isValid()) {
            market->setQuoteInfo(signal.quote.symbol, signal.quote.precision);
        }

        if (signal.base.vol24h >= 0.0) {
            market->setBaseVol24h(signal.base.vol24h);
        }

        if (signal.quote.vol24h >= 0.0) {
            market->setQuoteVol24h(signal.quote.vol24h);
        }
    }
}

void TraderProxy::onOrderSignal(const OrderSignal &signal)
{
    // only for pre-defined markets
    Market *market = m_connector->handler()->market(signal.marketId);
    if (market) {
        // dispatch to the related strategy
        Strategy *strategy = m_connector->handler()->strategy(market->marketId());
        if (strategy) {
            strategy->onOrderSignal(signal);
        }
    }
}

void TraderProxy::onPositionSignal(const PositionSignal &signal)
{
    // only for pre-defined markets
    Market *market = m_connector->handler()->market(signal.marketId);
    if (market) {
        // dispatch to the related strategy
        Strategy *strategy = m_connector->handler()->strategy(market->marketId());
        if (strategy) {
            strategy->onPositionSignal(signal);
        }
    }
}

void TraderProxy::onStatusSignal(const StatusSignal &signal)
{
    if (signal.event == StatusSignal::CONN_ACQUIRED) {
        // @todo could propagate a signal on each strategy if was lost and now acquired
        m_alive = true;
    } else if (signal.event == StatusSignal::CONN_LOST) {
        // we cannot do anything if connection is lost on client side
        m_alive = false;
    }
}
