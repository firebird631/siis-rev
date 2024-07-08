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
class Market;

/**
 * @brief SiiS strategy local connector implementation.
 * @author Frederic Scherma
 * @date 2019-06-03
 * Always connected, no fetching, no outside data, state, only accept
 * order, position modification, deletion, no subscriptions.
 * @todo mutex locker to protect maps when used in multi market async
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
    o3d::FastMutex m_mutex;

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

        o3d::Double drawDownRate = 0.0;  //!< in percentile
        o3d::Double drawDown = 0.0;      //!< in currency

        o3d::Double maxDrawDownRate = 0.0;   //!< in percentile
        o3d::Double maxDrawDown = 0.0;       //!< in currency

        o3d::CString currency = "USD";
        o3d::Int32 precision = 2;

        std::vector<AccountSample> samples;  //!< per day sample of the state of the account
        o3d::CStringMap<VirtualAsset> assets;

        o3d::Double updateBalance() const;
        void updateDrawDown();
    };

    VirtualAccountData m_virtualAccount;

    o3d::StringMap<Order*> m_virtualOrders;
    o3d::StringMap<Position*> m_virtualPositions;

    std::list<Order*> m_removedOrders;
    std::list<Position*> m_removedPositions;

    o3d::Bool _handleLimitOrder(Order *order, const Market *market);
    o3d::Bool _handleStopOrder(Order *order, const Market *market);
    o3d::Bool _handleStopLimitOrder(Order *order, const Market *market);
    o3d::Bool _handleTakeProfitOrder(Order *order, const Market *market);
    o3d::Bool _handleTakeProfitLimitOrder(Order *order, const Market *market);

    //
    // asset order management (@see localconnectorasset.cpp)
    //

    o3d::Int32 _execAssetOrder(Order *order, const Market *market,
                               o3d::Double openExecPrice, o3d::Double closeExecPrice);

    o3d::Int32 _execBuyAssetOrder(Order *order, const Market *market, o3d::Double openExecPrice);
    o3d::Int32 _execSellAssetOrder(Order *order, const Market *market, o3d::Double closeExecPrice);

    //
    // indivisible margin position and order management (@see localconnectorindmargin.cpp)
    //

    o3d::Int32 _execIndMarginOrder(Order *order, const Market *market,
                                   o3d::Double openExecPrice, o3d::Double closeExecPrice);

    o3d::Int32 _indMarginOpenPosition(Order* order, const Market *market, o3d::Double openExecPrice);
    o3d::Int32 _indMarginIncreasePosition(Order* order, Position *position, const Market *market,
                                          o3d::Double openExecPrice);
    o3d::Int32 _indMarginClosePosition(Order* order, Position *position, const Market *market,
                                       o3d::Double closeExecPrice);
    o3d::Int32 _indMarginReversePosition(Order* order, Position *position, const Market *market,
                                         o3d::Double closeExecPrice);

    //
    // margin position and FIFO order management (@see localconnectorfifomargin.cpp)
    //

    o3d::Int32 _execFifoMarginOrder(Order *order, const Market *market,
                                    o3d::Double openExecPrice, o3d::Double closeExecPrice);

    //
    // individual position and order management (@see localconnectorposition.cpp)
    //

    o3d::Int32 _execPositionOrder(Order *order, const Market *market,
                                  o3d::Double openExecPrice, o3d::Double closeExecPrice);
    o3d::Int32 _createPosition(Order *order, const Market *market, o3d::Double openExecPrice);
    o3d::Int32 _reducePosition(Position *position, const Market *market, o3d::Double closeExecPrice);
    o3d::Int32 _closePosition(Position *order, const Market *market, o3d::Double closeExecPrice);

    void _updatePosition(Position *position, const Market *market);
};

} // namespace siis

#endif // SIIS_LOCALCONNECTOR_H
