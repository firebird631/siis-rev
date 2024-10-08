/**
 * @brief SiiS strategy asset trade model specialization.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_ASSETTRADE_H
#define SIIS_ASSETTRADE_H

#include "trade.h"

namespace siis {

/**
 * @brief Strategy asset/spot trade model specialization.
 * @author Frederic Scherma
 * @date 2019-03-17
 * @todo distinct limit and stop (per order) qty and exec qty, and distincts fees, from them
 * recompute exit state qty and fees
 */
class SIIS_API AssetTrade : public Trade
{
public:

    AssetTrade(TraderProxy *proxy);

    virtual ~AssetTrade() override;

    virtual void init(o3d::Double timeframe) override;

    //
    // capacities
    //

    virtual o3d::Bool hasStopOrder() const override;
    virtual o3d::Bool hasLimitOrder() const override;
    virtual o3d::Bool supportBothOrder() const override;

    //
    // processing
    //

    virtual void open(
            Strategy *strategy,
            o3d::Int32 direction,
            Order::OrderType orderType,
            o3d::Double orderPrice,
            o3d::Double quantity,
            o3d::Double takeProfitPrice,
            o3d::Double stopLossPrice) override;

    virtual void remove() override;
    virtual void cancelOpen() override;
    virtual void cancelClose() override;
    virtual void modifyTakeProfit(o3d::Double price, ModifierType mod = MOD_LOCAL) override;
    virtual void modifyStopLoss(o3d::Double price, ModifierType mod = MOD_LOCAL) override;
    virtual void close(TradeStats::ExitReason reason) override;

    //
    // processing states
    //

    virtual void process(o3d::Double timestamp) override;

    virtual o3d::Bool isActive() const override;
    virtual o3d::Bool isOpened() const override;
    virtual o3d::Bool isCanceled() const override;
    virtual o3d::Bool isOpening() const override;
    virtual o3d::Bool isClosing() const override;
    virtual o3d::Bool isClosed() const override;

    //
    // signals update
    //

    virtual void orderSignal(const OrderSignal &signal) override;
    virtual void positionSignal(const PositionSignal &signal) override;

    virtual o3d::Bool isTargetOrder(const o3d::String &orderId, const o3d::String &orderRefId) const override;
    virtual o3d::Bool isTargetPosition(const o3d::String &positionId, const o3d::String &orderRefId) const override;

    //
    // helpers
    //

    virtual o3d::String formatToStr() const override;
    virtual o3d::String stateToStr() const override;

    //
    // persistance
    //

    virtual void dumps(o3d::Variadic &trades, class Market *market) const override;
    virtual void loads(const o3d::Variadic &trade) override;

private:

    State m_buyState;
    State m_stopState;
    State m_limitState;
};

} // namespace siis

#endif // SIIS_ASSETTRADE_H
