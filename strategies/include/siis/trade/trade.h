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

#include "tradeoperation.h"

#include "../connector/ordersignal.h"
#include "../connector/positionsignal.h"

#include <o3d/core/base.h>
#include <o3d/core/variadic.h>

namespace siis {

class TraderProxy;
class Market;
class Trade;

/**
 * @brief Strategy trade statistics model.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API TradeStats
{
public:

    enum ExitReason {
        REASON_NONE = 0,
        REASON_TAKE_PROFIT_MARKET = 1,   //!< take-profit market hit
        REASON_TAKE_PROFIT_LIMIT = 2,    //!< take-profit limit hit
        REASON_STOP_LOSS_MARKET = 3,     //!< stop-loss market hit
        REASON_STOP_LOSS_LIMIT = 4,      //!< stop-loss limit hit
        REASON_CLOSE_MARKET = 5,         //!< exit signal at market
        REASON_CANCELED_TIMEOUT = 6,     //!< canceled after a timeout expiration delay
        REASON_CANCELED_TARGETED = 7,    //!< canceled before entering because take-profit price reached before entry price
        REASON_MARKET_TIMEOUT = 8        //!< closed (in profit or in loss) after a timeout
    };

    o3d::Double bestPrice;
    o3d::Double bestPriceTimestamp;

    o3d::Double worstPrice;
    o3d::Double worstPriceTimestamp;

    Order::OrderType entryOrderType;
    Order::OrderType takeProfitOrderType;
    Order::OrderType stopOrderType;

    o3d::Double firstRealizedEntryTimestamp;
    o3d::Double lastRealizedEntryTimestamp;

    o3d::Double firstRealizedExitTimestamp;
    o3d::Double lastRealizedExitTimestamp;

    o3d::Double unrealizedProfitLoss;
    o3d::CString profitLossCurrency;

    o3d::Double notionalValue;
    o3d::Double entryFees;
    o3d::Double exitFees;
    o3d::Double marginFees;

    ExitReason exitReason;

    TradeStats() :
        bestPrice(0.0),
        bestPriceTimestamp(0.0),
        worstPrice(0.0),
        worstPriceTimestamp(0.0),
        entryOrderType(Order::ORDER_UNDEFINED),
        takeProfitOrderType(Order::ORDER_UNDEFINED),
        stopOrderType(Order::ORDER_UNDEFINED),
        firstRealizedEntryTimestamp(0.0),
        lastRealizedEntryTimestamp(0.0),
        firstRealizedExitTimestamp(0.0),
        lastRealizedExitTimestamp(0.0),
        unrealizedProfitLoss(0.0),
        profitLossCurrency(),
        notionalValue(0.0),
        entryFees(0.0),
        exitFees(0.0),
        marginFees(0.0),
        exitReason(REASON_NONE)
    {
    }

    void init()
    {
        bestPrice = 0.0;
        bestPriceTimestamp = 0.0;
        worstPrice = 0.0;
        worstPriceTimestamp = 0.0;
        entryOrderType = Order::ORDER_UNDEFINED;
        takeProfitOrderType = Order::ORDER_UNDEFINED;
        stopOrderType = Order::ORDER_UNDEFINED;
        firstRealizedEntryTimestamp = 0.0;
        lastRealizedEntryTimestamp = 0.0;
        firstRealizedExitTimestamp = 0.0;
        lastRealizedExitTimestamp = 0.0;
        unrealizedProfitLoss = 0.0;
        profitLossCurrency = "";
        notionalValue = 0.0;
        entryFees = 0.0;
        exitFees = 0.0;
        marginFees = 0.0;
        exitReason = REASON_NONE;
    }

    void loads();
    void dumps() const;
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
        TYPE_ASSET = 0,
        TYPE_SPOT = 0,
        TYPE_MARGIN = 1,
        TYPE_IND_MARGIN = 2,
        TYPE_POSITION = 3,
        MAX_TYPE = TYPE_POSITION
    };

    static constexpr o3d::Int32 NUM_TYPES = MAX_TYPE + 1;

    enum OrderType {
        ORDER_CREATE = 0,
        ORDER_TP = 1,
        ORDER_SL = 2
    };

    enum ModifierType {
        MOD_NONE = 0,      //!< don't apply
        MOD_LOCAL = 1,     //!< modify the local value, managed locally
        MOD_DISTANT = 2,   //!< modify the distant value (apply on exchange by an order or on position)
        MOD_PREVIOUS = 3   //!< use previous mod, meaning if previous is distant modify distant.
    };

    /**
     * @brief The State of an entry or exit.
     */
    enum State {
        STATE_UNDEFINED = -1,
        STATE_NEW = 0,
        STATE_REJECTED = 1,
        STATE_DELETED = 2,
        STATE_CANCELED = 3,
        STATE_OPENED = 4,
        STATE_PARTIALLY_FILLED = 5,
        STATE_FILLED = 6,
        STATE_ERROR = 7
    };

