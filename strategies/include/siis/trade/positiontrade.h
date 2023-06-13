/**
 * @brief SiiS strategy position trade model specialization.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-28
 */

#ifndef SIIS_POSITIONTRADE_H
#define SIIS_POSITIONTRADE_H

#include "trade.h"

namespace siis {

/**
 * @brief Strategy position trade model specialization.
 * @author Frederic Scherma
 * @date 2023-04-28
 * @todo case where error during cancel an order (entry, previous stop or limit
 * Fees are in the spread in most of the cases and no commissions.
 */
class SIIS_API PositionTrade : public Trade
{
public:

    PositionTrade(TraderProxy *proxy);

    virtual ~PositionTrade() override;

    virtual void init(o3d::Double timeframe) override;

    //
    // processing
    //

    virtual void open(Strategy *strategy,
            o3d::Int32 direction,
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

    virtual o3d::Bool isTargetOrder(const o3d::String &orderId, const o3d::String &refId) const override;
    virtual o3d::Bool isTargetPosition(const o3d::String &positionId, const o3d::String &refId) const override;

    //
    // helpers
    //

    virtual o3d::String formatToStr() const override;
    virtual o3d::String stateToStr() const override;

    //
    // stats
    //

    virtual void updateStats(o3d::Double lastPrice, o3d::Double timestamp) override;

    //
    // persistance
    //

    virtual void dumps(o3d::Variadic &trades, class Market *market) const override;
    virtual void loads(const o3d::Variadic &trade) override;

private:

    o3d::CString m_entryOrderId;
    o3d::CString m_entryRefId;

    o3d::CString m_positionId;

    State m_entryState;
    State m_exitState;

    o3d::Bool m_closing;

    o3d::Double m_positionLimitPrice;   //!< position limit price or 0.0
    o3d::Double m_positionStopPrice;    //!< position stop price or 0.0
    o3d::Double m_positionQuantity;     //!< current position quantity

    void updateRealizedPnl();
};

} // namespace siis

#endif // SIIS_POSITIONTRADE_H
