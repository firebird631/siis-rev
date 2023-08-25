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
#include "siis/trade/positiontrade.h"

#include "siis/connector/statussignal.h"
#include "siis/connector/accountsignal.h"
#include "siis/connector/assetsignal.h"
#include "siis/connector/ordersignal.h"
#include "siis/connector/marketsignal.h"
#include "siis/connector/positionsignal.h"

#include <o3d/core/uuid.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

TraderProxy::TraderProxy(Connector *connector) :
    m_connector(connector),
    m_nextTradeId(1),
    m_nextOrderId(1),
    m_nextPositionId(1),
    m_freeMargin(0),
    m_reservedMargin(0),
    m_marginFactor(0),
    m_freeTrades{{BUCKET_SIZE, BUCKET_SIZE}, {BUCKET_SIZE, BUCKET_SIZE},
                 {BUCKET_SIZE, BUCKET_SIZE}, {BUCKET_SIZE, BUCKET_SIZE}},
    m_freeOrders(BUCKET_SIZE, BUCKET_SIZE),
    m_freePositions(BUCKET_SIZE, BUCKET_SIZE)
{
    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freeOrders[i] = new Order();
        m_freeOrders[i]->proxy = this;
    }

    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freeTrades[Trade::TYPE_BUY_SELL][i] = new AssetTrade(this);
    }

    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freeTrades[Trade::TYPE_MARGIN][i] = new MarginTrade(this);
    }

    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freeTrades[Trade::TYPE_IND_MARGIN][i] = new IndMarginTrade(this);
    }

    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freeTrades[Trade::TYPE_POSITION][i] = new PositionTrade(this);
    }

    for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
        m_freePositions[i] = new Position();
        m_freePositions[i]->proxy = this;
    }
}

TraderProxy::~TraderProxy()
{
    if (m_freeOrders.getSize() < m_freeOrders.getMaxSize()) {
        WARN("memory", o3d::String("{0} orders are not freed").arg(m_freeOrders.getMaxSize() - m_freeOrders.getSize()));
    }

    for (o3d::Int32 i = 0; i < m_freeOrders.getMaxSize(); ++i) {
        o3d::deletePtr(m_freeOrders[i]);
    }

    if (m_freeTrades[Trade::TYPE_BUY_SELL].getSize() < m_freeTrades[Trade::TYPE_BUY_SELL].getMaxSize()) {
        WARN("memory", o3d::String("{0} spots trades are not freed").arg(
                 m_freeTrades[Trade::TYPE_BUY_SELL].getMaxSize() - m_freeTrades[Trade::TYPE_BUY_SELL].getSize()));
    }

    for (o3d::Int32 i = 0; i < m_freeTrades[Trade::TYPE_BUY_SELL].getMaxSize(); ++i) {
        o3d::deletePtr(m_freeTrades[Trade::TYPE_BUY_SELL][i]);
    }

    if (m_freeTrades[Trade::TYPE_MARGIN].getSize() < m_freeTrades[Trade::TYPE_MARGIN].getMaxSize()) {
        WARN("memory", o3d::String("{0} margins trades are not freed").arg(
                 m_freeTrades[Trade::TYPE_MARGIN].getMaxSize() - m_freeTrades[Trade::TYPE_MARGIN].getSize()));
    }

    for (o3d::Int32 i = 0; i < m_freeTrades[Trade::TYPE_MARGIN].getMaxSize(); ++i) {
        o3d::deletePtr(m_freeTrades[Trade::TYPE_MARGIN][i]);
    }

    if (m_freeTrades[Trade::TYPE_IND_MARGIN].getSize() < m_freeTrades[Trade::TYPE_IND_MARGIN].getMaxSize()) {
        WARN("memory", o3d::String("{0} indivisbles margins trades are not freed").arg(
                 m_freeTrades[Trade::TYPE_IND_MARGIN].getMaxSize() - m_freeTrades[Trade::TYPE_IND_MARGIN].getSize()));
    }

    for (o3d::Int32 i = 0; i < m_freeTrades[Trade::TYPE_IND_MARGIN].getMaxSize(); ++i) {
        o3d::deletePtr(m_freeTrades[Trade::TYPE_IND_MARGIN][i]);
    }

    if (m_freeTrades[Trade::TYPE_POSITION].getSize() < m_freeTrades[Trade::TYPE_POSITION].getMaxSize()) {
        WARN("memory", o3d::String("{0} positions trades are not freed").arg(
                 m_freeTrades[Trade::TYPE_POSITION].getMaxSize() - m_freeTrades[Trade::TYPE_POSITION].getSize()));
    }

    for (o3d::Int32 i = 0; i < m_freeTrades[Trade::TYPE_POSITION].getMaxSize(); ++i) {
        o3d::deletePtr(m_freeTrades[Trade::TYPE_POSITION][i]);
    }

    if (m_freePositions.getSize() < m_freePositions.getMaxSize()) {
        WARN("memory", o3d::String("{0} positions are not freed").arg(
                 m_freePositions.getMaxSize() - m_freePositions.getSize()));
    }

    for (o3d::Int32 i = 0; i < m_freePositions.getMaxSize(); ++i) {
        o3d::deletePtr(m_freePositions[i]);
    }
}

