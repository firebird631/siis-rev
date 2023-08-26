/**
 * @brief SiiS strategy local connector implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#include "siis/connector/localconnector.h"
#include "siis/connector/traderproxy.h"

#include "siis/handler.h"
#include "siis/strategy.h"
#include "siis/config/config.h"
#include "siis/connector/ordersignal.h"

#include <o3d/core/debug.h>
#include <o3d/core/uuid.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;


LocalConnector::LocalConnector(Handler *handler) :
    m_handler(handler),
    m_running(false),
    m_traderProxy(nullptr)
{

}

LocalConnector::~LocalConnector()
{
    stop();
}

void LocalConnector::start()
{
    if (!m_running) {
        m_running = true;
    }
}

void LocalConnector::stop()
{
    if (m_running) {
        m_running = false;
    }
}

void LocalConnector::update()
{
    m_mutex.lock();

    if (!m_virtualOrders.empty()) {
        for (auto it = m_virtualOrders.begin(); it != m_virtualOrders.end(); ++it) {
            // check and execute order...
            Order *order = it->second;
            const Market *market = order->strategy->market();
            o3d::Bool closed = false;

            try {
                if (order->orderType == Order::ORDER_LIMIT) {
                    closed = handleLimitOrder(order, market);
                }
                else if (order->orderType == Order::ORDER_STOP) {
                    closed = handleStopOrder(order, market);
                }
                else if (order->orderType == Order::ORDER_STOP_LIMIT) {
                    closed = handleStopLimitOrder(order, market);
                }
                else if (order->orderType == Order::ORDER_TAKE_PROFIT) {
                    closed = handleTakeProfitOrder(order, market);
                }
                else if (order->orderType == Order::ORDER_TAKE_PROFIT_LIMIT) {
                    closed = handleTakeProfitLimitOrder(order, market);
                }
            } catch (o3d::E_BaseException &e) {
            }

            if (closed) {
                m_removedOrders.push_back(order);
            }
        }
    }

    // update positions
    if (m_virtualPositions.empty()) {
        for (auto it = m_virtualPositions.begin(); it != m_virtualPositions.end(); ++it) {
            Position *position = it->second;
            const Market *market = position->strategy->market();
            o3d::Bool closed = false;

            try {
                // trigger position limit/stop for position only
                if (market->hasPosition()) {
                    _updatePosition(position, market);
                }

                // update profit/loss for stats
                position->updatePnl(market);
            } catch (o3d::E_BaseException &e) {
            }

            if (closed) {
                m_removedPositions.push_back(position);
            }
        }
    }

    // update virtual account
    m_virtualAccount.updateBalance();
    m_virtualAccount.updateDrawDown();

    // cleanup resources
    if (!m_removedOrders.empty()) {
        for (auto it = m_removedOrders.begin(); it != m_removedOrders.end(); ++it) {
            // and remove from pending orders
            auto mit = m_virtualOrders.find((*it)->orderId);
            if (mit != m_virtualOrders.end()) {
                m_virtualOrders.erase(mit);
            }

            // clean non removed orders
            m_traderProxy->freeOrder(*it);
        }

        m_removedOrders.clear();
    }

    if (!m_removedPositions.empty()) {
        for (auto it = m_removedPositions.begin(); it != m_removedPositions.end(); ++it) {
            // and remove from actives positions
            auto mit = m_virtualPositions.find((*it)->positionId);
            if (mit != m_virtualPositions.end()) {
                m_virtualPositions.erase(mit);
            }

            // clean non removed positions
            m_traderProxy->freePosition(*it);
        }

        m_removedPositions.clear();
    }

    m_mutex.unlock();
}

void LocalConnector::connect()
{
    // nothing
}

void LocalConnector::disconnect()
{
    // nothing
}

void LocalConnector::subscribe(Subscription &sub)
{
    // nothing
}

void LocalConnector::unsubscribe(Subscription &sub)
{
    // nothing
}

void LocalConnector::unsubscribeAll()
{
    // nothing
}

void LocalConnector::setTraderProxy(TraderProxy *traderProxy)
{
    m_traderProxy = traderProxy;
}

TraderProxy *LocalConnector::traderProxy()
{
    return m_traderProxy;
}

const TraderProxy *LocalConnector::traderProxy() const
{
    return m_traderProxy;
}

Handler *LocalConnector::handler()
{
    return m_handler;
}

const Handler *LocalConnector::handler() const
{
    return m_handler;
}

void LocalConnector::init(Config *config)
{
    if (config) {
        m_virtualAccount.balance = config->getInitialBalance();
        m_virtualAccount.currency = config->getInitialCurrency();

        // setup precision
        if (m_virtualAccount.currency == "USD" || m_virtualAccount.currency == "USDT") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "USD" || m_virtualAccount.currency == "USDT" || m_virtualAccount.currency == "ZUSD") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "EUR" || m_virtualAccount.currency == "ZEUR") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "CAD" || m_virtualAccount.currency == "ZCAD") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "JPY" || m_virtualAccount.currency == "ZJPY") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "BTC" || m_virtualAccount.currency == "XBT") {
            m_virtualAccount.precision = 8;
        } else if (m_virtualAccount.currency == "ETH") {
            m_virtualAccount.precision = 8;
        } else if (m_virtualAccount.currency == "BNB") {
            m_virtualAccount.precision = 8;
        } else if (m_virtualAccount.currency == "XRP") {
            m_virtualAccount.precision = 4;
        } else {
            m_virtualAccount.precision = 2;
        }
    }
}

void LocalConnector::terminate()
{
    for (auto it = m_virtualOrders.begin(); it != m_virtualOrders.end(); ++it) {
        // clean non closed orders
        m_traderProxy->freeOrder(it->second);
    }

    m_virtualOrders.clear();

    for (auto it = m_virtualPositions.begin(); it != m_virtualPositions.end(); ++it) {
        // clean non closed positions
        m_traderProxy->freePosition(it->second);
    }

    m_virtualPositions.clear();

    if (!m_removedOrders.empty()) {
        WARN("memory", o3d::String("{0} virtual orders are not removed").arg(
                           static_cast<o3d::Int32>(m_removedOrders.size())));

        for (auto it = m_removedOrders.begin(); it != m_removedOrders.end(); ++it) {
            // clean non removed orders
            m_traderProxy->freeOrder(*it);
        }

        m_removedOrders.clear();
    }

    if (!m_removedPositions.empty()) {
        WARN("memory", o3d::String("{0} virtual positions are not removed").arg(
                           static_cast<o3d::Int32>(m_removedPositions.size())));

        for (auto it = m_removedPositions.begin(); it != m_removedPositions.end(); ++it) {
            // clean non removed positions
            m_traderProxy->freePosition(*it);
        }

        m_removedPositions.clear();
    }
}

void LocalConnector::fetchAnyOrders()
{
    // nothing
}

void LocalConnector::fetchOrder(const o3d::CString &marketId)
{
    // nothing
}

void LocalConnector::fetchAnyPositions()
{
    // nothing
}

void LocalConnector::fetchPositions(const o3d::CString &marketId)
{
    // nothing
}

o3d::Int32 LocalConnector::createOrder(Order *order)
{
    if (m_traderProxy) {
        if (!order) {
            return Order::RET_INVALID_ARGS;
        }

        if (order->orderType == Order::ORDER_MARKET) {
            // @todo refactoring
            // direct execution and return
            Strategy *strategy = order->strategy;
            const Market* market = strategy->market();

            o3d::Double execPrice = market->openExecPrice(order->direction);

            order->orderId = o3d::Uuid::uuid5("siis").toCString();
            order->created = handler()->timestamp();

            order->executed = handler()->timestamp();
            order->execPrice = execPrice;
            order->avgPrice = execPrice;
            order->filled = order->orderQuantity;
            order->cumulativeFilled = order->orderQuantity;

            // direct order signal to strategy
            OrderSignal openOrderSignal(OrderSignal::OPENED);
            openOrderSignal.direction = order->direction;
            openOrderSignal.marketId = order->marketId;
            openOrderSignal.created = handler()->timestamp();
            openOrderSignal.orderId = order->orderId;
            openOrderSignal.refId = order->refId;
            openOrderSignal.orderType = order->orderType;
            openOrderSignal.flags = order->flags;

            // excepted for spot, need a position opened signal
            if (strategy->tradeType() != Trade::TYPE_ASSET) {
                if (strategy->tradeType() == Trade::TYPE_POSITION) {
                    // with the same order as order id for a position
                    openOrderSignal.positionId = order->orderId;
                } else {
                    // or market id for margin
                    openOrderSignal.positionId = market->marketId();
                }
            }

            // order is accepted
            strategy->onOrderSignal(openOrderSignal);

            OrderSignal tradedOrderSignal(OrderSignal::TRADED);
            tradedOrderSignal.direction = order->direction;
            tradedOrderSignal.marketId = order->marketId;
            tradedOrderSignal.executed = handler()->timestamp();
            tradedOrderSignal.orderId = order->orderId;
            tradedOrderSignal.refId = order->refId;
            tradedOrderSignal.orderType = order->orderType;
            tradedOrderSignal.flags = order->flags;

            tradedOrderSignal.avgPrice = execPrice;
            tradedOrderSignal.execPrice = execPrice;
            tradedOrderSignal.filled = order->orderQuantity;
            tradedOrderSignal.cumulativeFilled = order->orderQuantity;
            tradedOrderSignal.completed = true;

            // @todo compute quote transacted
            // tradedOrderSignal.quoteTransacted =
            // @todo commission fees and its currency

            strategy->onOrderSignal(tradedOrderSignal);

            if (order->closeOnly() || order->reduceOnly()) {
                // only for margin and indivisible margin, need a position updated or deleted signal
                // for position use modifyPosition or closePosition
                if (strategy->tradeType() == Trade::TYPE_MARGIN || strategy->tradeType() == Trade::TYPE_IND_MARGIN) {
                    PositionSignal updatedPositionSignal(PositionSignal::DELETED);

                    // retrieve a virtual position                   
                    Position *position = nullptr;

                    m_mutex.lock();

                    auto pit = m_virtualPositions.find(market->marketId());
                    if (pit != m_virtualPositions.end()) {
                        position = pit->second;
                    } else {
                        // error cannot close a non existing position, free order
                        m_traderProxy->freeOrder(order);
                        m_mutex.unlock();

                        return Order::RET_INSUFFICIENT_FUNDS;
                    }

                    m_mutex.unlock();

                    o3d::Double closeExecPrice = market->closeExecPrice(position->direction);

                    // @todo check qty, compute avgPrice, execPrice, entryPrice, entryQty, exitPrice, exitQty
                    position->quantity -= order->orderQuantity;

                    updatedPositionSignal.direction = order->direction;
                    updatedPositionSignal.marketId = order->marketId;
                    updatedPositionSignal.created = position->created;
                    updatedPositionSignal.updated = handler()->timestamp();
                    updatedPositionSignal.refOrderId = order->refId;
                    updatedPositionSignal.positionId = position->positionId;
                    // openPositionSignal.commission @todo

                    // exit traded
                    updatedPositionSignal.avgPrice = closeExecPrice;
                    updatedPositionSignal.execPrice = closeExecPrice;
                    updatedPositionSignal.filled = order->orderQuantity;
                    updatedPositionSignal.cumulativeFilled = order->orderQuantity;

                    strategy->onPositionSignal(updatedPositionSignal);

                    // need to delete position
                    if (position->quantity <= 0.0) {
                        PositionSignal deletedPositionSignal(PositionSignal::DELETED);

                        deletedPositionSignal.direction = order->direction;
                        deletedPositionSignal.marketId = order->marketId;
                        deletedPositionSignal.created = position->created;
                        deletedPositionSignal.updated = handler()->timestamp();
                        deletedPositionSignal.refOrderId = order->refId;
                        deletedPositionSignal.positionId = position->positionId;

                        strategy->onPositionSignal(deletedPositionSignal);

                        m_mutex.lock();
                        m_traderProxy->freePosition(position);
                        m_virtualPositions.erase(pit);
                        m_mutex.unlock();
                    }
                }
            } else {
                // @todo refactoring
                // excepted for spot, need a position opened signal
                if (strategy->tradeType() != Trade::TYPE_ASSET) {
                    // create a virtual position
                    Position *position = nullptr;

                    if (strategy->tradeType() == Trade::TYPE_POSITION) {
                        // always a new position with the same order as order id for a position
                        position = traderProxy()->newPosition(strategy);

                        position->positionId = order->orderId;   // same as order id
                        position->refOrderId = order->refId;
                        position->direction = order->direction;
                        position->marketId = order->marketId;
                        position->created = handler()->timestamp();
                        position->updated = handler()->timestamp();

                        position->stopPrice = order->stopPrice;
                        position->limitPrice = order->limitPrice;

                        m_mutex.lock();
                        m_virtualPositions[position->positionId] = position;
                        m_mutex.unlock();

                    } else if (strategy->tradeType() == Trade::TYPE_MARGIN || strategy->tradeType() == Trade::TYPE_IND_MARGIN) {
                        // or market id for margin, try to retrieve if exists else new
                        m_mutex.lock();

                        auto pit = m_virtualPositions.find(market->marketId());
                        if (pit != m_virtualPositions.end()) {
                            position = pit->second;
                        } else {
                            position = traderProxy()->newPosition(strategy);

                            position->positionId = market->marketId();  // same as market id (only if no hedging)
                            position->refOrderId = order->refId;
                            position->direction = order->direction;
                            position->marketId = order->marketId;
                            position->created = handler()->timestamp();
                            position->updated = handler()->timestamp();

                            m_virtualPositions[position->positionId] = position;
                        }

                        m_mutex.unlock();
                    }

                    order->positionId = position->positionId;

                    // local position data
                    position->local.entryPrice = execPrice;
                    position->local.entryQty = order->orderQuantity;  // 100% entry

                    PositionSignal openPositionSignal(PositionSignal::OPENED);
                    openPositionSignal.direction = order->direction;
                    openPositionSignal.marketId = order->marketId;
                    openPositionSignal.created = handler()->timestamp();
                    openPositionSignal.updated = handler()->timestamp();
                    openPositionSignal.refOrderId = order->refId;
                    openPositionSignal.positionId = position->positionId;
                    // openPositionSignal.commission @todo

                    // entry traded
                    openPositionSignal.avgPrice = execPrice;
                    openPositionSignal.execPrice = execPrice;
                    openPositionSignal.filled = order->orderQuantity;
                    openPositionSignal.cumulativeFilled = order->orderQuantity;

                    strategy->onPositionSignal(openPositionSignal);
                }
            }

            OrderSignal deletedOrderSignal(OrderSignal::DELETED);
            deletedOrderSignal.direction = order->direction;
            deletedOrderSignal.marketId = order->marketId;
            deletedOrderSignal.executed = handler()->timestamp();
            deletedOrderSignal.orderId = order->orderId;
            deletedOrderSignal.refId = order->refId;
            deletedOrderSignal.orderType = order->orderType;
            deletedOrderSignal.flags = order->flags;
            // @todo do we set cumulative, avg and completed here ?

            strategy->onOrderSignal(deletedOrderSignal);

            // finally free the order because it is fully executed
            m_mutex.lock();
            m_traderProxy->freeOrder(order);
            m_mutex.unlock();

            return Order::RET_OK;

        } else {
            // check and insert for later execution
            order->orderId = o3d::Uuid::uuid5("siis").toCString();

            // direct order signal to strategy
            Strategy *strategy = order->strategy;

            OrderSignal openOrderSignal(OrderSignal::OPENED);
            openOrderSignal.direction = order->direction;
            openOrderSignal.marketId = order->marketId;
            openOrderSignal.created = handler()->timestamp();
            openOrderSignal.orderId = order->orderId;
            openOrderSignal.refId = order->refId;

            openOrderSignal.orderType = order->orderType;
            openOrderSignal.orderQuantity = order->orderQuantity;
            openOrderSignal.orderPrice = order->orderPrice;
            openOrderSignal.orderStopPrice = order->orderStopPrice;
            openOrderSignal.flags = order->flags;

            m_mutex.lock();
            m_virtualOrders[order->orderId] = order;
            m_mutex.unlock();

            strategy->onOrderSignal(openOrderSignal);

            return Order::RET_OK;
        }
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Int32 LocalConnector::cancelOrder(const o3d::CString &orderId)
{
    if (m_traderProxy) {
        if (orderId.isValid()) {
            o3d::FastMutexLocker _(m_mutex);

            auto it = m_virtualOrders.find(orderId);
            if (it != m_virtualOrders.end()) {
                Order *order = it->second;
                Strategy *strategy = order->strategy;

                OrderSignal cancelOrderSignal(OrderSignal::CANCELED);
                cancelOrderSignal.executed = handler()->timestamp();
                cancelOrderSignal.orderId = orderId;
                cancelOrderSignal.refId = order->refId;
                cancelOrderSignal.orderType = order->orderType;

                strategy->onOrderSignal(cancelOrderSignal);

                // finally free the order because it is fully executed
                m_virtualOrders.erase(it);
                m_traderProxy->freeOrder(order);

                return Order::RET_OK;
            }
        }

        return Order::RET_INVALID_ARGS;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Int32 LocalConnector::closePosition(const o3d::CString &positionId,
                                         o3d::Int32 direction,
                                         o3d::Double quantity,
                                         o3d::Bool taker,
                                         o3d::Double limitPrice)
{
    // @todo only taker and no limit price supported
    if (m_traderProxy) {
        if (positionId.isValid()) {
            o3d::FastMutexLocker _(m_mutex);

            auto it = m_virtualPositions.find(positionId);
            if (it != m_virtualPositions.end()) {
                Position *position = it->second;
                Strategy *strategy = position->strategy;  // @warn not sure to have it on live
                const Market *market = strategy->market();

                o3d::Double closeExecPrice = strategy->market()->closeExecPrice(position->direction);
                o3d::Int32 res = Order::RET_ERROR;

                if (strategy->tradeType() == Trade::TYPE_MARGIN) {
                    // @todo rare case
                    // res = execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
                } else if (strategy->tradeType() == Trade::TYPE_IND_MARGIN) {
                    // @todo not really a good way
                    // res = execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
                } else if (strategy->tradeType() == Trade::TYPE_POSITION) {
                    if (quantity > 0 && quantity < position->quantity) {
                        res = _reducePosition(position, market, closeExecPrice);
                    } else {
                        res = _closePosition(position, market, closeExecPrice);
                    }
                }

                if (res == Order::RET_OK) {
                    // finally free the position because it is fully executed
                    m_virtualPositions.erase(it);
                    m_traderProxy->freePosition(position);
                }

                return res;
            }
        }

        return Order::RET_INVALID_ARGS;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

void LocalConnector::fetchAccount()
{
    if (m_traderProxy) {
        // @todo could set initial account margin from configuration
    }
}

void LocalConnector::fetchAnyAssets()
{
    if (m_traderProxy) {
        // @todo could set initial assets quantities from configuration
    }
}

void LocalConnector::fetchAssets(const o3d::CString &assetId)
{
    if (m_traderProxy) {
        // @todo could set initial asset quantity from configuration
    }
}

o3d::Bool LocalConnector::handleLimitOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_LIMIT) {
        return false;
    }

    if ((order->direction > 0 && market->ask() <= order->orderPrice) ||
        (order->direction < 0 && market->bid() >= order->orderPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        if (order->direction > 0) {
            openExecPrice = market->ask();
            closeExecPrice = market->bid();
        } else if (order->direction < 0) {
            openExecPrice = market->bid();
            closeExecPrice = market->ask();
        }

        if (market->hasPosition()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasMargin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Bool LocalConnector::handleStopOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_STOP) {
        return false;
    }

    if ((order->direction > 0 && market->bid() >= order->stopPrice) ||
        (order->direction < 0 && market->ask() <= order->stopPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        if (order->direction > 0) {
            openExecPrice = market->ask();
            closeExecPrice = market->bid();
        } else if (order->direction < 0) {
            openExecPrice = market->bid();
            closeExecPrice = market->ask();
        }

        if (market->hasPosition()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasMargin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Bool LocalConnector::handleStopLimitOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_STOP_LIMIT) {
        return false;
    }

    if ((order->direction > 0 && market->bid() >= order->stopPrice) ||
        (order->direction < 0 && market->ask() <= order->stopPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        // because of a limit price take the best
        if (order->direction > 0) {
            openExecPrice = o3d::min(order->orderPrice, market->ask());
            closeExecPrice = o3d::min(order->orderPrice, market->bid());
        } else if (order->direction < 0) {
            openExecPrice = o3d::max(order->orderPrice, market->bid());
            closeExecPrice = o3d::max(order->orderPrice, market->ask());
        }

        if (market->hasPosition()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasMargin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Bool LocalConnector::handleTakeProfitOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_TAKE_PROFIT) {
        return false;
    }

    // act as a stop but in opposite direction
    if ((order->direction > 0 && market->bid() <= order->stopPrice) ||
        (order->direction < 0 && market->ask() >= order->stopPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        if (order->direction > 0) {
            openExecPrice = market->ask();
            closeExecPrice = market->bid();
        } else if (order->direction < 0) {
            openExecPrice = market->bid();
            closeExecPrice = market->ask();
        }

        if (market->hasPosition()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasMargin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Bool LocalConnector::handleTakeProfitLimitOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_TAKE_PROFIT_LIMIT) {
        return false;
    }

    // act as a stop but in opposite direction and the as a limit order
    if ((order->direction > 0 && market->bid() <= order->stopPrice) ||
        (order->direction < 0 && market->ask() >= order->stopPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        // because of a limit price take the best
        if (order->direction > 0) {
            openExecPrice = o3d::min(order->orderPrice, market->ask());
            closeExecPrice = o3d::min(order->orderPrice, market->bid());
        } else if (order->direction < 0) {
            openExecPrice = o3d::max(order->orderPrice, market->bid());
            closeExecPrice = o3d::max(order->orderPrice, market->ask());
        }

        if (market->hasPosition()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasMargin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Int32 LocalConnector::modifyPosition(const o3d::CString &positionId,
        o3d::Double stopPrice,
        o3d::Double limitPrice)
{
    if (m_traderProxy) {
        // retrieve position
        // @todo
        return Order::RET_OK;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Double LocalConnector::VirtualAccountData::updateBalance() const
{
    return 0.0;
}

void LocalConnector::VirtualAccountData::updateDrawDown()
{
    if (balance > 0 && profitLoss > 0) {
        o3d::Double dd = (profitLoss + assetProfitLoss) / balance;
        if (dd < 0.0) {
            drawDown = -dd;
            maxDrawDown = o3d::max(maxDrawDown, drawDown);
        }
    } else {
        drawDown = 0.0;
    }
}