    /**
     * @brief Partially compatible with order return codes.
     */
    enum ReturnCode {
        INSUFFICIENT_MARGIN = -3,
        INSUFFICIENT_FUNDS = -2,
        ERROR = -1,
        REJECTED = 0,
        ACCEPTED = 1,
        NOTHING_TO_DO = 2
    };

    Trade(TraderProxy *proxy, Type type, o3d::Double timeframe);
    virtual ~Trade();

    /**
     * @brief init Init the members.
     */
    virtual void init(o3d::Double timeframe);

    /**
     * @brief zero Zero the members.
     */
    void reset() { init(0.0); }

    //
    // getters
    //

    TraderProxy* traderProxy() { return m_proxy; }
    const TraderProxy* traderProxy() const { return m_proxy; }

    Strategy* strategy() { return m_strategy; }
    const Strategy* strategy() const { return m_strategy; }

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

    o3d::Double orderPrice() const { return m_orderPrice; }
    o3d::Double op() const { return m_orderPrice; }

    o3d::Double orderQuantity() const { return m_orderQuantity; }
    o3d::Double oq() const { return m_orderQuantity; }

    o3d::Double openTimestamp() const { return m_openTimeStamp; }
    o3d::Double openTs() const { return m_openTimeStamp; }

    o3d::Double exitTimestamp() const { return m_exitTimeStamp; }
    o3d::Double exitTs() const { return m_exitTimeStamp; }

    o3d::Double entryPrice() const { return m_entryPrice; }
    o3d::Double ep() const { return m_entryPrice; }

    o3d::Double exitPrice() const { return m_exitPrice; }
    o3d::Double xp() const { return m_exitPrice; }

    o3d::Double stopLossPrice() const { return m_stopLossPrice; }
    o3d::Double stopLoss() const { return m_stopLossPrice; }
    o3d::Double sl() const { return m_stopLossPrice; }

    o3d::Double takeProfitPrice() const { return m_takeProfitPrice; }
    o3d::Double takeProfit() const { return m_takeProfitPrice; }
    o3d::Double tp() const { return m_takeProfitPrice; }

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
     * @param strategy Valid related strategy.
     * @param direction -1 for short, 1 for long.
     * @param orderType Order type (market, limit, stop).
     * @param orderPrice Must be valid for limit or stop, optional for market.
     * @param quantity
     * @param takeProfitPrice If > 0 then defines the take-profit (limit) price of the position or a SL order.
     * @param stopLossPrice If > 0 then defines the stop-loss (stop) price of the position or a TP order.
     */
    virtual void open(
            Strategy *strategy,
            o3d::Int32 direction,
            Order::OrderType orderType,
            o3d::Double orderPrice,
            o3d::Double quantity,
            o3d::Double takeProfitPrice,
            o3d::Double stopLossPrice) = 0;

    /**
     * @brief remove Remove the trade and related existings orders.
     */
    virtual void remove() = 0;

    /**
     * @brief cancelOpen Cancel the entire or remaining open order.
     */
    virtual void cancelOpen() = 0;

    /**
     * @brief Cancel the entire or remaining close order.
     */
    virtual void cancelClose() = 0;

    /**
     * @brief modifyTakeProfit Create/modify the take-order limit order or position limit.
     * @param mod Apply locally or distant or as previous.
     * If false and previous order exists it is canceled.
     */
    virtual void modifyTakeProfit(o3d::Double price, ModifierType mod = MOD_LOCAL) = 0;

    /**
     * @brief modifyStopLoss Create/modify the stop-loss taker order or position stop-loss.
     * @param mod Apply locally or distant or as previous.
     * If false and previous order exists it is canceled.
     */
    virtual void modifyStopLoss(o3d::Double price, ModifierType mod = MOD_LOCAL) = 0;

    /**
     * @brief close Close the position or sell the entire asset.
     * Any others related orders are canceled.
     */
    virtual void close(TradeStats::ExitReason reason) = 0;

    //
    // processing states
    //