o3d::Bool TraderProxy::alive() const
{
    return m_alive;
}

Trade* TraderProxy::createTrade(Market *market, Trade::Type tradeType, o3d::Double timeframe)
{
    Trade *trade = nullptr;

    if (market == nullptr) {
        return nullptr;
    }

    m_mutex.lock();

    if (tradeType == Trade::TYPE_BUY_SELL && market->hasSpot()) {
        if (m_freeTrades[Trade::TYPE_BUY_SELL].getSize() == 0) {
            // empty allocator, make some news
            for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
                m_freeTrades[Trade::TYPE_BUY_SELL][i] = new AssetTrade(this);
            }
        }

        // get the last free and pop it
        trade = m_freeTrades[Trade::TYPE_BUY_SELL].getLast();
        m_freeTrades[Trade::TYPE_BUY_SELL].pop();

    } else if (tradeType == Trade::TYPE_MARGIN && market->hasMargin()) {
        if (m_freeTrades[Trade::TYPE_MARGIN].getSize() == 0) {
            // empty allocator, make some news
            for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
                m_freeTrades[Trade::TYPE_MARGIN][i] = new MarginTrade(this);
            }
        }

        // get the last free and pop it
        trade = m_freeTrades[Trade::TYPE_MARGIN].getLast();
        m_freeTrades[Trade::TYPE_MARGIN].pop();

    } else if (tradeType == Trade::TYPE_IND_MARGIN && market->hasMargin() && market->indivisiblePosition()) {
        if (m_freeTrades[Trade::TYPE_IND_MARGIN].getSize() == 0) {
            // empty allocator, make some news
            for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
                m_freeTrades[Trade::TYPE_IND_MARGIN][i] = new IndMarginTrade(this);
            }
        }

        // get the last free and pop it
        trade = m_freeTrades[Trade::TYPE_IND_MARGIN].getLast();
        m_freeTrades[Trade::TYPE_IND_MARGIN].pop();

    } else if (tradeType == Trade::TYPE_POSITION && market->hasPosition()) {
        if (m_freeTrades[Trade::TYPE_POSITION].getSize() == 0) {
            // empty allocator, make some news
            for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
                m_freeTrades[Trade::TYPE_POSITION][i] = new PositionTrade(this);
            }
        }

        // get the last free and pop it
        trade = m_freeTrades[Trade::TYPE_POSITION].getLast();
        m_freeTrades[Trade::TYPE_POSITION].pop();
    }

    if (trade) {
        trade->init(timeframe);
        trade->setId(m_nextTradeId++);
        O3D_ASSERT(m_nextTradeId <= o3d::Limits<o3d::Int32>::max());
    }

    m_mutex.unlock();

    return trade;
}

