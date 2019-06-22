/**
 * @brief SiiS strategy indivisible position, margin trade model specialization.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_INDMARGINTRADE_H
#define SIIS_INDMARGINTRADE_H

#include "trade.h"

namespace siis {

/**
 * @brief Strategy indivisible position, margin trade model specialization.
 * @author Frederic Scherma
 * @date 2019-03-17
 * Some brokers only offers a unique position per market, without hedging neither individual position management.
 * In that case this trade model is prefered.
 */
class SIIS_API IndMarginTrade : public Trade
{
public:

    IndMarginTrade();
    IndMarginTrade(o3d::Double timeframe);

    virtual ~IndMarginTrade() override;

    //
    // processing
    //

    virtual void open(
            TraderProxy *trader,
            Market *market,
            o3d::Int32 direction,
            OrderType orderType,
            o3d::Double orderPrice,
            o3d::Double quantity,
            o3d::Double stopLossPrice,
            o3d::Double takeProfitPrice,
            o3d::Double leverage = 1.0) override;

    virtual void remove(TraderProxy *trader) override;
    virtual void cancelOpen(TraderProxy *trader) override;
    virtual void cancelClose(TraderProxy *trader) override;
    virtual void modifyTakeProfit(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder) override;
    virtual void modifyStopLoss(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder) override;
    virtual void close(TraderProxy *trader, Market *market) override;

    //
    // processing states
    //

    virtual o3d::Bool canDelete() const override;
    virtual o3d::Bool isActive() const override;
    virtual o3d::Bool isOpened() const override;
    virtual o3d::Bool isCanceled() const override;
    virtual o3d::Bool isOpening() const override;
    virtual o3d::Bool isClosing() const override;
    virtual o3d::Bool isClosed() const override;
    virtual o3d::Bool isEntryTimeout(o3d::Double timestamp, o3d::Double timeout) const override;
    virtual o3d::Bool isExitTimeout(o3d::Double timestamp, o3d::Double timeout) const override;
    virtual o3d::Bool isValid() const override;

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

    virtual void save(class Database *db, class Market *market) override;

private:

    State m_entryState;
    State m_stopState;
    State m_limitState;
};

} // namespace siis

#endif // SIIS_INDMARGINTRADE_H
