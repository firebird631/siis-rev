/**
 * @brief SiiS strategy margin trade model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/trade/margintrade.h"
#include "siis/utils/common.h"
#include "siis/connector/traderproxy.h"
#include "siis/strategy.h"
#include "siis/market.h"

using namespace siis;

MarginTrade::MarginTrade(TraderProxy *proxy) :
    Trade(proxy, Trade::TYPE_MARGIN, -1.0),
    m_entry(),
    m_stop(),
    m_limit()
{

}

MarginTrade::~MarginTrade()
{

}

void MarginTrade::init(o3d::Double timeframe)
{
    Trade::init(timeframe);

    m_entry.reset();
    m_stop.reset();
    m_limit.reset();

    m_positionId = "";
}

void MarginTrade::open(Strategy *strategy,
        o3d::Int32 direction,
        Order::OrderType orderType,
        o3d::Double orderPrice,
        o3d::Double quantity,
        o3d::Double takeProfitPrice,
        o3d::Double stopLossPrice)
{
    m_strategy = strategy;

    m_direction = direction;
    m_orderQuantity = quantity;
    m_orderPrice = orderPrice;
    m_takeProfitPrice = takeProfitPrice;
    m_stopLossPrice = stopLossPrice;

    Order *entryOrder = traderProxy()->newOrder(m_strategy);
    entryOrder->direction = direction;
    entryOrder->orderQuantity = quantity;
    entryOrder->strategy = m_strategy;
    entryOrder->orderType = orderType;
    entryOrder->orderPrice = orderPrice;

    m_entry.refId = entryOrder->refId;
    m_stats.entryOrderType = entryOrder->orderType;

    o3d::Int32 ret = traderProxy()->createOrder(entryOrder);
    if (ret == Order::RET_OK) {
    } else {
        m_entry.state = STATE_REJECTED;
    }
}

void MarginTrade::remove()
{
    if (m_stop.closing) {
        return;
    }

    if (m_entry.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_entry.orderId);
        if (ret == Order::RET_OK) {
            m_entry.orderId = "";
            m_entry.refId = "";

            m_entry.state = STATE_CANCELED;
        } else {

        }
    }

    if (m_stop.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_stop.orderId);
        if (ret == Order::RET_OK) {
            m_stop.orderId = "";
            m_stop.refId = "";

            m_stop.state = STATE_CANCELED;
        } else {

        }
    }

    if (m_limit.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_limit.orderId);
        if (ret == Order::RET_OK) {
            m_limit.orderId = "";
            m_limit.refId = "";

            m_limit.state = STATE_CANCELED;
        } else {

        }
    }
}

void MarginTrade::cancelOpen()
{
    if (isOpened()) {
        if (m_entry.hasOrder()) {
            o3d::Int32 ret = traderProxy()->cancelOrder(m_entry.orderId);
            if (ret == Order::RET_OK) {
                m_entry.orderId = "";
                m_entry.refId = "";

                m_entry.state = STATE_CANCELED;
            } else {

            }
        }
    }
}

void MarginTrade::cancelClose()
{
    if (m_stop.closing) {
        return;
    }

    if (!isActive()) {
        return;
    }

    if (m_stop.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_stop.orderId);
        if (ret == Order::RET_OK) {
            m_stop.orderId = "";
            m_stop.refId = "";

            m_stop.state = STATE_CANCELED;
        } else {

        }
    }

    if (m_limit.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_limit.orderId);
        if (ret == Order::RET_OK) {
            m_limit.orderId = "";
            m_limit.refId = "";

            m_limit.state = STATE_CANCELED;
        } else {

        }
    }
}

void MarginTrade::modifyTakeProfit(o3d::Double price, ModifierType mod)
{
    if (m_stop.closing) {
        return;
    }

    if (m_limit.hasOrder()) {
        if (mod == MOD_PREVIOUS) {
            mod = MOD_DISTANT;
        }

        o3d::Int32 ret = traderProxy()->cancelOrder(m_limit.orderId);
        if (ret == Order::RET_OK) {
            m_limit.orderId = "";
            m_limit.refId = "";

            m_limit.state = STATE_CANCELED;
        } else {

        }
    }

    if (mod == MOD_DISTANT) {
        if (price > 0.0) {
            o3d::Double remaining_qty =  m_filledEntryQuantity - m_filledExitQuantity;

            Order *limitOrder = traderProxy()->newOrder(m_strategy);
            limitOrder->direction = -m_direction;
            limitOrder->orderQuantity = remaining_qty;

            limitOrder->orderType = Order::ORDER_LIMIT;
            limitOrder->orderPrice = price;
            limitOrder->closeOnly = 1;

            m_limit.refId = limitOrder->refId;

            o3d::Int32 ret = traderProxy()->createOrder(limitOrder);
            if (ret == Order::RET_OK) {
            } else {

            }
        }
    }

    m_takeProfitPrice = price;
}

void MarginTrade::modifyStopLoss(o3d::Double price, ModifierType mod)
{
    if (m_stop.closing) {
        return;
    }

    if (m_stop.hasOrder()) {
        if (mod == MOD_PREVIOUS) {
            mod = MOD_DISTANT;
        }

        o3d::Int32 ret = traderProxy()->cancelOrder(m_stop.orderId);
        if (ret == Order::RET_OK) {
            m_stop.orderId = "";
            m_stop.refId = "";

            m_stop.state = STATE_CANCELED;
        } else {

        }
    }

    if (mod == MOD_DISTANT) {
        if (price > 0.0) {
            o3d::Double remaining_qty =  m_filledEntryQuantity - m_filledExitQuantity;

            Order *stopOrder = traderProxy()->newOrder(m_strategy);
            stopOrder->direction = -m_direction;
            stopOrder->orderQuantity = remaining_qty;

            stopOrder->orderType = Order::ORDER_MARKET;
            stopOrder->orderPrice = price;
            stopOrder->closeOnly = 1;

            m_stop.refId = stopOrder->refId;

            o3d::Int32 ret = traderProxy()->createOrder(stopOrder);
            if (ret == Order::RET_OK) {
            } else {

            }
        }
    }

    m_stopLossPrice = price;
}

void MarginTrade::close(TradeStats::ExitReason reason)
{
    if (m_stop.closing) {
        return;
    }

    o3d::Double remaining_qty = m_filledEntryQuantity - m_filledExitQuantity;
    if (remaining_qty <= 0.0) {
        return;
    }

    if (m_stop.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_stop.orderId);
        if (ret == Order::RET_OK) {
            m_stop.orderId = "";
            m_stop.refId = "";

            m_stop.state = STATE_CANCELED;
        } else {

        }
    }

    if (m_limit.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_limit.orderId);
        if (ret == Order::RET_OK) {
            m_limit.orderId = "";
            m_limit.refId = "";

            m_limit.state = STATE_CANCELED;
        } else {

        }
    }

    m_stats.exitReason = reason;

    Order *stopOrder = traderProxy()->newOrder(m_strategy);
    stopOrder->direction = -m_direction;
    stopOrder->orderQuantity = remaining_qty;
    stopOrder->orderType = Order::ORDER_MARKET;
    stopOrder->closeOnly = 1;

    o3d::Int32 ret = traderProxy()->createOrder(stopOrder);
    if (ret == Order::RET_OK) {
    } else {

    }
}

void MarginTrade::process(o3d::Double timestamp)
{
    if (isActive()) {
        if (m_stopLossPrice > 0.0 && !m_stop.orderId.isValid()) {
            o3d::Double closeExecPrice = m_strategy->market()->closeExecPrice(m_direction);

            if (m_direction > 0) {
                if (closeExecPrice < m_stopLossPrice) {
                    close(TradeStats::REASON_STOP_LOSS_MARKET);
                    return;
                }
            } else if (m_direction < 0) {
                if (closeExecPrice > m_stopLossPrice) {
                    close(TradeStats::REASON_STOP_LOSS_MARKET);
                    return;
                }
            }
        }

        if (m_takeProfitPrice > 0.0 && !m_limit.orderId.isValid()) {
            o3d::Double closeExecPrice = m_strategy->market()->closeExecPrice(m_direction);

            if (m_direction > 0) {
                if (closeExecPrice > m_takeProfitPrice) {
                    close(TradeStats::REASON_TAKE_PROFIT_MARKET);
                    return;
                }
            } else if (m_direction < 0) {
                if (closeExecPrice < m_takeProfitPrice) {
                    close(TradeStats::REASON_TAKE_PROFIT_MARKET);
                    return;
                }
            }
        }
    }
}

o3d::Bool MarginTrade::isActive() const
{
    if (m_stop.state == STATE_FILLED || m_limit.state == STATE_FILLED) {
        return false;
    }

    return m_entry.state == STATE_PARTIALLY_FILLED || m_entry.state == STATE_FILLED;
}

o3d::Bool MarginTrade::isOpened() const
{
    return m_entry.state == STATE_OPENED;
}

o3d::Bool MarginTrade::isCanceled() const
{
    if (m_entry.state == STATE_REJECTED) {
        return true;
    }

    if ((m_entry.state == STATE_CANCELED || m_entry.state == STATE_DELETED) && m_filledEntryQuantity <= 0.0) {
        return true;
    }

    return false;
}

o3d::Bool MarginTrade::isOpening() const
{
    return m_entry.state == STATE_OPENED || m_entry.state == STATE_PARTIALLY_FILLED;
}

o3d::Bool MarginTrade::isClosing() const
{
    return m_limit.state == STATE_PARTIALLY_FILLED || m_stop.state == STATE_PARTIALLY_FILLED || m_stop.closing;
}

o3d::Bool MarginTrade::isClosed() const
{
    return m_limit.state == STATE_FILLED || m_stop.state == STATE_FILLED;
}

void MarginTrade::orderSignal(const OrderSignal &signal)
{
    if ((signal.orderId.isValid() && signal.orderId == m_entry.orderId) ||
        (signal.refId.isValid() && signal.refId == m_entry.refId)) {

        if (m_entry.orderId.isEmpty()) {
            m_entry.orderId = signal.orderId;
        }

        if (signal.event == signal.OPENED) {
            m_openTimeStamp = signal.executed;

            m_entry.state = STATE_OPENED;
        } else if (signal.event == signal.REJECTED) {
            m_entry.orderId = "";
            m_entry.refId = "";

            m_entry.state = STATE_REJECTED;
        } else if (signal.event == signal.DELETED) {
            m_entry.orderId = "";
            m_entry.refId = "";

        } else if (signal.event == signal.CANCELED) {
            m_entry.orderId = "";
            m_entry.refId = "";

            m_entry.state = STATE_CANCELED;
        } else if (signal.event == signal.UPDATED) {
            // no supported
        } else if (signal.event == signal.TRADED) {
            if (signal.cumulativeFilled > 0.0) {
                m_filledEntryQuantity = signal.cumulativeFilled;
            } else if (signal.filled > 0.0) {
                m_filledEntryQuantity += signal.filled;
            }

            if (signal.avgPrice > 0.0) {
                m_entryPrice = signal.avgPrice;
            } else if (signal.execPrice > 0.0) {
                // @todo avg
                m_entryPrice = signal.execPrice;
            }

            if (m_stats.firstRealizedEntryTimestamp <= 0.0) {
                m_stats.firstRealizedEntryTimestamp = signal.executed;
            }
            m_stats.lastRealizedEntryTimestamp = signal.executed;

            if (m_filledEntryQuantity >= m_orderQuantity || signal.completed) {
                m_entry.state = STATE_FILLED;
            } else {
                m_entry.state = STATE_PARTIALLY_FILLED;
            }
        }
    } else if ((signal.orderId.isValid() && signal.orderId == m_limit.orderId) ||
        (signal.refId.isValid() && signal.refId == m_limit.refId)) {

        if (m_limit.orderId.isEmpty()) {
            m_limit.orderId = signal.orderId;
        }

        if (signal.event == signal.OPENED) {
            if (m_limit.state == STATE_NEW) {
                m_limit.state = STATE_OPENED;
                if (!m_exitTimeStamp) {
                    m_exitTimeStamp = signal.executed;
                }
            }
        } else if (signal.event == signal.REJECTED) {
            m_limit.orderId = "";
            m_limit.refId = "";

            m_limit.state = STATE_REJECTED;
        } else if (signal.event == signal.DELETED) {
            m_limit.orderId = "";
            m_limit.refId = "";

            // @todo qty, avg entry price, timestamp...
            // m_limit.state = STATE_DELETED;
        } else if (signal.event == signal.CANCELED) {
            m_limit.orderId = "";
            m_limit.refId = "";

            m_limit.state = STATE_CANCELED;
        } else if (signal.event == signal.UPDATED) {
            // no supported
        } else if (signal.event == signal.TRADED) {
            if (signal.cumulativeFilled > 0.0) {
                m_filledExitQuantity = signal.cumulativeFilled;
            } else if (signal.filled > 0.0) {
                m_filledExitQuantity += signal.filled;
            }

            if (signal.avgPrice > 0.0) {
                m_exitPrice = signal.avgPrice;
            } else if (signal.execPrice > 0.0) {
                // @todo avg
                m_exitPrice = signal.execPrice;
            }

            if (m_stats.firstRealizedExitTimestamp <= 0.0) {
                m_stats.firstRealizedExitTimestamp = signal.executed;
            }
            m_stats.lastRealizedExitTimestamp = signal.executed;

            if (m_filledEntryQuantity >= m_orderQuantity || signal.completed) {
                m_limit.state = STATE_FILLED;
            } else {
                m_limit.state = STATE_PARTIALLY_FILLED;
            }
        }
    } else if ((signal.orderId.isValid() && signal.orderId == m_stop.orderId) ||
        (signal.refId.isValid() && signal.refId == m_stop.refId)) {

        if (m_stop.orderId.isEmpty()) {
            m_stop.orderId = signal.orderId;
        }

        if (signal.event == signal.OPENED) {
            if (m_stop.state == STATE_NEW) {
                m_stop.state = STATE_OPENED;
                if (!m_exitTimeStamp) {
                    m_exitTimeStamp = signal.executed;
                }
            }
        } else if (signal.event == signal.REJECTED) {
            m_stop.orderId = "";
            m_stop.refId = "";

            m_stop.state = STATE_REJECTED;
        } else if (signal.event == signal.DELETED) {
            m_stop.orderId = "";
            m_stop.refId = "";

            // @todo qty, avg entry price, timestamp...
            // m_stop.state = STATE_DELETED;
        } else if (signal.event == signal.CANCELED) {
            m_stop.orderId = "";
            m_stop.refId = "";

            m_stop.state = STATE_CANCELED;
        } else if (signal.event == signal.UPDATED) {
            // no supported
        } else if (signal.event == signal.TRADED) {
            if (signal.cumulativeFilled > 0.0) {
                m_filledExitQuantity = signal.cumulativeFilled;
            } else if (signal.filled > 0.0) {
                m_filledExitQuantity += signal.filled;
            }

            if (signal.avgPrice > 0.0) {
                m_exitPrice = signal.avgPrice;
            } else if (signal.execPrice > 0.0) {
                // @todo avg
                m_exitPrice = signal.execPrice;
            }

            if (m_stats.firstRealizedExitTimestamp <= 0.0) {
                m_stats.firstRealizedExitTimestamp = signal.executed;
            }
            m_stats.lastRealizedExitTimestamp = signal.executed;

            if (m_filledEntryQuantity >= m_orderQuantity || signal.completed) {
                m_stop.state = STATE_FILLED;
            } else {
                m_stop.state = STATE_PARTIALLY_FILLED;
            }
        }
    }
}

void MarginTrade::positionSignal(const PositionSignal &signal)
{
    if (signal.positionId.isValid() && signal.positionId == m_positionId) {
        if (signal.event == signal.OPENED) {
            // nothing to do
        } else if (signal.event == signal.UPDATED) {
            // nothing to do
        } else if (signal.event == signal.DELETED) {
            // @todo must close the trade
        }
    }
}

o3d::Bool MarginTrade::isTargetOrder(const o3d::String &orderId, const o3d::String &refId) const
{
    if ((orderId.isValid() && orderId == m_entry.orderId) || (refId.isValid() && refId == m_entry.refId)) {
        return true;
    }

    if ((orderId.isValid() && orderId == m_limit.orderId) || (refId.isValid() && refId == m_limit.refId)) {
        return true;
    }

    if ((orderId.isValid() && orderId == m_stop.orderId) || (refId.isValid() && refId == m_stop.refId)) {
        return true;
    }

    return false;
}

o3d::Bool MarginTrade::isTargetPosition(const o3d::String &positionId, const o3d::String &refId) const
{
    if (positionId.isValid() && positionId == m_positionId) {
        return true;
    }

    return false;
}

o3d::String MarginTrade::formatToStr() const
{
    o3d::String result;

    // @todo format

    return result;
}

o3d::String MarginTrade::stateToStr() const
{
    if (m_entry.state == STATE_NEW) {
        return "new";
    } else if (m_entry.state == STATE_OPENED) {
        return "opened";
    } else if (m_entry.state == STATE_REJECTED) {
        return "rejected";
    } else if (m_entry.state == STATE_REJECTED && m_filledEntryQuantity > m_filledExitQuantity) {
        return "problem";
    } else if (m_filledEntryQuantity < m_filledExitQuantity && (m_entry.state == STATE_PARTIALLY_FILLED || m_entry.state == STATE_OPENED)) {
        return "filling";
    } else if (m_filledEntryQuantity > 0.0 && m_filledExitQuantity < m_filledEntryQuantity && m_stop.state != STATE_FILLED && m_limit.state != STATE_FILLED) {
        return "closing";
    } else if (m_filledEntryQuantity > 0.0 && m_filledExitQuantity >= m_filledEntryQuantity) {
        return "closed";
    } else if (m_filledEntryQuantity >= m_orderQuantity) {
        return "filled";
    } else {
        return "waiting";
    }
}

void MarginTrade::dumps(o3d::Variadic &trades, Market *market) const
{
    // @todo
}

void MarginTrade::loads(const o3d::Variadic &trade)
{
    // @todo
}
