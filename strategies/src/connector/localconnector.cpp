/**
 * @brief SiiS strategy local connector implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#include "siis/connector/localconnector.h"
#include "siis/connector/traderproxy.h"

#include "siis/handler.h"
#include "siis/config/config.h"

#include <o3d/core/debug.h>

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
    // @todo update positions
    for (Position *position : m_virtualPositions) {

    }

    // @todo execute orders (stop, limit)
    for (Order *order : m_virtualOrders) {

    }

    // update virtual account
    // @todo
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

void LocalConnector::fetchOrder(const o3d::String &marketId)
{
    // nothing
}

void LocalConnector::fetchAnyPositions()
{
    // nothing
}

void LocalConnector::fetchPositions(const o3d::String &marketId)
{
    // nothing
}

o3d::Int32 LocalConnector::createOrder(Order *order)
{
    if (m_traderProxy) {
        if (order->orderType == Order::ORDER_MARKET) {
            // direct execution and return
            // @todo execution
            return Order::RET_OK;
        } else {
            // check and insert for later execution
            // @todo check
            m_virtualOrders.push_back(order);
        }
        return Order::RET_OK;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Int32 LocalConnector::cancelOrder(const o3d::String &orderId)
{
    if (m_traderProxy) {
        return 0;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Int32 LocalConnector::closePosition(const o3d::String &positionId)
{
    if (m_traderProxy) {
        return 0;
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

void LocalConnector::fetchAssets(const o3d::String &assetId)
{
    if (m_traderProxy) {
        // @todo could set initial asset quantity from configuration
    }
}

o3d::Int32 LocalConnector::modifyPosition(
        const o3d::String &positionId,
        o3d::Double stopLossPrice,
        o3d::Double takeProfitPrice)
{
    if (m_traderProxy) {
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
