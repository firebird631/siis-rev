/**
 * @brief SiiS strategy position trade model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-28
 */

#include "siis/trade/positiontrade.h"
#include "siis/utils/common.h"
#include "siis/connector/traderproxy.h"
#include "siis/strategy.h"
#include "siis/market.h"

using namespace siis;

PositionTrade::PositionTrade(TraderProxy *proxy) :
    Trade(proxy, Trade::TYPE_POSITION, -1.0),
    m_entryState(STATE_UNDEFINED),
    m_exitState(STATE_UNDEFINED),
    m_closing(false),
    m_positionLimitPrice(0.0),
    m_positionStopPrice(0.0),
    m_positionQuantity(0.0)
{

}

PositionTrade::~PositionTrade()
{

}

void PositionTrade::init(o3d::Double timeframe)
{
    Trade::init(timeframe);

    m_entryState = STATE_UNDEFINED;
    m_exitState = STATE_UNDEFINED;

    m_positionLimitPrice = 0.0;
    m_positionStopPrice = 0.0;
    m_positionQuantity = 0.0;

    m_closing = false;

    m_entryOrderId = "";
    m_entryRefId = "";
    m_positionId = "";
}

void PositionTrade::open(
        Strategy *strategy,
        o3d::Int32 direction,
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

    if (orderPrice <= 0.0) {
        entryOrder->orderType = Order::ORDER_MARKET;
    } else {
        entryOrder->orderType = Order::ORDER_LIMIT;
        entryOrder->orderPrice = orderPrice;
    }

    m_entryRefId = entryOrder->refId;

    o3d::Int32 ret = traderProxy()->createOrder(entryOrder);
    if (ret == Order::RET_OK) {
    } else {
        m_entryState = STATE_REJECTED;
    }
}

void PositionTrade::remove()
{
    if (m_closing) {
        return;
    }

    if (m_entryOrderId.isValid() || m_entryRefId.isValid()) {
        o3d::Int32 ret = traderProxy()->cancelOrder(m_entryOrderId);
        if (ret == Order::RET_OK) {
            m_entryOrderId = "";
            m_entryRefId = "";

            if (m_filledEntryQuantity <= 0) {
                m_entryState = STATE_CANCELED;
            } else {
                m_entryState = STATE_FILLED;
            }
        } else {
            // @todo
        }
    }
}

void PositionTrade::cancelOpen()
{
    if (isOpened()) {
        if (m_entryOrderId.isValid()) {
            o3d::Int32 ret = traderProxy()->cancelOrder(m_entryOrderId);
            if (ret == Order::RET_OK) {
                m_entryOrderId = "";
                m_entryRefId = "";

                m_entryState = STATE_CANCELED;
            } else {
                // @todo
            }
        }
    }
}

void PositionTrade::cancelClose()
{
    // nothing to do
}

void PositionTrade::modifyTakeProfit(o3d::Double price, ModifierType mod)
{
    if (m_closing) {
        return;
    }

    if (m_positionId.isValid()) {
        if (mod == MOD_PREVIOUS && m_positionLimitPrice > 0.0) {
            mod = MOD_DISTANT;
        }

        if (mod == MOD_DISTANT) {
            // only update the limit price (let to the previous stop)
            o3d::Int32 ret = traderProxy()->modifyPosition(m_positionId, m_positionStopPrice, price);
            if (ret == Order::RET_OK) {
                m_positionLimitPrice = price;
            } else {
                // @todo
            }
        }
    }

    m_takeProfitPrice = price;
}

void PositionTrade::modifyStopLoss(o3d::Double price, ModifierType mod)
{
    if (m_closing) {
        return;
    }

    if (m_positionId.isValid()) {
        if (mod == MOD_PREVIOUS && m_positionStopPrice > 0.0) {
            mod = MOD_DISTANT;
        }

        if (mod == MOD_DISTANT) {
            // only update the stop price (let to the previous limit)
            o3d::Int32 ret = traderProxy()->modifyPosition(m_positionId, price, m_positionLimitPrice);
            if (ret == Order::RET_OK) {
                m_positionStopPrice = price;
            } else {
                // @todo
            }
        }
    }

    m_stopLossPrice = price;
}

