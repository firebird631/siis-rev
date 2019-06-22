/**
 * @brief SiiS strategy local connector implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#ifndef SIIS_LOCALCONNECTOR_H
#define SIIS_LOCALCONNECTOR_H

#include "connector.h"

#include <o3d/core/hashmap.h>
#include <o3d/core/thread.h>
#include <o3d/core/mutex.h>

#include <queue>

namespace siis {

class Handler;

/**
 * @brief SiiS strategy local connector implementation.
 * @author Frederic Scherma
 * @date 2019-06-03
 * Always connected, no fetching, no outside data, state, only accept
 * order, position modification, deletion, no subscriptions.
 */
class SIIS_API LocalConnector : public Connector
{
public:

    LocalConnector(Handler *handler);

    virtual ~LocalConnector() override;

    virtual void start() override;
    virtual void stop() override;

    virtual void connect() override;
    virtual void disconnect() override;

    virtual void subscribe(Subscription &sub) override;
    virtual void unsubscribe(Subscription &sub) override;
    virtual void unsubscribeAll() override;

    virtual void setTraderProxy(TraderProxy *traderProxy) override;

    virtual TraderProxy* traderProxy() override;
    virtual const TraderProxy* traderProxy() const override;

    virtual Handler* handler() override;
    virtual const Handler* handler() const override;

    //
    // order
    //

    virtual void fetchAnyOrders() override;
    virtual void fetchOrder(const o3d::String& marketId) override;

    virtual o3d::Int32 createOrder(Order *order) override;

    virtual o3d::Int32 cancelOrder(const o3d::String &orderId) override;

    //
    // position
    //

    virtual void fetchAnyPositions() override;
    virtual void fetchPositions(const o3d::String& marketId) override;

    virtual o3d::Int32 closePosition(const o3d::String &positionId) override;

    virtual o3d::Int32 modifyPosition(
            const o3d::String &positionId,
            o3d::Double stopLossPrice,
            o3d::Double takeProfitPrice) override;

    //
    // account
    //

    virtual void fetchAccount() override;

    //
    // asset
    //

    virtual void fetchAnyAssets() override;

    virtual void fetchAssets(const o3d::String& assetId) override;

protected:

    Handler *m_handler;

    o3d::Bool m_running;

    TraderProxy *m_traderProxy;
};

} // namespace siis

#endif // SIIS_LOCALCONNECTOR_H
