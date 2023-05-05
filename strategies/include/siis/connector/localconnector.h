/**
 * @brief SiiS strategy local connector implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#ifndef SIIS_LOCALCONNECTOR_H
#define SIIS_LOCALCONNECTOR_H

#include "connector.h"
#include "../statistics/statistics.h"

#include <o3d/core/hashmap.h>
#include <o3d/core/stringmap.h>
#include <o3d/core/thread.h>
#include <o3d/core/mutex.h>

#include <queue>

namespace siis {

class Handler;
class Config;

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

    virtual void init(Config *config) override;
    virtual void terminate() override;

    virtual void start() override;
    virtual void stop() override;

    virtual void update() override;

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
    virtual void fetchOrder(const o3d::CString& marketId) override;

    virtual o3d::Int32 createOrder(Order *order) override;

    virtual o3d::Int32 cancelOrder(const o3d::CString &orderId) override;

    //
    // position
    //

    virtual void fetchAnyPositions() override;
    virtual void fetchPositions(const o3d::CString& marketId) override;

    virtual o3d::Int32 closePosition(const o3d::CString &positionId,
                                     o3d::Int32 direction,
                                     o3d::Double quantity,
                                     o3d::Bool taker,
                                     o3d::Double limitPrice) override;

    virtual o3d::Int32 modifyPosition(
            const o3d::CString &positionId,
            o3d::Double stopPrice,
            o3d::Double limitPrice) override;

    //
    // account
    //

    virtual void fetchAccount() override;

    //
    // asset
    //

    virtual void fetchAnyAssets() override;

    virtual void fetchAssets(const o3d::CString& assetId) override;

protected:

    Handler *m_handler;

    o3d::Bool m_running;

    TraderProxy *m_traderProxy;

    struct VirtualAsset
    {
        o3d::CString symbol;
        o3d::Double free;
        o3d::Double locked;

        inline o3d::Double total() const { return free + locked; }
    };

    struct VirtualAccountData
    {
        o3d::Double balance = 1000.0;
        o3d::Double usedBalance = 0.0;
        o3d::Double profitLoss = 0.0;
        o3d::Double assetProfitLoss = 0.0;
        o3d::Double drawDown = 0.0;
        o3d::Double maxDrawDown = 0.0;

        o3d::CString currency = "USD";
        o3d::Int32 precision = 2;

        std::vector<AccountSample> samples;  //! per day sample of the state of the account
        o3d::CStringMap<VirtualAsset> assets;

        o3d::Double updateBalance() const;
        void updateDrawDown();
    };

    VirtualAccountData m_virtualAccount;

    o3d::StringMap<Order*> m_virtualOrders;
    o3d::StringMap<Position*> m_virtualPositions;
};

} // namespace siis

#endif // SIIS_LOCALCONNECTOR_H
