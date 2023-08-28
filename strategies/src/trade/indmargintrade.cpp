/**
 * @brief SiiS strategy indivisible position, margin trade model specialization.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/trade/indmargintrade.h"
#include "siis/utils/common.h"
#include "siis/connector/traderproxy.h"
#include "siis/strategy.h"
#include "siis/market.h"

using namespace siis;

IndMarginTrade::IndMarginTrade(TraderProxy *proxy) :
    Trade(proxy, Trade::TYPE_IND_MARGIN, -1.0),
    m_entry(),
    m_stop(),
    m_limit()
{

}

IndMarginTrade::~IndMarginTrade()
{

}

void IndMarginTrade::init(o3d::Double timeframe)
{
    Trade::init(timeframe);

    m_entry.reset();
    m_stop.reset();
    m_limit.reset();

    m_positionId = "";
}

void IndMarginTrade::open(Strategy *strategy,
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
    entryOrder->orderType = orderType;
    entryOrder->orderPrice = orderPrice;
    entryOrder->setMargin();

    m_entry.refId = entryOrder->refId;
    m_stats.entryOrderType = entryOrder->orderType;

    o3d::Int32 ret = traderProxy()->createOrder(entryOrder);
    if (ret == Order::RET_OK) {
    } else {
        m_entry.state = STATE_REJECTED;
        m_stats.entryOrderType = Order::ORDER_UNDEFINED;
    }
}

void IndMarginTrade::remove()
{
    if (m_stop.closing) {
        return;
    }

    if (m_entry.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_entry.orderId);
        if (ret == Order::RET_OK) {
            m_entry.clear();
            m_entry.state = STATE_CANCELED;
        } else {

        }
    }

    if (m_stop.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_stop.orderId);
        if (ret == Order::RET_OK) {
            m_stop.clear();
            m_stop.state = STATE_CANCELED;
        } else {

        }
    }

    if (m_limit.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_limit.orderId);
        if (ret == Order::RET_OK) {
            m_limit.clear();
            m_limit.state = STATE_CANCELED;
        } else {

        }
    }
}

void IndMarginTrade::cancelOpen()
{
    if (isOpened()) {
        if (m_entry.hasOrder()) {
            o3d::Int32 ret = traderProxy()->cancelOrder(m_entry.orderId);
            if (ret == Order::RET_OK) {
                m_entry.clear();
                m_entry.state = STATE_CANCELED;
            } else {

            }
        }
    }
}

void IndMarginTrade::cancelClose()
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
            m_stop.clear();
            m_stop.state = STATE_CANCELED;
        } else {

        }
    }

    if (m_limit.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_limit.orderId);
        if (ret == Order::RET_OK) {
            m_limit.clear();
            m_limit.state = STATE_CANCELED;
        } else {

        }
    }
}

void IndMarginTrade::modifyTakeProfit(o3d::Double price, ModifierType mod)
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
            m_limit.clear();
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
            limitOrder->setMargin();
            limitOrder->setReduceOnly();

            m_limit.refId = limitOrder->refId;
            m_limit.orderedQty = limitOrder->orderQuantity;

            m_stats.stopOrderType = limitOrder->orderType;

            o3d::Int32 ret = traderProxy()->createOrder(limitOrder);            
            if (ret == Order::RET_OK) {
            } else {
                m_stats.stopOrderType = Order::ORDER_UNDEFINED;
                m_limit.clear();
            }
        }
    }

    m_takeProfitPrice = price;
}

void IndMarginTrade::modifyStopLoss(o3d::Double price, ModifierType mod)
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
            m_stop.clear();
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
            stopOrder->setMargin();
            stopOrder->setReduceOnly();

            m_stop.refId = stopOrder->refId;
            m_stop.orderedQty = stopOrder->orderQuantity;

            m_stats.stopOrderType = stopOrder->orderType;

            o3d::Int32 ret = traderProxy()->createOrder(stopOrder);
            if (ret == Order::RET_OK) {
            } else {
                m_stats.stopOrderType = Order::ORDER_UNDEFINED;
                m_stop.clear();
            }
        }
    }

    m_stopLossPrice = price;
}

void IndMarginTrade::close(TradeStats::ExitReason reason)
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
            m_stop.clear();
            m_stop.state = STATE_CANCELED;
        } else {

        }
    }

    if (m_limit.hasOrder()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_limit.orderId);
        if (ret == Order::RET_OK) {
            m_limit.clear();
            m_limit.state = STATE_CANCELED;
        } else {

        }
    }

    m_stats.exitReason = reason;

    Order *stopOrder = traderProxy()->newOrder(m_strategy);
    stopOrder->direction = -m_direction;
    stopOrder->orderQuantity = remaining_qty;
    stopOrder->orderType = Order::ORDER_MARKET;
    stopOrder->setMargin();
    stopOrder->setReduceOnly();

    m_stop.refId = stopOrder->refId;
    m_stop.orderedQty = stopOrder->orderQuantity;
    m_stop.closing = true;

    m_stats.stopOrderType = stopOrder->orderType;

    o3d::Int32 ret = traderProxy()->createOrder(stopOrder);
    if (ret == Order::RET_OK) {
    } else {
        m_stats.stopOrderType = Order::ORDER_UNDEFINED;
        m_stop.clear();
    }
}

o3d::Bool IndMarginTrade::hasStopOrder() const
{
    return m_stop.hasOrder();
}

o3d::Bool IndMarginTrade::hasLimitOrder() const
{
    return m_limit.hasOrder();
}

o3d::Bool IndMarginTrade::supportBothOrder() const
{
    return true;
}

void IndMarginTrade::process(o3d::Double timestamp)
{
    if (isActive()) {
        m_strategy->updateTrade(this);

        // @todo could have a stop directive at a candle close or ... and so need to adjust here
        if (m_stopLossPrice > 0.0 && !m_stop.orderId.isValid()) {
            o3d::Double closeExecPrice = m_strategy->market()->closeExecPrice(m_direction);

            if (m_direction > 0) {
                if (closeExecPrice <= m_stopLossPrice) {
                    close(TradeStats::REASON_STOP_LOSS_MARKET);
                    return;
                }
            } else if (m_direction < 0) {
                if (closeExecPrice >= m_stopLossPrice) {
                    close(TradeStats::REASON_STOP_LOSS_MARKET);
                    return;
                }
            }
        }

        if (m_takeProfitPrice > 0.0 && !m_limit.orderId.isValid()) {
            o3d::Double closeExecPrice = m_strategy->market()->closeExecPrice(m_direction);

            if (m_direction > 0) {
                if (closeExecPrice >= m_takeProfitPrice) {
                    close(TradeStats::REASON_TAKE_PROFIT_MARKET);
                    return;
                }
            } else if (m_direction < 0) {
                if (closeExecPrice <= m_takeProfitPrice) {
                    close(TradeStats::REASON_TAKE_PROFIT_MARKET);
                    return;
                }
            }
        }
    }
}

o3d::Bool IndMarginTrade::isActive() const
{
    if (m_stop.state == STATE_FILLED || m_limit.state == STATE_FILLED) {
        return false;
    }

    return m_entry.state == STATE_PARTIALLY_FILLED || m_entry.state == STATE_FILLED;
}

o3d::Bool IndMarginTrade::isOpened() const
{
    return m_entry.state == STATE_OPENED;
}

o3d::Bool IndMarginTrade::isCanceled() const
{
    if (m_entry.state == STATE_REJECTED) {
        return true;
    }

    if ((m_entry.state == STATE_CANCELED || m_entry.state == STATE_DELETED) && m_filledEntryQuantity <= 0.0) {
        return true;
    }

    return false;
}

o3d::Bool IndMarginTrade::isOpening() const
{
    return m_entry.state == STATE_OPENED || m_entry.state == STATE_PARTIALLY_FILLED;
}

o3d::Bool IndMarginTrade::isClosing() const
{
    return m_limit.state == STATE_PARTIALLY_FILLED || m_stop.state == STATE_PARTIALLY_FILLED || m_stop.closing;
}

o3d::Bool IndMarginTrade::isClosed() const
{
    return m_limit.state == STATE_FILLED || m_stop.state == STATE_FILLED;
}

void IndMarginTrade::orderSignal(const OrderSignal &signal)
{
    //
    // entry
    //

    if ((signal.orderId.isValid() && signal.orderId == m_entry.orderId) ||
        (signal.refId.isValid() && signal.refId == m_entry.refId)) {

        if (m_entry.orderId.isEmpty()) {
            m_entry.orderId = signal.orderId;
        }

        if (signal.event == signal.OPENED) {
            if (m_entry.state == STATE_NEW) {
                m_entry.state = STATE_OPENED;
            }
            if (!m_openTimeStamp && signal.execPrice > 0.0) {
                m_openTimeStamp = signal.executed;
            }
        } else if (signal.event == signal.REJECTED) {
            m_entry.clear();
            m_entry.state = STATE_REJECTED;

        } else if (signal.event == signal.DELETED) {
            m_entry.clear();

        } else if (signal.event == signal.CANCELED) {
            m_entry.clear();
            m_entry.state = STATE_CANCELED;

        } else if (signal.event == signal.UPDATED) {
            // not supported

        } else if (signal.event == signal.TRADED) {
            // computed relative qty (because there is a signle entry order no need of a relative variable)
            o3d::Double filled = 0.0;

            if (signal.cumulativeFilled > 0.0) {
                filled = signal.cumulativeFilled - m_filledEntryQuantity;
            } else if (signal.filled > 0.0) {
                filled = signal.filled;
            }

            if (signal.avgPrice > 0.0) {
                // in that case we have avg-price already computed
                m_entryPrice = signal.avgPrice;
            } else if (signal.execPrice > 0.0) {
                // compute the average entry price
                m_entryPrice = strategy()->market()->adjustPrice((m_entryPrice * m_filledEntryQuantity +
                    signal.execPrice * filled) / (m_filledEntryQuantity + filled));
            } else {
                m_entryPrice = m_orderPrice;
            }

            // cumulative filled entry qty
            if (signal.cumulativeFilled > 0.0) {
                m_filledEntryQuantity = signal.cumulativeFilled;
            } else if (filled > 0.0) {
                m_filledEntryQuantity = strategy()->market()->adjustQty(m_filledEntryQuantity + filled);
            }

            if (filled > 0.0) {
                // probably need to update exit orders
                m_dirty = true;

                // update notional value of the position
                m_stats.notionalValue = strategy()->market()->effectiveCost(m_filledEntryQuantity, m_entryPrice);
            }

            // realized fees : in cumulated or compute from filled quantity and trade execution
            if (signal.cumulativeCommissionAmount != signal.FEE_UNDEFINED &&
                signal.cumulativeCommissionAmount != 0.0 &&
                signal.cumulativeCommissionAmount != m_stats.entryFees) {

                m_stats.entryFees = signal.cumulativeCommissionAmount;

            } else if (signal.commissionAmount != signal.FEE_UNDEFINED && signal.commissionAmount != 0.0) {
                m_stats.entryFees += signal.commissionAmount;

            } else if (filled > 0.0) {
                // update entry fees
                o3d::Bool maker = signal.maker > 0;

                if (signal.maker < 0) {
                    // no information, try to detect it
                    if (m_stats.entryOrderType == Order::ORDER_LIMIT) {
                        // @todo only if execution price is equal or better, then order price (depends on direction)
                        // or if post-only is defined
                        maker = true;
                    } else {
                        maker = false;
                    }
                }

                // recompute entry-fees proportionate to notional-value
                m_stats.entryFees = m_stats.notionalValue * (maker ? strategy()->market()->makerFee().rate :
                                                                 strategy()->market()->takerFee().rate);

                // plus the initial commission fee
                m_stats.entryFees += maker ? strategy()->market()->makerFee().commission :
                                         strategy()->market()->takerFee().commission;
            }

            // state
            if (m_filledEntryQuantity >= m_orderQuantity || signal.completed) {
                m_entry.state = STATE_FILLED;

                // bitmex does not send ORDER_DELETED signal, cleanup here
                m_entry.clear();
            } else {
                m_entry.state = STATE_PARTIALLY_FILLED;
            }

            // stats
            if (m_stats.firstRealizedEntryTimestamp <= 0.0) {
                m_stats.firstRealizedEntryTimestamp = signal.executed;
            }
            m_stats.lastRealizedEntryTimestamp = signal.executed;
        }

    //
    // limit
    //

    } else if ((signal.orderId.isValid() && signal.orderId == m_limit.orderId) ||
        (signal.refId.isValid() && signal.refId == m_limit.refId)) {

        if (m_limit.orderId.isEmpty()) {
            m_limit.orderId = signal.orderId;
        }

        if (signal.event == signal.OPENED) {
            if (m_limit.state == STATE_NEW) {
                m_limit.state = STATE_OPENED;
            }
            if (!m_exitTimeStamp && signal.executed > 0.0) {
                m_exitTimeStamp = signal.executed;
            }
        } else if (signal.event == signal.REJECTED) {
            m_limit.clear();
            m_limit.state = STATE_REJECTED;

        } else if (signal.event == signal.DELETED) {
            m_limit.clear();
            // @todo qty, avg entry price, timestamp...
            // m_limit.state = STATE_DELETED;

        } else if (signal.event == signal.CANCELED) {
            m_limit.clear();
            m_limit.state = STATE_CANCELED;

        } else if (signal.event == signal.UPDATED) {
            // not supported

        } else if (signal.event == signal.TRADED) {
            // qty
            o3d::Double prevLimitOrderExec = m_limit.execQty;
            o3d::Double filled = _updateExitQty(signal, m_limit.execQty);

            // cumulative filled exit qty, update trade qty and order related qty
            if (filled > 0.0) {
                m_limit.execQty = strategy()->market()->adjustQty(m_limit.execQty + filled);
            }

            // fees/commissions
            o3d::Double filledCommission = _updateExitFeesAndQty(signal, m_limit.fees, m_limit.execQty,
                                                                 prevLimitOrderExec, m_stats.takeProfitOrderType);

            if (filledCommission != 0.0) {
                m_limit.fees += filledCommission;
            }

            // state
            m_limit.state = _updateExitState(signal);

            // order relative executed qty reached ordered qty or completed : reset limit order state
            if (m_limit.execQty >= m_limit.orderedQty || signal.completed) {
                m_limit.clear();
            }

            // stats
            _updateExitStats(signal);
        }

    //
    // stop
    //

    } else if ((signal.orderId.isValid() && signal.orderId == m_stop.orderId) ||
        (signal.refId.isValid() && signal.refId == m_stop.refId)) {

        if (m_stop.orderId.isEmpty()) {
            m_stop.orderId = signal.orderId;
        }

        if (signal.event == signal.OPENED) {
            if (m_stop.state == STATE_NEW) {
                m_stop.state = STATE_OPENED;
            }
            if (!m_exitTimeStamp && signal.executed > 0.0) {
                m_exitTimeStamp = signal.executed;
            }
        } else if (signal.event == signal.REJECTED) {
            m_stop.clear();
            m_stop.state = STATE_REJECTED;

        } else if (signal.event == signal.DELETED) {
            m_stop.clear();
            // @todo qty, avg entry price, timestamp...
            // m_stop.state = STATE_DELETED;

        } else if (signal.event == signal.CANCELED) {
            m_stop.clear();
            m_stop.state = STATE_CANCELED;

        } else if (signal.event == signal.UPDATED) {
            // not supported

        } else if (signal.event == signal.TRADED) {
            // qty
            o3d::Double prevStopOrderExec = m_stop.execQty;
            o3d::Double filled = _updateExitQty(signal, m_stop.execQty);

            // cumulative filled exit qty, update trade qty and order related qty
            if (filled > 0.0) {
                m_stop.execQty = strategy()->market()->adjustQty(m_stop.execQty + filled);
            }

            // fees/commissions
            o3d::Double filledCommission = _updateExitFeesAndQty(signal, m_stop.fees, m_stop.execQty,
                                                                 prevStopOrderExec, m_stats.stopOrderType);

            if (filledCommission != 0.0) {
                m_stop.fees += filledCommission;
            }

            // state
            m_stop.state = _updateExitState(signal);

            // order relative executed qty reached ordered qty or completed : reset limit order state
            if (m_stop.execQty >= m_stop.orderedQty || signal.completed) {
                m_stop.clear();
            }

            // stats
            _updateExitStats(signal);
        }
    }
}

void IndMarginTrade::positionSignal(const PositionSignal &signal)
{
    if (signal.positionId.isValid() && signal.positionId == m_positionId) {
        if (signal.event == signal.OPENED) {
            // nothing to do
        } else if (signal.event == signal.UPDATED) {
            // nothing to do
        } else if (signal.event == signal.DELETED) {
            // @todo must close the trade, compute profitLoss
        }
    }
}

o3d::Bool IndMarginTrade::isTargetOrder(const o3d::String &orderId, const o3d::String &refId) const
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

o3d::Bool IndMarginTrade::isTargetPosition(const o3d::String &positionId, const o3d::String &refId) const
{
    if (positionId.isValid() && positionId == m_positionId) {
        return true;
    }

    return false;
}

o3d::String IndMarginTrade::formatToStr() const
{
    o3d::String result;

    // @todo format

    return result;
}

o3d::String IndMarginTrade::stateToStr() const
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

void IndMarginTrade::updateStats(o3d::Double lastPrice, o3d::Double timestamp)
{
    Trade::updateStats(lastPrice, timestamp);

    if (isActive()) {
        // @todo
//        last_price = instrument.close_exec_price(self.direction)
//        if last_price <= 0:
//            return

//        // non realized quantity
//        nrq = self.e - self.x

//        u_pnl = instrument.compute_pnl(nrq, self.dir, self.aep, last_price)
//        r_pnl = instrument.compute_pnl(self.x, self.dir, self.aep, self.axp)

//        // including fees and realized profit and loss
//        self._stats['unrealized-profit-loss'] = instrument.adjust_settlement(
//            u_pnl + r_pnl - self._stats['entry-fees'] - self._stats['exit-fees'] - self._stats['margin-fees'])
    }
}

void IndMarginTrade::dumps(o3d::Variadic &trades, Market *market) const
{
    // @todo
}

void IndMarginTrade::loads(const o3d::Variadic &trade)
{
    // @todo
}

void IndMarginTrade::updateRealizedPnl()
{
    // over exit qty because it as a ratio proportional to entry and not exit qty
    m_profitLossRate = m_direction * (m_exitPrice * m_filledExitQuantity - m_entryPrice * m_filledExitQuantity) /
                       (m_entryPrice * m_filledEntryQuantity);
}

o3d::Double IndMarginTrade::_updateExitQty(const OrderSignal &signal, o3d::Double cumulatedQty)
{
    o3d::Double _filled = 0.0;

    // either we have 'filled' component (partial qty) or completed or both
    if (signal.cumulativeFilled > 0.0 && signal.cumulativeFilled > cumulatedQty) {
        // compute filled qty since last signal
        _filled = signal.cumulativeFilled - cumulatedQty;
    } else if (signal.filled > 0.0) {
        // relative data field
        _filled = signal.filled;
    }

    if (signal.avgPrice > 0.0) {
        // recompute profit-loss
        // m_profitLossRate = m_direction * (signal.avgPrice - m_entryPrice) / m_entryPrice;

        // in that case we have avg-price already computed but not sufficient in case of multiple orders for exit
        // m_exitPrice = signal.avgPrice;
        m_exitPrice = strategy()->market()->adjustPrice(
                          m_exitPrice * m_filledExitQuantity) + (signal.avgPrice * _filled) / (
                          m_filledExitQuantity + _filled);

    } else if (signal.execPrice > 0.0) {
        // increase/decrease profit/loss (over entry executed quantity)
        // m_profitLossRate += m_direction * (signal.execPrice * _filled - m_entryPrice * _filled) / (
        //                         m_entryPrice * m_filledEntryQuantity);

        // compute the average exit price
        m_exitPrice = strategy()->market()->adjustPrice(
                          m_exitPrice * m_filledExitQuantity) + (signal.execPrice * _filled) / (
                          m_filledExitQuantity + _filled);
    }

    if (_filled > 0.0) {
        // update realized exit qty
        m_filledExitQuantity = strategy()->market()->adjustQty(m_filledExitQuantity + _filled);

        // and realized PNL
        if (m_entryPrice > 0.0 && m_filledEntryQuantity > 0.0) {
            m_profitLossRate = m_direction * (m_exitPrice * m_filledExitQuantity -
                m_entryPrice * m_filledExitQuantity) / (m_entryPrice * m_filledEntryQuantity);
        }
    }

    return _filled;
}

o3d::Double IndMarginTrade::_updateExitFeesAndQty(const OrderSignal &signal,
                                                  o3d::Double cumulatedFees, o3d::Double cumulatedQty,
                                                  o3d::Double prevOrderExec, Order::OrderType orderType)
{
    // filled fees/commissions
    o3d::Double _filledCommission = 0.0;

    if (signal.cumulativeCommissionAmount == signal.FEE_UNDEFINED && signal.commissionAmount == signal.FEE_UNDEFINED) {
        // no value but have an order execution then compute locally
        if (cumulatedQty > prevOrderExec) {
            // compute from instrument details
            o3d::Bool maker = signal.maker > 0;

            if (signal.maker < 0) {
                // no information, try to detect it
                if (m_stats.entryOrderType == Order::ORDER_LIMIT) {
                    // @todo only if execution price is equal or better, then order price (depends on direction)
                    // or if post-only is defined
                    maker = true;
                } else {
                    maker = false;
                }
            }

            // proportionate to filled qty m_stats.notionalValue is proportionate to m_filledEntryQuantity)
            o3d::Double _filledQtyRate = m_filledEntryQuantity > 0 ? (cumulatedQty - prevOrderExec) /
                                                                        m_filledEntryQuantity : 0;

            _filledCommission = (_filledQtyRate * m_stats.notionalValue) * (m_exitPrice / m_entryPrice) * (maker ?
                strategy()->market()->makerFee().rate : strategy()->market()->takerFee().rate);

            if (prevOrderExec == 0.0) {
                // for initial fill add the commission fee
                _filledCommission += maker ? strategy()->market()->makerFee().commission :
                                         strategy()->market()->takerFee().commission;
            }
        }
    } else if (signal.cumulativeCommissionAmount != signal.FEE_UNDEFINED && signal.cumulativeCommissionAmount != 0.0 &&
               signal.cumulativeCommissionAmount != cumulatedFees) {
        // compute filled commission amount since last signal
        _filledCommission = signal.cumulativeCommissionAmount - cumulatedFees;

    } else if (signal.commissionAmount != signal.FEE_UNDEFINED && signal.commissionAmount != 0.0) {
        // relative data field
        _filledCommission = signal.commissionAmount;
    }

    // realized fees : in cumulated amount or computed from filled quantity and trade execution
    if (_filledCommission != 0.0) {
        m_stats.exitFees += _filledCommission;
    }

    return _filledCommission;
}

Trade::State IndMarginTrade::_updateExitState(const OrderSignal &signal)
{
    if (m_entry.state == STATE_FILLED) {
        if (m_filledExitQuantity > m_filledEntryQuantity || signal.completed) {
            // entry fully-filled : exit fully-filled or exit quantity reach entry quantity
            return STATE_FILLED;
        } else {
            // entry fully-filled : exit quantity not reached entry quantity
            return STATE_PARTIALLY_FILLED;
        }
    } else {
        if (m_entry.orderId.isValid() && m_filledEntryQuantity < m_orderQuantity) {
            // entry order still exists and entry quantity not reached order entry quantity
            return STATE_PARTIALLY_FILLED;
        } else {
            // or there is no longer entry order then we have fully filled the exit
            return STATE_FILLED;
        }
    }
}

void IndMarginTrade::_updateExitStats(const OrderSignal &signal)
{
    if (m_stats.firstRealizedExitTimestamp <= 0.0) {
        m_stats.firstRealizedExitTimestamp = signal.executed;
    }
    m_stats.lastRealizedExitTimestamp = signal.executed;
}
