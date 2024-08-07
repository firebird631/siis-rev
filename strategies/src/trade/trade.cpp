/**
 * @brief SiiS strategy trade model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/trade/trade.h"
#include "siis/trade/tradeoperation.h"
#include "siis/utils/common.h"
#include "siis/strategy.h"
#include "siis/handler.h"

using namespace siis;

Trade::Trade(TraderProxy *proxy, Type type, o3d::Double timeframe) :
    m_proxy(proxy),
    m_strategy(nullptr),
    m_id(-1),
    m_type(type),
    m_timeframe(timeframe),
    m_timestamp(0.0),
    m_direction(0),
    m_dirty(false),
    m_expiry(0.0),
    m_entryTimeout(0.0),
    m_openTimeStamp(0.0),
    m_exitTimeStamp(0.0),
    m_entryPrice(0.0),
    m_exitPrice(0.0),
    m_takeProfitPrice(0.0),
    m_stopLossPrice(0.0),
    m_orderPrice(0.0),
    m_orderQuantity(0.0),
    m_filledEntryQuantity(0.0),
    m_filledExitQuantity(0.0),
    m_profitLossRate(0.0)
{

}

Trade::~Trade()
{

}

void Trade::init(o3d::Double timeframe)
{
    m_id = -1;
    m_timeframe = timeframe;
    m_timestamp = 0.0;
    m_direction = 0;
    m_dirty = false;
    m_expiry = 0.0;
    m_entryTimeout = 0.0;
    m_openTimeStamp = 0.0;
    m_exitTimeStamp = 0.0;
    m_entryPrice = 0.0;
    m_exitPrice = 0.0;
    m_takeProfitPrice = 0.0;
    m_stopLossPrice = 0.0;
    m_orderPrice = 0.0;
    m_orderQuantity = 0.0;
    m_filledEntryQuantity = 0.0;
    m_filledExitQuantity = 0.0;
    m_profitLossRate = 0.0;

    m_stats.init();
}

o3d::Bool Trade::hasStopOrder() const
{
    return false;
}

o3d::Bool Trade::hasLimitOrder() const
{
    return false;
}

o3d::Bool Trade::supportBothOrder() const
{
    return false;
}

o3d::Bool Trade::isDirty() const
{
    return m_dirty;
}

o3d::Bool Trade::canDelete() const
{
    if (isClosed()) {
        return true;
    }

    if (isCanceled()) {
        return true;
    }

    return false;
}

o3d::Bool Trade::isActive() const
{
    return m_filledEntryQuantity > 0.0 && m_filledExitQuantity < m_filledEntryQuantity;
}

o3d::Bool Trade::isEntryTimeout(o3d::Double timestamp, o3d::Double timeout) const
{
    if (timeout <= 0.0) {
        timeout = m_entryTimeout;
    }
    if (timestamp > 0.0 && timeout > 0.0) {
        return isOpened() && m_openTimeStamp > 0.0  && timestamp - m_openTimeStamp > timeout;
    }

    return false;
}

o3d::Bool Trade::isTradeTimeout(o3d::Double timestamp, o3d::Double timeout) const
{
    if (timeout <= 0.0) {
        timeout = m_expiry;
    }

    if (timestamp > 0.0 && timeout > 0.0) {
        return isActive() && m_openTimeStamp > 0.0 && timestamp - m_openTimeStamp > timeout;
    }

    return false;
}

o3d::Double Trade::estimateProfitLossRate() const
{
    if (m_entryPrice <= 0.0) {
        return 0.0;
    }

    // estimation at close price
    o3d::Double closeExecPrice = m_strategy->market()->closeExecPrice(m_direction);

    if (closeExecPrice <= 0.0) {
        return 0.0;
    }

    o3d::Double pnl = m_direction * (closeExecPrice - m_entryPrice) / m_entryPrice;

    // minus realized entry fees rate
    pnl -= entryFeesRate();

    // and estimation of the exit fees rate
    pnl -= estimateExitFeesRate();

    return pnl;
}

o3d::Double Trade::estimateTakeProfitRate() const
{
    if (m_takeProfitPrice > 0.0) {
        o3d::Double entryPrice = m_entryPrice;
        if (entryPrice <= 0.0) {
            entryPrice = m_orderPrice;
        }

        return m_direction * (m_takeProfitPrice - entryPrice) / entryPrice;
    }

    return 0.0;
}

o3d::Double Trade::estimateStopLossRate() const
{
    if (m_stopLossPrice > 0.0) {
        o3d::Double entryPrice = m_entryPrice;
        if (entryPrice <= 0.0) {
            entryPrice = m_orderPrice;
        }

        return -m_direction * (entryPrice - m_stopLossPrice) / entryPrice;
    }

    return 0.0;
}

o3d::Double Trade::entryFeesRate() const
{
    if (m_stats.entryFees != 0.0 && m_stats.notionalValue > 0.0) {
        return m_stats.entryFees / m_stats.notionalValue;
    }

    return 0.0;
}

o3d::Double Trade::exitFeesRate() const
{
    if (m_stats.exitFees != 0.0 && m_stats.notionalValue > 0.0) {
        return m_stats.exitFees / m_stats.notionalValue;
    }

    return 0.0;
}

o3d::Double Trade::marginFeesRate() const
{
    if (m_stats.marginFees != 0.0 && m_stats.notionalValue > 0.0) {
        return m_stats.marginFees / m_stats.notionalValue;
    }

    return 0.0;
}

o3d::Double Trade::totalFees() const
{
    return m_stats.entryFees + m_stats.exitFees + m_stats.marginFees;
}

o3d::Double Trade::totalFeesRate() const
{
    o3d::Double _totalFees = totalFees();

    if (_totalFees != 0 && m_stats.notionalValue > 0.0) {
        return _totalFees / m_stats.notionalValue;
    }

    return 0.0;
}

o3d::Double Trade::estimateExitFeesRate() const
{
    // count the exit fees related to limit order type
    if (m_stats.takeProfitOrderType == Order::ORDER_LIMIT || m_stats.takeProfitOrderType == Order::ORDER_STOP_LIMIT ||
        m_stats.takeProfitOrderType == Order::ORDER_TAKE_PROFIT_LIMIT) {

        return m_strategy->market()->makerFee().rate;

    } else if (m_stats.takeProfitOrderType == Order::ORDER_MARKET || m_stats.takeProfitOrderType == Order::ORDER_STOP ||
               m_stats.takeProfitOrderType == Order::ORDER_TAKE_PROFIT) {

        return m_strategy->market()->takerFee().rate;
    }

    return m_strategy->market()->takerFee().rate;
}


o3d::Double Trade::deltaPrice() const
{
    if (m_entryPrice > 0.0) {
        o3d::Double closeExecPrice = m_strategy->market()->closeExecPrice(m_direction);
        return closeExecPrice - m_entryPrice;
    }

    return 0.0;
}

o3d::Bool Trade::canModify(o3d::Double timeout) const
{
    return true;
}

void Trade::addOperation(TradeOperation *tradeOp)
{
    m_operations.push_back(tradeOp);
}

void Trade::removeOperation(o3d::Int32 id)
{
    for (auto it = m_operations.begin(); it != m_operations.end(); ++it) {
        if ((*it)->id() == id) {
            m_operations.erase(it);
            return;
        }
    }
}

void Trade::updateStats(o3d::Double lastPrice, o3d::Double timestamp)
{
    if (/*isActive() && */lastPrice > 0.0 && timestamp > 0.0) {
        if (m_stats.bestPrice == 0.0 || m_stats.worstPrice == 0.0) {
            m_stats.bestPrice = m_stats.worstPrice = entryPrice();
            m_stats.bestPriceTimestamp = m_stats.worstPriceTimestamp = stats().firstRealizedEntryTimestamp;
        }

        if (m_direction > 0) {
            if (lastPrice > m_stats.bestPrice) {
                m_stats.bestPrice = lastPrice;
                m_stats.bestPriceTimestamp = timestamp;
            }

            if (lastPrice < m_stats.worstPrice) {
                m_stats.worstPrice = lastPrice;
                m_stats.worstPriceTimestamp = timestamp;
            }
        } else if (m_direction < 0) {
            if (lastPrice < m_stats.bestPrice) {
                m_stats.bestPrice = lastPrice;
                m_stats.bestPriceTimestamp = timestamp;
            }

            if (lastPrice > m_stats.worstPrice) {
                m_stats.worstPrice = lastPrice;
                m_stats.worstPriceTimestamp = timestamp;
            }
        }
    }
}

void TradeStats::loads()
{
    // @todo
    // plus specializations
}

void TradeStats::dumps() const
{
//            'modified-timestamp': modifiedTimestamp
//            'best-price': 0.0,
//            'best-timestamp': 0.0,
//            'worst-price': 0.0,
//            'worst-timestamp': 0.0,
//            'entry-order-type': Order.ORDER_LIMIT,
//            'take-profit-order-type': Order.ORDER_LIMIT,
//            'stop-order-type': Order.ORDER_MARKET,
//            'first-realized-entry-timestamp': 0.0,
//            'first-realized-exit-timestamp': 0.0,
//            'last-realized-entry-timestamp': 0.0,
//            'last-realized-exit-timestamp': 0.0,
//            'unrealized-profit-loss': 0.0,
//            'profit-loss-currency': "",
//            'exit-reason': StrategyTrade.REASON_NONE,
//            'notional-value': 0.0,
//            'entry-fees': 0.0,
//            'exit-fees': 0.0,
//            'margin-fees': 0.0

    // plus specializations
}