    /**
     * @brief canDelete Is the trade can be deleted.
     * Either the trade is completed, or rejected, or in a state meaning the trade manager of the strategy can
     * delete it.
     */
    o3d::Bool canDelete() const;

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
     * @param timeout Timeout delay if not specified or zero use m_entryTimeout.
     */
    o3d::Bool isEntryTimeout(o3d::Double timestamp, o3d::Double timeout = 0.0) const;

    /**
     * @brief isTradeTimeout Return true if the trade has expired after a partial or complete entry.
     * @param timestamp Current timestamp.
     * @param timeout Timeout delay. If not specified or zero use m_expiry.
     */
    o3d::Bool isTradeTimeout(o3d::Double timestamp, o3d::Double timeout = 0.0) const;

    //
    // signals update
    //

    /**
     * @brief process Process operation like local take-profit or stop-loss.
     * @param timestamp Current timestamp
     * @param lastTimestamp Previous strategy update timestamp
     */
    virtual void process(o3d::Double timestamp) = 0;

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
     * @brief toString Format to a human readable string principals trade informations.
     */
    virtual o3d::String formatToStr() const = 0;

    /**
     * @brief estimateProfitLossRate Compute a precise estimation of the current realized and unrealized rate.
     */
    o3d::Double estimateProfitLossRate() const;

    /**
     * @brief estimateTakeProfitRate Compute a precise estimation of the possible realized profit.
     */
    o3d::Double estimateTakeProfitRate() const;

    /**
     * @brief estimateProfitLossRate Compute a precise estimation of the possible realized loss.
     */
    o3d::Double estimateStopLossRate() const;

    /**
     * @brief entryFeesRate Return the applied entry fees rate.
     */
    o3d::Double entryFeesRate() const;

    /**
     * @brief exitFeesRate Return the applied exit fees rate.
     */
    o3d::Double exitFeesRate() const;

    /**
     * @brief estimateExitFeesRate Estimate the exit fees rate that will be applied.
     * It depends of the type of the exit order. Default use taker fees.
     */
    o3d::Double estimateExitFeesRate() const;

    /**
     * @brief deltaPrice Return the delta price from the entry to the last price.
     */
    o3d::Double deltaPrice() const;

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

    /**
     * @brief dumps Dumps and push the persistant content of the trade to a variadic array.
     * @param trades Valid Variadic of type array.
     * @param market Related market.
     */
    virtual void dumps(o3d::Variadic &trades, class Market *market) const = 0;

    /**
     * @brief loads Loads the persistant content of the trade from a variadic object.
     * @param trade Valid variadic of type object.
     */
    virtual void loads(const o3d::Variadic &trade) = 0;

    //
    // statistics
    //

    virtual void updateStats(o3d::Double lastPrice, o3d::Double timestamp);

    const TradeStats& stats() const { return m_stats; }

    //
    // operations
    //

    void addOperation(TradeOperation *tradeOp);

    void removeOperation(o3d::Int32 id);

    /**
     * @brief operations List all pending/peristants operations
     */
    // const std::list<TradeOperation*> operations() const { return m_operations; }

    /**
     * @brief cleanupOperations Regenerate the list of operations by removing the finished operations.
     */
    void cleanupOperations();

    // o3d::Bool hasOperations() { return !m_operations.empty(); }

protected:

    TraderProxy *m_proxy;   //!< must be valid
    Strategy *m_strategy;   //!< must be valid at open at least

    o3d::Int32 m_id;
    Type m_type;
    Mode m_mode;                  //!< real trade or on the paper trader

    o3d::Double m_timeframe;
    o3d::Double m_timestamp;      //!< trade entry order creation timestamp

    o3d::Int32 m_direction;

    o3d::Double m_expiry;          //!< max life duration of the trade
    o3d::Double m_entryTimeout;    //!< max life duration of the entry order to be filled or partially

    o3d::Double m_openTimeStamp;   //!< first open trade timestamp
    o3d::Double m_exitTimeStamp;   //!< last exit trade timestamp

    o3d::Double m_entryPrice;      //!< average realized entry price
    o3d::Double m_exitPrice;       //!< average realized exit price

    o3d::Double m_takeProfitPrice;
    o3d::Double m_stopLossPrice;

    o3d::Double m_orderPrice;
    o3d::Double m_orderQuantity;

    o3d::Double m_filledEntryQuantity;
    o3d::Double m_filledExitQuantity;

    o3d::Double m_profitLossRate;

    // std::list<TradeOperation> m_operations;
    TradeStats m_stats;
};

} // namespace siis

#endif // SIIS_TRADE_H
