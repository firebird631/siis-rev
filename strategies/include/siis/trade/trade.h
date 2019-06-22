/**
 * @brief SiiS strategy trade model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_TRADE_H
#define SIIS_TRADE_H

#include "../base.h"
#include "../utils/common.h"

#include "../connector/ordersignal.h"
#include "../connector/positionsignal.h"

#include <o3d/core/base.h>

namespace siis {

class TraderProxy;
class Market;

/**
 * @brief Strategy trade condition model.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API TradeCondition
{
public:

    o3d::String name;

    o3d::Double v1;
    o3d::Double v2;
    o3d::Double v3;
    o3d::Double v4;

    TradeCondition(
            const o3d::String& _name,
            o3d::Double _v1,
            o3d::Double _v2 = 0.0,
            o3d::Double _v3 = 0.0,
            o3d::Double _v4 = 0.0) :
        name(_name),
        v1(_v1),
        v2(_v2),
        v3(_v3),
        v4(_v4)
    {
    }
};

/**
 * @brief Strategy trade statistics model.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API TradeStats
{
public:

    o3d::Double bestPrice;
    o3d::Double bestPriceTimestamp;

    o3d::Double worstPrice;
    o3d::Double worstPriceTimestamp;

    TradeStats() :
        bestPrice(0),
        bestPriceTimestamp(0),
        worstPrice(0),
        worstPriceTimestamp(0)
    {
    }

    void init()
    {
        bestPrice = 0;
        bestPriceTimestamp =0;
        worstPrice = 0;
        worstPriceTimestamp = 0;
    }
};

/**
 * @brief Strategy trade model.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API Trade
{
public:

    enum Mode {
        REAL = 0,
        PAPER = 1
    };

    enum Type {
        TYPE_BUY_SELL = 0,
        TYPE_MARGIN = 1,
        TYPE_IND_MARGIN = 2,
        MAX_TYPE = TYPE_IND_MARGIN
    };

    static constexpr o3d::Int32 NUM_TYPES = MAX_TYPE + 1;

    enum OrderType {
        ORDER_CREATE = 0,
        ORDER_TP = 1,
        ORDER_SL = 2
    };

    enum State {
        STATE_NONE = 0,
        STATE_NEW = 1,
        STATE_REJECTED = 2,
        STATE_DELETED = 3,
        STATE_CANCELED = 4,
        STATE_OPENED = 5,
        STATE_PARTIALLY_FILLED = 6,
        STATE_FILLED = 7
    };

    Trade(Type type, o3d::Double timeframe);
    virtual ~Trade();

    /**
     * @brief init Init the members.
     */
    void init(o3d::Double timeframe);

    //
    // getters
    //

    void setId(o3d::Int32 id) { m_id = id; }

    o3d::Int32 id() const { return m_id; }
    Type type() const { return m_type; }

    Mode mode() const { return m_mode; }

    o3d::Double timeframe() const { return m_timeframe; }
    o3d::Double tf() const { return m_timeframe; }

    o3d::Double timestamp() const { return m_timestamp; }
    o3d::Double ts() const { return m_timestamp; }

    o3d::Int32 direction() const { return m_direction; }
    o3d::Int32 dir() const { return m_direction; }
    o3d::Int32 d() const { return m_direction; }

    o3d::Double openTimestamp() const { return m_openTimeStamp; }
    o3d::Double openTs() const { return m_openTimeStamp; }

    o3d::Double exitTimestamp() const { return m_exitTimeStamp; }
    o3d::Double exitTs() const { return m_exitTimeStamp; }

    o3d::Double entryPrice() const { return m_entryPrice; }
    o3d::Double ep() const { return m_entryPrice; }

    o3d::Double stopLossPrice() const { return m_stopLossPrice; }
    o3d::Double stopLoss() const { return m_stopLossPrice; }
    o3d::Double sl() const { return m_stopLossPrice; }

    o3d::Double takeProfitPrice() const { return m_takeProfitPrice; }
    o3d::Double takeProfit() const { return m_takeProfitPrice; }
    o3d::Double tp() const { return m_takeProfitPrice; }

    o3d::Double orderQuantity() const { return m_orderQuantity; }
    o3d::Double oq() const { return m_orderQuantity; }

    o3d::Double filledEntryQuantity() const { return m_filledEntryQuantity; }
    o3d::Double eq() const { return m_filledEntryQuantity; }

    o3d::Double filledExitQuantity() const { return m_filledExitQuantity; }
    o3d::Double xq() const { return m_filledExitQuantity; }

    o3d::Double profitLossRate() const { return m_profitLossRate; }
    o3d::Double pl() const { return m_profitLossRate; }

    //
    // processing
    //

    /**
     * @brief open Initial creation of the entry order using the trader proxy and market.
     * @param trader Valid trader proxy related to the primary connector.
     * @param market Valid market related to the strategy.
     * @param direction -1 for short, 1 for long.
     * @param orderType @see OrderType
     * @param orderPrice If <= 0 then market order, else limit order.
     * @param quantity
     * @param stopLossPrice If > 0 then defines the stop-loss (stop) price of the position or a TP order.
     * @param takeProfitPrice If > 0 then defines the take-profit (limit) price of the position or a SL order.
     * @param leverage Should not be used with most of the exchanges.
     */
    virtual void open(
            TraderProxy *trader,
            Market *market,
            o3d::Int32 direction,
            OrderType orderType,
            o3d::Double orderPrice,
            o3d::Double quantity,
            o3d::Double stopLossPrice,
            o3d::Double takeProfitPrice,
            o3d::Double leverage = 1.0) = 0;

    /**
     * @brief remove Remove the trade and related existings orders.
     */
    virtual void remove(TraderProxy *trader) = 0;

    /**
     * @brief cancelOpen Cancel the entire or remaining open order.
     */
    virtual void cancelOpen(TraderProxy *trader) = 0;

    /**
     * @brief Cancel the entire or remaining close order.
     */
    virtual void cancelClose(TraderProxy *trader) = 0;

    /**
     * @brief modifyTakeProfit Create/modify the take-order limit order or position limit.
     * @param asOrder If true create/modify a limit order. If false only modify the takeProfit price member.
     * If false and previous order exists it is canceled.
     */
    virtual void modifyTakeProfit(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder) = 0;

    /**
     * @brief modifyStopLoss Create/modify the stop-loss taker order or position stop-loss.
     * @param asOrder If true create/modify a stop-loss order. If false only modify the stopLoss price member.
     * If false and previous order exists it is canceled.
     */
    virtual void modifyStopLoss(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder) = 0;

    /**
     * @brief close Close the position or sell the entire asset.
     * Any others related orders are canceled.
     */
    virtual void close(TraderProxy *trader, Market *market) = 0;

    //
    // processing states
    //

    /**
     * @brief canDelete Is the trade can be deleted.
     * Either the trade is completed, or rejected, or in a state meaning the trade manager of the strategy can
     * delete it.
     */
    virtual o3d::Bool canDelete() const = 0;

    /**
     * @brief isActive Is the entry trade accepted, and partially filled, or fully filled or exit state is defined,
     * but not fully completed.
     */
    virtual o3d::Bool isActive() const;

    /**
     * @brief isOpened Is the entry is accepted, but no quantity is still filled at this time.
     */
    virtual o3d::Bool isOpened() const = 0;

    /**
     * @brief isCanceled Is the trade is not active, canceled or rejected.
     * @return
     */
    virtual o3d::Bool isCanceled() const = 0;

    /**
     * @brief isOpening Is some entry order are orderer, or processing, but not fully filled.
     */
    virtual o3d::Bool isOpening() const = 0;

    /**
     * @brief isClosing Is some exit order are ordered, or processing, but not fully filled.
     */
    virtual o3d::Bool isClosing() const = 0;

    /**
     * @brief isClosed Is trade fully closed (all quantity closed).
     */
    virtual o3d::Bool isClosed() const = 0;

    /**
     * @brief isEntryTimeout Return true if the trade entry timeout.
     * @param timestamp Current timestamp.
     * @param timeout Timeout delay.
     */
    virtual o3d::Bool isEntryTimeout(o3d::Double timestamp, o3d::Double timeout) const = 0;

    /**
     * @brief isExitTimeout Return true if the trade exit timeout.
     * @param timestamp Current timestamp.
     * @param timeout Timeout delay.
     */
    virtual o3d::Bool isExitTimeout(o3d::Double timestamp, o3d::Double timeout) const = 0;

    /**
     * @brief isValid Return true if the trade is not expired (signal still acceptable) and entry quantity not fully filled.
     */
    virtual o3d::Bool isValid() const = 0;

    //
    // signals update
    //

    /**
     * @brief orderSignal Update the trade information according to the received order signal.
     */
    virtual void orderSignal(const OrderSignal &signal) = 0;

    /**
     * @brief positionSignal Update the trade information according to the received position signal.
     */
    virtual void positionSignal(const PositionSignal &signal) = 0;

    /**
     * @brief isTargetOrder Returns true if the id of the order or of the reference order is related to one
     * of the order of this trade.
     * @param orderId Order identifier.
     * @param orderRefId Order reference identifier.
     */
    virtual o3d::Bool isTargetOrder(const o3d::String &orderId, const o3d::String &orderRefId) const = 0;

    /**
     * @brief isTargetPosition Returns true if the id of the position of the reference order is related to one
     * of the position of this trade.
     * @param positionId Position identifier.
     * @param orderRefId Order reference identifier.
     */
    virtual o3d::Bool isTargetPosition(const o3d::String &positionId, const o3d::String &orderRefId) const = 0;

    //
    // helpers
    //

    /**
     * @brief toString Format to a human readable string principals trade information.
     */
    virtual o3d::String formatToStr() const = 0;

    /**
     * @brief stateToStr Return a string of the state of the trade
     * @return String one of (new, opened, rejected, problem, filling, filled, closing, closed, waiting).
     */
    virtual o3d::String stateToStr() const = 0;

    o3d::String directionToStr() const { return siis::directionToStr(m_direction); }
    o3d::String timeframeToStr() const { return siis::timeframeToStr(m_timeframe); }

    //
    // persistance
    //

    virtual void save(class Database *db, class Market *market) = 0;

    //
    // statistics
    //

    void updateStats(o3d::Double lastPrice, o3d::Double timestamp);

    const TradeStats& stats() const { return m_stats; }

    const std::list<TradeCondition>& getConditions() const { return m_conditions; }

    void addCondition(
            const o3d::String& name,
            o3d::Double v1,
            o3d::Double v2 = 0.0,
            o3d::Double v3 = 0.0,
            o3d::Double v4 = 0.0);

protected:

    o3d::Int32 m_id;
    Type m_type;
    Mode m_mode;                  //!< real trade or on the paper trader

    o3d::Double m_timeframe;
    o3d::Double m_timestamp;       //!< trade entry order creation timestamp

    o3d::Int32 m_direction;

    o3d::Double m_openTimeStamp;   //!< first open trade timestamp
    o3d::Double m_exitTimeStamp;   //!< last exit trade timestamp

    o3d::Double m_entryPrice;
    o3d::Double m_takeProfitPrice;
    o3d::Double m_stopLossPrice;

    o3d::Double m_orderQuantity;
    o3d::Double m_filledEntryQuantity;
    o3d::Double m_filledExitQuantity;

    o3d::Double m_profitLossRate;

    std::list<TradeCondition> m_conditions;
    TradeStats m_stats;
};

} // namespace siis

#endif // SIIS_TRADE_H