void PositionTrade::close(TradeStats::ExitReason reason)
{
    if (m_closing) {
        return;
    }

    if (m_entryOrderId.isValid()) {
        // cancel the remaining buy order
        o3d::Int32 ret = traderProxy()->cancelOrder(m_entryOrderId);
        if (ret == Order::RET_OK) {
            m_entryRefId = "";
            m_entryOrderId = "";

            m_entryState = STATE_CANCELED;
        } else {
            // @todo
        }
    }

    m_stats.exitReason = reason;

    if (m_positionId.isValid()) {
        m_closing = true;

        o3d::Int32 ret = traderProxy()->closePosition(m_positionId, m_direction, m_positionQuantity, true, 0.0);
        if (ret == Order::RET_OK) {
        } else {
            m_closing = false;
        }
    }
}

void PositionTrade::process(o3d::Double timestamp)
{
    if (isActive()) {
        m_strategy->updateTrade(this);

        if (m_stopLossPrice > 0.0 && !m_positionStopPrice) {
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

        if (m_takeProfitPrice > 0.0 && !m_positionLimitPrice) {
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

o3d::Bool PositionTrade::isActive() const
{
    if (m_exitState == STATE_FILLED) {
        return false;
    }

    return m_entryState == STATE_PARTIALLY_FILLED || m_entryState == STATE_FILLED;
}

o3d::Bool PositionTrade::isOpened() const
{
    return m_entryState == STATE_OPENED;
}

o3d::Bool PositionTrade::isCanceled() const
{
    if (m_entryState == STATE_REJECTED) {
        return true;
    }

    if ((m_entryState == STATE_CANCELED || m_entryState == STATE_DELETED) && m_filledEntryQuantity <= 0.0) {
        return true;
    }

    return false;
}

o3d::Bool PositionTrade::isOpening() const
{
    return m_entryState == STATE_OPENED || m_entryState == STATE_PARTIALLY_FILLED;
}

o3d::Bool PositionTrade::isClosing() const
{
    return m_exitState == STATE_PARTIALLY_FILLED || m_closing;
}

o3d::Bool PositionTrade::isClosed() const
{
    return m_exitState == STATE_FILLED;
}

void PositionTrade::orderSignal(const OrderSignal &signal)
{
    if ((signal.orderId.isValid() && signal.orderId == m_entryOrderId) ||
        (signal.refId.isValid() && signal.refId == m_entryRefId)) {

        if (signal.event == signal.OPENED) {
            if (m_entryOrderId.isEmpty()) {
                m_entryOrderId = signal.orderId;
            }

            if (m_positionId.isEmpty() && signal.positionId.isValid()) {
                m_positionId = signal.positionId;
            }

            if (m_openTimeStamp != signal.TIMESTAMP_UNDEFINED) {
                m_openTimeStamp = signal.created;
            }

            if (signal.stopPrice != signal.PRICE_UNDEFINED) {
                m_stopLossPrice = signal.stopPrice;
            }

            if (signal.limitPrice != signal.PRICE_UNDEFINED) {
                m_takeProfitPrice = signal.limitPrice;
            }

            // in case it occurs after position open signal
            if (m_filledEntryQuantity != signal.QUANTITY_UNDEFINED) {
                m_entryState = STATE_OPENED;
            }
        } else if (signal.event == signal.DELETED) {
            // create order is no longer active
            if (signal.orderId == m_entryOrderId) {
                m_entryRefId = "";
                m_entryOrderId = "";

                if (m_positionId.isEmpty()) {
                    m_entryState = STATE_DELETED;
                }
            }
        } else if (signal.event == signal.CANCELED) {
            // create order is no longer active
            if (signal.orderId == m_entryOrderId) {
                m_entryRefId = "";
                m_entryOrderId = "";

                if (m_positionId.isEmpty()) {
                    m_entryState = STATE_CANCELED;
                }
            }
        } else if (signal.event == signal.UPDATED) {
            // order price/qty modified, cannot really be used because the strategy might
            // cancel the trade or create another one.
            // for the qty we could have a remaining_qty member, then comparing
            // others updates are done at position signals
        } else if (signal.event == signal.TRADED) {
            // done at position signal
        }
    }
}

void PositionTrade::updateRealizedPnl()
{
    if (m_entryPrice > 0.0 && m_filledExitQuantity > 0.0) {

        m_profitLossRate = m_direction * ((m_exitPrice * m_filledExitQuantity) - (m_entryPrice * m_filledExitQuantity)) /
                           (m_entryPrice * m_filledExitQuantity);
    }
}

void PositionTrade::positionSignal(const PositionSignal &signal)
{
    if ((signal.positionId.isValid() && m_positionId == signal.positionId) ||
        (signal.orderRefId.isValid() && m_entryRefId == signal.orderRefId)) {

        if (signal.event == signal.OPENED) {
            m_entryState = STATE_OPENED;

            if (m_positionId.isEmpty() && signal.positionId.isValid()) {
                m_positionId = signal.positionId;
            }

            // init created timestamp when create order open if not defined at open
            if (m_openTimeStamp != signal.TIMESTAMP_UNDEFINED) {
                m_openTimeStamp = signal.created;
            }

            if (signal.limitPrice != signal.PRICE_UNDEFINED) {
                m_positionLimitPrice = signal.limitPrice;
            }

            if (signal.stopPrice != signal.PRICE_UNDEFINED) {
                m_positionStopPrice = signal.stopPrice;
            }

            if (m_exitTimeStamp <= 0.0 && (
                    signal.limitPrice != signal.PRICE_UNDEFINED || signal.stopPrice != signal.PRICE_UNDEFINED)) {
                // determine exit order timestamp only if stop or limit defined at open
                m_exitTimeStamp = signal.created;
            }

            // current quantity
            o3d::Double lastQty = 0.0;

            if (signal.cumulativeFilled != signal.QUANTITY_UNDEFINED) {
                lastQty = signal.cumulativeFilled;
            } else if (signal.filled != signal.QUANTITY_UNDEFINED) {
                lastQty = signal.filled;
            }

            if (lastQty > 0.0) {
                // increase entry qty
                if (m_stats.firstRealizedEntryTimestamp <= 0.0 && signal.created != signal.TIMESTAMP_UNDEFINED) {
                    m_stats.firstRealizedEntryTimestamp = signal.created;
                }

                if (signal.created != signal.TIMESTAMP_UNDEFINED) {
                    m_stats.lastRealizedEntryTimestamp = signal.created;
                }

                // filled entry quantity from the diff with the previous one (position quantity might be 0)
                m_filledEntryQuantity += lastQty - m_positionQuantity;

                if (m_filledEntryQuantity >= m_orderQuantity) {
                    m_entryState = STATE_FILLED;

                    // entry cannot longer be canceled once fully filled
                    m_entryOrderId = "";
                    m_entryRefId = "";
                } else {
                    m_entryState = STATE_PARTIALLY_FILLED;
                }

                // keep for close and for delta computation on update
                m_positionQuantity = lastQty;

                // average entry price at open (simply set)
                if (signal.avgPrice != signal.PRICE_UNDEFINED) {
                    m_entryPrice = signal.avgPrice;
                } else if (signal.execPrice != signal.PRICE_UNDEFINED) {
                    m_entryPrice = signal.execPrice;
                }

                updateRealizedPnl();
            } else {
                // in case of a limit order
                m_entryState = STATE_OPENED;
            }
        } else if (signal.event == signal.UPDATED) {
            // update stop_loss/take_profit
            if (signal.limitPrice != signal.PRICE_UNDEFINED) {
                m_positionLimitPrice = signal.limitPrice;
            }

            if (signal.stopPrice != signal.PRICE_UNDEFINED) {
                m_positionStopPrice = signal.stopPrice;
            }

            // current quantity
            o3d::Double lastQty = m_positionQuantity;

            if (signal.cumulativeFilled != signal.QUANTITY_UNDEFINED) {
                lastQty = signal.cumulativeFilled;
            } else if (signal.filled != signal.QUANTITY_UNDEFINED) {
                lastQty = signal.filled + m_positionQuantity;
            } else if (signal.quantity != signal.QUANTITY_UNDEFINED) {
                lastQty = signal.quantity;
            }

            if (lastQty != m_positionQuantity) {
                if (lastQty < m_positionQuantity) {
                    // decrease mean exit
                    if (m_stats.firstRealizedExitTimestamp <= 0.0 && signal.updated != signal.TIMESTAMP_UNDEFINED) {
                        m_stats.firstRealizedExitTimestamp = signal.updated;
                    }

                    if (signal.updated != signal.TIMESTAMP_UNDEFINED) {
                        m_stats.lastRealizedExitTimestamp = signal.updated;
                    }

                    // filled entry quantity from the diff with the previous one
                    o3d::Double prevFilledExitQuantity = m_filledExitQuantity;
                    m_filledExitQuantity += m_positionQuantity - lastQty;

                    if (m_filledExitQuantity >= m_filledEntryQuantity) {
                        m_exitState = STATE_FILLED;
                    } else {
                        m_exitState = STATE_PARTIALLY_FILLED;
                    }

                    // average exit price at update
                    if (signal.avgPrice != signal.PRICE_UNDEFINED) {
                        m_exitPrice = signal.avgPrice;
                    } else if (signal.execPrice != signal.PRICE_UNDEFINED) {
                        if (m_exitPrice > 0.0) {
                            // compute
                            o3d::Double decQty = m_positionQuantity - lastQty;
                            m_exitPrice = (m_exitPrice * prevFilledExitQuantity + signal.execPrice * decQty) / (prevFilledExitQuantity + decQty);
                        } else {
                            // first time
                            m_exitPrice = signal.execPrice;
                        }
                    }

                    updateRealizedPnl();

                } else if (lastQty > m_positionQuantity) {
                    // increase mean entry
                    if (m_stats.firstRealizedExitTimestamp <= 0.0 && signal.updated != signal.TIMESTAMP_UNDEFINED) {
                        m_stats.firstRealizedExitTimestamp = signal.updated;
                    }

                    if (signal.updated != signal.TIMESTAMP_UNDEFINED) {
                        m_stats.lastRealizedExitTimestamp = signal.updated;
                    }

                    // filled entry quantity from the diff with the previous one
                    o3d::Double prevFilledEntryQuantity = m_filledEntryQuantity;
                    m_filledEntryQuantity += lastQty - m_positionQuantity;

                    if (m_filledEntryQuantity >= m_orderQuantity) {
                        m_entryState = STATE_FILLED;

                        // entry cannot longer be canceled once fully filled
                        m_entryOrderId = "";
                        m_entryRefId = "";
                    } else {
                        m_entryState = STATE_PARTIALLY_FILLED;
                    }

                    // average entry price at update
                    if (signal.avgPrice != signal.PRICE_UNDEFINED) {
                        m_entryPrice = signal.avgPrice;
                    } else if (signal.execPrice != signal.PRICE_UNDEFINED) {
                        if (m_entryPrice > 0.0) {
                            // compute
                            o3d::Double incQty = lastQty - m_positionQuantity;
                            m_entryPrice = (m_entryPrice * prevFilledEntryQuantity + signal.execPrice * incQty) / (prevFilledEntryQuantity * incQty);
                        } else {
                            // first time
                            m_entryPrice = signal.execPrice;
                        }
                    }

                    updateRealizedPnl();
                }

                // keep for close and for delta computation on update
                m_positionQuantity = lastQty;
            }
        } else if (signal.event == signal.DELETED) {
            // no longer related position
            m_positionId = "";

            // related create order is no longer valid
            m_entryOrderId = "";
            m_entryRefId = "";

            // filled exit quantity equal to the entry
            o3d::Double prevFilledExitQuantity = m_filledExitQuantity;

            // realized qty at close
            o3d::Double decQty = 0.0;
            if (m_filledExitQuantity < m_filledEntryQuantity) {
                decQty = m_filledEntryQuantity - m_filledExitQuantity;
                m_filledExitQuantity = m_filledEntryQuantity;
            }

            m_exitState = STATE_FILLED;

            // retains the last trade timestamp
            if (signal.updated != signal.TIMESTAMP_UNDEFINED) {
                m_stats.lastRealizedExitTimestamp = signal.updated;
            }

            // average exit price at delete
            if (signal.avgPrice != signal.PRICE_UNDEFINED) {
                m_exitPrice = signal.avgPrice;
            } else if (signal.execPrice != signal.PRICE_UNDEFINED) {
                if (m_exitPrice > 0.0) {
                    // compute
                    m_exitPrice = (m_exitPrice * prevFilledExitQuantity + signal.execPrice * decQty) / (prevFilledExitQuantity * decQty);
                } else {
                    m_exitPrice = signal.execPrice;
                }
            }

            // DBG(o3d::String("Exit position avg-price={0} cum-filled={1}").arg(m_exitPrice).arg(m_filledExitQuantity), "");

            // finally empty
            m_positionQuantity = 0.0;

            updateRealizedPnl();

        } else if (signal.event == signal.AMENDED) {
            // update stop_loss/take_profit
            if (signal.limitPrice != signal.PRICE_UNDEFINED) {
                m_positionLimitPrice = signal.limitPrice;
            }

            if (signal.stopPrice != signal.PRICE_UNDEFINED) {
                m_positionStopPrice = signal.stopPrice;
            }
        }
    }
}

o3d::Bool PositionTrade::isTargetOrder(const o3d::String &orderId, const o3d::String &refId) const
{
    if (orderId.isValid() && orderId == m_entryOrderId) {
        return true;
    }

    if (refId.isValid() && refId == m_entryRefId) {
        return true;
    }

    return false;
}

o3d::Bool PositionTrade::isTargetPosition(const o3d::String &positionId, const o3d::String &refId) const
{
    if (positionId.isValid() && positionId == m_positionId) {
        return true;
    }

    if (refId.isValid() && refId == m_entryRefId) {
        return true;
    }

    return false;
}

o3d::String PositionTrade::formatToStr() const
{
    o3d::String result;

    if (m_entryState == STATE_NEW) {
        return "new";
    } else if (m_entryState== STATE_OPENED) {
        return "opened";
    } else if (m_entryState == STATE_REJECTED) {
        return "rejected";
    } else if (m_entryState == STATE_REJECTED && m_filledEntryQuantity > m_filledExitQuantity) {
        return "problem";
    } else if (m_filledEntryQuantity < m_filledExitQuantity && (m_entryState == STATE_PARTIALLY_FILLED || m_entryState == STATE_OPENED)) {
        return "filling";
    } else if (m_filledEntryQuantity > 0.0 && m_filledExitQuantity < m_filledEntryQuantity && m_exitState != STATE_FILLED) {
        return "closing";
    } else if (m_filledEntryQuantity > 0.0 && m_filledExitQuantity >= m_filledEntryQuantity) {
        return "closed";
    } else if (m_filledEntryQuantity >= m_orderQuantity) {
        return "filled";
    } else {
        return "waiting";
    }
}

o3d::String PositionTrade::stateToStr() const
{
    o3d::String result;

    // @todo format

    return result;
}

void PositionTrade::updateStats(o3d::Double lastPrice, o3d::Double timestamp)
{
    Trade::updateStats(lastPrice, timestamp);

    if (isActive()) {
        // @todo
//            # non realized quantity
//            nrq = self.e - self.x

//            delta_price = 0.0
//            r_delta_price = 0.0

//            if self.dir > 0:
//                delta_price = instrument.market_bid - self.aep
//                r_delta_price = self.axp - self.aep
//            elif self.dir < 0:
//                delta_price = self.aep - instrument.market_ask
//                r_delta_price = self.aep - self.axp

//            upnl = nrq * (delta_price / (instrument.one_pip_means or 1.0)) * instrument.value_per_pip
//            rpnl = self.x * (r_delta_price / (instrument.one_pip_means or 1.0)) * instrument.value_per_pip
//            # upnl = nrq * delta_price * instrument.contract_size  # no have contract size on instrument
//            # rpnl = self.x * r_delta_price * instrument.contract_size

//            # including fees and realized profit and loss
//            self._stats['unrealized-profit-loss'] = instrument.adjust_quote(
//                upnl + rpnl - self._stats['entry-fees'] - self._stats['exit-fees'])
    }
}

void PositionTrade::dumps(o3d::Variadic &trades, Market *market) const
{
    // @todo
}

void PositionTrade::loads(const o3d::Variadic &trade)
{
    // @todo
}
