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
            // @todo
        }
        else if (order->orderType == Order::ORDER_STOP) {
            // @todo
        }
        else if (order->orderType == Order::ORDER_STOP_LIMIT) {
            // @todo
        }
        else if (order->orderType == Order::ORDER_TAKE_PROFIT) {
            // @todo
        }
        else if (order->orderType == Order::ORDER_TAKE_PROFIT_LIMIT) {
            // @todo
        }
    }

    // @todo update positions
    for (auto it = m_virtualPositions.begin(); it != m_virtualPositions.end(); ++it) {
        Position *position = it->second;
        const Market *market = position->strategy->market();

        if (position->quantity > 0.0) {
            // update position state, profit and loss...
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
        if (order->orderType == Order::ORDER_MARKET) {
            // direct execution and return
            const Strategy *strategy = order->strategy;
            const Market* market = strategy->market();

            order->orderId = o3d::Uuid::uuid5("siis").toCString();
            order->created = strategy->handler()->timestamp();

            order->executed = strategy->handler()->timestamp();
            order->execPrice = market->openExecPrice(order->direction);
            order->avgPrice = market->openExecPrice(order->direction);
            order->cumulativeFilled = order->orderQuantity;

            order->filled = true;

            // @todo order signal to trade manager
            // strategy->onOrderSignal(orderSignal);

            return Order::RET_OK;
        } else {
            // check and insert for later execution
            // @todo check
            order->orderId = o3d::Uuid::uuid5("siis").toCString();

            m_virtualOrders[order->orderId] = order;
        }
        return Order::RET_OK;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Int32 LocalConnector::cancelOrder(const o3d::CString &orderId)
{
    if (m_traderProxy) {
        // @todo find order and remove it, then must be delete on trade manager and free by proxy
        // if (orderId )

        return Order::RET_OK;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Int32 LocalConnector::closePosition(const o3d::CString &positionId)
{
    if (m_traderProxy) {
        return Order::RET_OK;
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
