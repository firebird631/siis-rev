/**
 * @brief SiiS strategy local connector implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#include "siis/connector/localconnector.h"
#include "siis/connector/traderproxy.h"

#include "siis/handler.h"

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
        return 0;
    } else {
        return -1;
    }
}

o3d::Int32 LocalConnector::cancelOrder(const o3d::String &orderId)
{
    if (m_traderProxy) {
        return 0;
    } else {
        return -1;
    }
}

o3d::Int32 LocalConnector::closePosition(const o3d::String &positionId)
{
    if (m_traderProxy) {
        return 0;
    } else {
        return -1;
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
        return 0;
    } else {
        return -1;
    }
}
