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
 * Some CEX only offers a unique position per market, without hedging neither individual position management.
 * In that case this trade model is prefered.
 * @todo case where error during cancel an order (entry, previous stop or limit)
 * Fees are generally taken from equity and generally no commissions.
 * Pnl is reduced from fees rates.
 * @todo distinct limit and stop (per order) qty and exec qty, and distincts fees, from them
 * recompute exit state qty and fees
 */
class SIIS_API IndMarginTrade : public Trade
{
public:

    IndMarginTrade(TraderProxy *proxy);

    virtual ~IndMarginTrade() override;

    virtual void init(o3d::Double timeframe) override;

    //
    // processing
    //

    virtual void open(Strategy *strategy,
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

    struct EntryExit
    {
        State state = STATE_UNDEFINED;

        o3d::CString orderId;
        o3d::CString refId;

        o3d::Double executed = 0.0;

        void reset() {
            state = STATE_UNDEFINED;
            orderId = "";
            refId = "";
            executed = 0.0;
        }

        inline o3d::Bool hasOrder() const {
            return orderId.isValid() || refId.isValid();
        }
    };

    /**
     * @brief For market or limit entry order.
     */
    struct Entry : EntryExit {};

    /**
     * @brief For stop (loss or in profit) and for market close order.
     */
    struct Stop : EntryExit
    {
        void reset() {
            EntryExit::reset();

            closing = false;

            orderedQty = 0.0;
            fees = 0.0;
        }

        o3d::Bool closing = false;
        o3d::Double orderedQty = 0.0;  //!< ordered qty
        o3d::Double fees = 0.0;        //! order relative fees
    };

    /**
     * @brief For limit (loss or in profit) order.
     */
    struct Limit : EntryExit
    {
        void reset() {
            EntryExit::reset();

            orderedQty = 0.0;
            fees = 0.0;
        }

        o3d::Double orderedQty = 0.0;  //!< ordered qty
        o3d::Double fees = 0.0;        //! order relative fees
    };

    o3d::CString m_positionId;   //!< Mostly similar to market id

    Entry m_entry;
    Stop m_stop;
    Limit m_limit;

    void updateRealizedPnl();
};

} // namespace siis

#endif // SIIS_INDMARGINTRADE_H
