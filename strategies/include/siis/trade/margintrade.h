/**
 * @brief SiiS strategy margin trade model specialization.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_MARGINTRADE_H
#define SIIS_MARGINTRADE_H

#include "trade.h"

namespace siis {

/**
 * @brief Strategy margin trade model specialization.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API MarginTrade : public Trade
{
public:

    MarginTrade(TraderProxy *proxy);

    virtual ~MarginTrade() override;

    //
    // processing
    //

    virtual void open(
            Strategy *strategy,
            o3d::Int32 direction,
            o3d::Double orderPrice,
            o3d::Double quantity,
            o3d::Double takeProfitPrice,
            o3d::Double stopLossPrice) override;

    virtual void remove() override;
    virtual void cancelOpen() override;
    virtual void cancelClose() override;
    virtual void modifyTakeProfit(o3d::Double price, o3d::Bool asOrder) override;
    virtual void modifyStopLoss(o3d::Double price, o3d::Bool asOrder) override;
    virtual void close() override;

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

    State m_entryState;
    State m_stopState;
    State m_limitState;
};

} // namespace siis

#endif // SIIS_MARGINTRADE_H
