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

    // @todo update positions
    for (auto it = m_virtualPositions.begin(); it != m_virtualPositions.end(); ++it) {
        Position *position = it->second;
        const Market *market = position->strategy->market();

        if (position->quantity > 0.0) {
            // update position state, profit and loss...
            // position->profitLoss =
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
            tradedOrderSignal.marketId = order->marketId;
            tradedOrderSignal.direction = order->direction;
            tradedOrderSignal.filled = order->orderQuantity;
            tradedOrderSignal.cumulativeFilled = order->orderQuantity;
            tradedOrderSignal.completed = true;

            // @todo compute quote transacted
            // tradedOrderSignal.quoteTransacted =
            // @todo commission fees and its currency

            strategy->onOrderSignal(tradedOrderSignal);

            OrderSignal deletedOrderSignal(OrderSignal::DELETED);
            deletedOrderSignal.direction = order->direction;
            deletedOrderSignal.marketId = order->marketId;
            deletedOrderSignal.executed = handler()->timestamp();
            deletedOrderSignal.orderId = order->orderId;
            deletedOrderSignal.refId = order->refId;
            deletedOrderSignal.orderType = order->orderType;
            // @todo do we set cumulative, avg and completed here ?

            strategy->onOrderSignal(deletedOrderSignal);

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
                // @todo

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
        o3d::Double stopLossPrice,
        o3d::Double takeProfitPrice)
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