Trade* TraderProxy::createTrade(const o3d::CString &marketId, Trade::Type tradeType, o3d::Double timeframe)
{
    Market *market = m_connector->handler()->market(marketId);
    return createTrade(market, tradeType, timeframe);
}

void TraderProxy::freeTrade(Trade *trade)
{
    if (trade) {
        trade->reset();

        m_mutex.lock();

        if (trade->type() == Trade::TYPE_BUY_SELL) {
            m_freeTrades[Trade::TYPE_BUY_SELL].push(trade);
        } else if (trade->type() == Trade::TYPE_MARGIN) {
            m_freeTrades[Trade::TYPE_MARGIN].push(trade);
        } else if (trade->type() == Trade::TYPE_IND_MARGIN) {
            m_freeTrades[Trade::TYPE_IND_MARGIN].push(trade);
        } else if (trade->type() == Trade::TYPE_POSITION) {
            m_freeTrades[Trade::TYPE_POSITION].push(trade);
        }

        m_mutex.unlock();
    }
}

o3d::Int32 TraderProxy::createOrder(Order *order)
{
    if (m_connector) {
        return m_connector->createOrder(order);
    }

    return Order::RET_UNREACHABLE_SERVICE;
}

o3d::Int32 TraderProxy::cancelOrder(const o3d::CString &orderId)
{
    if (m_connector) {
        return m_connector->cancelOrder(orderId);
    }

    return Order::RET_UNREACHABLE_SERVICE;
}

Order *TraderProxy::newOrder(Strategy *strategy)
{
    Order* order = nullptr;

    if (!strategy) {
        O3D_ERROR(o3d::E_NullPointer("Strategy must be a valid pointer"));
    }

    m_mutex.lock();

    if (m_freeOrders.getSize() == 0) {
        // empty allocator, make some news
        for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
            m_freeOrders[i] = new Order();
            m_freeOrders[i]->proxy = this;
        }
    }

    // get the last free and pop it
    order = m_freeOrders.getLast();
    m_freeOrders.pop();

    m_mutex.unlock();

    // set a unique id for reference
    if (order) {
        order->refId = o3d::Uuid::uuid5("siis").toCString();
        order->strategy = strategy;
    }

    return order;
}

void TraderProxy::freeOrder(Order *order)
{
    if (order) {
        order->reset();

        m_mutex.lock();
        m_freeOrders.push(order);
        m_mutex.unlock();
    }
}

Position *TraderProxy::newPosition(Strategy *strategy)
{
    Position *position = nullptr;

    if (!strategy) {
        O3D_ERROR(o3d::E_NullPointer("Strategy must be a valid pointer"));
    }

    m_mutex.lock();

    if (m_freePositions.getSize() == 0) {
        // empty allocator, make some news
        for (o3d::Int32 i = 0; i < BUCKET_SIZE; ++i) {
            m_freePositions[i] = new Position();
            m_freePositions[i]->proxy = this;
        }
    }

    // get the last free and pop it
    position = m_freePositions.getLast();
    m_freePositions.pop();

    m_mutex.unlock();

    // set a unique id for reference
    if (position) {
        position->positionId = o3d::Uuid::uuid5("siis").toCString();
        position->strategy = strategy;
    }

    return position;
}

void TraderProxy::freePosition(Position *position)
{
    if (position) {
        position->reset();

        m_mutex.lock();
        m_freePositions.push(position);
        m_mutex.unlock();
    }
}

o3d::Int32 TraderProxy::closePosition(const o3d::CString &positionId,
                                      o3d::Int32 direction,
                                      o3d::Double quantity,
                                      o3d::Bool taker,
                                      o3d::Double limitPrice)
{
    if (m_connector) {
        return m_connector->closePosition(positionId, direction, quantity, taker, limitPrice);
    }

    return Order::RET_UNREACHABLE_SERVICE;
}

o3d::Int32 TraderProxy::modifyPosition(const o3d::CString &positionId, o3d::Double stopPrice, o3d::Double limitPrice)
{
    if (m_connector) {
        return m_connector->modifyPosition(positionId, stopPrice, limitPrice);
    }

    return Order::RET_UNREACHABLE_SERVICE;
}

