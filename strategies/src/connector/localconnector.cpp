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
    // @todo execute orders (stop, limit)
    for (auto it = m_virtualOrders.begin(); it != m_virtualOrders.end(); ++it) {
        // check and execute order...
        Order *order = it->second;
        const Market *market = order->strategy->market();

        if (order->orderType == Order::ORDER_LIMIT) {
            if (order->direction > 0) {
                // @todo
            } else if (order->direction < 0) {

            }
        }
        else if (order->orderType == Order::ORDER_STOP) {
            if (order->direction > 0) {
                // @todo
            } else if (order->direction < 0) {

            }
        }
        else if (order->orderType == Order::ORDER_STOP_LIMIT) {
            if (order->direction > 0) {
                // @todo
            } else if (order->direction < 0) {

            }
        }
        else if (order->orderType == Order::ORDER_TAKE_PROFIT) {
            if (order->direction > 0) {
                // @todo
            } else if (order->direction < 0) {

            }
        }
        else if (order->orderType == Order::ORDER_TAKE_PROFIT_LIMIT) {
            if (order->direction > 0) {
                // @todo
            } else if (order->direction < 0) {

            }
        }
    }

    // update positions
    for (auto it = m_virtualPositions.begin(); it != m_virtualPositions.end(); ++it) {
        Position *position = it->second;
        const Market *market = position->strategy->market();

        if (position->quantity > 0.0) {
            // update position state, profit and loss...
            // position->profitLoss =
            // @todo

            if (position->stopPrice > 0.0) {

            }

            if (position->limitPrice > 0.0) {

            }
        }
    }

    // update virtual account
    m_virtualAccount.updateBalance();
    m_virtualAccount.updateDrawDown();
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

            // excepted for spot, need a position opened signal
            if (strategy->tradeType() != Trade::TYPE_ASSET) {
                if (strategy->tradeType() == Trade::TYPE_POSITION) {
                    // with the some order as order id for a position
                    openOrderSignal.positionId = order->orderId;
                } else {
                    // or market id for margin
                    openOrderSignal.positionId = market->marketId();
                }
            }

            strategy->onOrderSignal(openOrderSignal);

            OrderSignal tradedOrderSignal(OrderSignal::TRADED);
            tradedOrderSignal.direction = order->direction;
            tradedOrderSignal.marketId = order->marketId;
            tradedOrderSignal.executed = handler()->timestamp();
            tradedOrderSignal.orderId = order->orderId;
            tradedOrderSignal.refId = order->refId;
            tradedOrderSignal.orderType = order->orderType;

            tradedOrderSignal.avgPrice = execPrice;
            tradedOrderSignal.execPrice = execPrice;
            tradedOrderSignal.filled = order->orderQuantity;
            tradedOrderSignal.cumulativeFilled = order->orderQuantity;
            tradedOrderSignal.completed = true;

            // @todo compute quote transacted
            // tradedOrderSignal.quoteTransacted =
            // @todo commission fees and its currency

            strategy->onOrderSignal(tradedOrderSignal);

            // excepted for spot, need a position opened signal
            if (strategy->tradeType() != Trade::TYPE_ASSET) {
                PositionSignal openPositionSignal(PositionSignal::OPENED);

                // create a virtual position
                Position *position = traderProxy()->newPosition(strategy);
                position->orderRefId = order->orderId;
                position->direction = order->direction;
                position->marketId = order->marketId;
                position->created = handler()->timestamp();
                position->updated = handler()->timestamp();

                position->stopPrice = order->stopPrice;
                position->limitPrice = order->limitPrice;

                if (strategy->tradeType() == Trade::TYPE_POSITION) {
                    // with the some order as order id for a position
                    position->positionId = order->orderId;
                } else {
                    // or market id for margin
                    position->positionId = market->marketId();
                }

                m_virtualPositions[position->positionId] = position;

                openPositionSignal.direction = order->direction;
                openPositionSignal.marketId = order->marketId;
                openPositionSignal.created = handler()->timestamp();
                openPositionSignal.updated = handler()->timestamp();
                openPositionSignal.orderRefId = order->orderId;
                openPositionSignal.positionId = position->positionId;
                // openPositionSignal.commission @todo

                // entry traded
                openPositionSignal.avgPrice = execPrice;
                openPositionSignal.execPrice = execPrice;
                openPositionSignal.filled = order->orderQuantity;
                openPositionSignal.cumulativeFilled = order->orderQuantity;

                strategy->onPositionSignal(openPositionSignal);
            }

            OrderSignal deletedOrderSignal(OrderSignal::DELETED);
            deletedOrderSignal.direction = order->direction;
            deletedOrderSignal.marketId = order->marketId;
            deletedOrderSignal.executed = handler()->timestamp();
            deletedOrderSignal.orderId = order->orderId;
            deletedOrderSignal.refId = order->refId;
            deletedOrderSignal.orderType = order->orderType;
            // @todo do we set cumulative, avg and completed here ?

            strategy->onOrderSignal(deletedOrderSignal);

            // finally free the order because it is fully executed
            m_traderProxy->freeOrder(order);

            return Order::RET_OK;
        } else {
            // check and insert for later execution
            // @todo check
            order->orderId = o3d::Uuid::uuid5("siis").toCString();

            m_virtualOrders[order->orderId] = order;

            // direct order signal to strategy
            Strategy *strategy = order->strategy;

            OrderSignal openOrderSignal(OrderSignal::OPENED);
            openOrderSignal.direction = order->direction;
            openOrderSignal.marketId = order->marketId;
            openOrderSignal.created = handler()->timestamp();
            openOrderSignal.orderId = order->orderId;
            openOrderSignal.refId = order->refId;
            openOrderSignal.orderType = order->orderType;

            strategy->onOrderSignal(openOrderSignal);
        }
        return Order::RET_OK;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Int32 LocalConnector::cancelOrder(const o3d::CString &orderId)
{
    if (m_traderProxy) {
        if (orderId.isValid()) {
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
    if (m_traderProxy) {
        if (positionId.isValid()) {
            auto it = m_virtualPositions.find(positionId);
            if (it != m_virtualPositions.end()) {
                Position *position = it->second;
                Strategy *strategy = position->strategy;  // @warn not sure to have it on live

                o3d::Double execPrice = strategy->market()->closeExecPrice(position->direction);

                // DBG(o3d::String("close position {0} at={1} pl={2}").arg(positionId).arg(execPrice).arg(position->profitLoss), "");

                // @todo last update and deleted signal
                PositionSignal deletedPositionSignal(PositionSignal::DELETED);

                deletedPositionSignal.direction = position->direction;
                deletedPositionSignal.marketId = position->marketId;
                deletedPositionSignal.created = position->created;
                deletedPositionSignal.updated = handler()->timestamp();
                deletedPositionSignal.orderRefId = position->orderRefId;
                deletedPositionSignal.positionId = position->positionId;
                // deletedPositionSignal.commission @todo

                // exit traded @todo correct qty
                deletedPositionSignal.avgPrice = execPrice;
                deletedPositionSignal.execPrice = execPrice;
                deletedPositionSignal.filled = position->quantity;
                deletedPositionSignal.cumulativeFilled = position->quantity;

                strategy->onPositionSignal(deletedPositionSignal);

                // finally free the position because it is fully executed
                m_virtualPositions.erase(it);
                m_traderProxy->freePosition(position);

                return Order::RET_OK;
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