o3d::Double TraderProxy::freeMargin() const
{
    return m_freeMargin;
}

o3d::Double TraderProxy::freeAssetQuantity(const o3d::CString &assetId) const
{
    o3d::Double qty = 0.0;

    m_mutex.lock();

    auto cit = m_assets.find(assetId);
    if (cit != m_assets.cend()) {
        qty = cit->second.freeQuantity();
    }

    m_mutex.unlock();

    return qty;
}

o3d::Double TraderProxy::lockedAssetQuantity(const o3d::CString &assetId) const
{
    o3d::Double qty = 0.0;

    m_mutex.lock();

    auto cit = m_assets.find(assetId);
    if (cit != m_assets.cend()) {
        return cit->second.lockedQuantity();
    }

    m_mutex.unlock();

    return qty;
}

void TraderProxy::onAccountSignal(const AccountSignal &signal)
{
    m_mutex.lock();

    m_freeMargin = signal.freeMargin;
    m_reservedMargin = signal.reservedMargin;
    m_marginFactor = signal.marginFactor;

    m_mutex.unlock();
}

void TraderProxy::onAssetSignal(const AssetSignal &signal)
{
    m_mutex.lock();

    auto it = m_assets.find(signal.symbol);
    if (it != m_assets.end()) {
        it->second.setQuantity(signal.freeQuantity, signal.lockedQuantity, signal.timestamp);
    } else {
        // append new
        m_assets[signal.symbol] = Asset(signal.symbol);
        m_assets[signal.symbol].setQuantity(signal.freeQuantity, signal.lockedQuantity, signal.timestamp);
    }

    m_mutex.unlock();
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

        if (signal.marginFactor > 0.0) {
            market->setMarginFactor(signal.marginFactor);
        }

        if (signal.pair.isValid()) {
            market->setPair(signal.pair);
        }

        if (signal.alias.isValid()) {
            market->setAlias(signal.alias);
        }

        if (signal.contractSize > 0.0) {
            market->setDetails(signal.contractSize, signal.lotSize, signal.valuePerPip, signal.onePipMeans, signal.hedging);
        }

        if (signal.bid >= 0.0) {
            market->setPrice(signal.bid, signal.ask, signal.timestamp);
        }

        if (signal.makerFees.commission >= 0.0) {
            market->setMakerFee(signal.makerFees.rate, signal.makerFees.commission, signal.makerFees.limits);
        }

        if (signal.takerFees.commission >= 0.0) {
            market->setMakerFee(signal.takerFees.rate, signal.takerFees.commission, signal.takerFees.limits);
        }

        if (signal.marketType != MarketSignal::TYPE_UNDEFINED) {
            market->setType(static_cast<Market::Type>(signal.marketType),
                            static_cast<Market::Contract>(signal.marketContract),
                            static_cast<Market::Unit>(signal.marketUnit));
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

        if (signal.tradeCaps != MarketSignal::TRADE_UNDEFINED) {
            market->setTradeCapacities(static_cast<o3d::Int32>(signal.tradeCaps));
        }

        if (signal.orderCaps != MarketSignal::ORDER_UNDEFINED) {
            market->setOrderCapacities(static_cast<o3d::Int32>(signal.orderCaps));
        }

        if (signal.base.symbol.isValid()) {
            market->setBaseInfo(signal.base.symbol, signal.base.precision);
        }

        if (signal.quote.symbol.isValid()) {
            market->setQuoteInfo(signal.quote.symbol, signal.quote.precision);
        }

        if (signal.settlement.symbol.isValid()) {
            market->setSettlementInfo(signal.settlement.symbol, signal.settlement.precision);
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
    m_mutex.lock();

    if (signal.event == StatusSignal::CONN_ACQUIRED) {
        // @todo could propagate a signal on each strategy if was lost and now acquired
        m_alive = true;
    } else if (signal.event == StatusSignal::CONN_LOST) {
        // we cannot do anything if connection is lost on client side
        m_alive = false;
    }

    m_mutex.unlock();
}
