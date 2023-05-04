/**
 * @brief SiiS strategy position trade model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-28
 */

#include "siis/trade/positiontrade.h"
#include "siis/utils/common.h"
#include "siis/connector/traderproxy.h"
#include "siis/market.h"

using namespace siis;

PositionTrade::PositionTrade(TraderProxy *proxy) :
    Trade(proxy, Trade::TYPE_POSITION, -1.0),
    m_entryState(STATE_UNDEFINED),
    m_exitState(STATE_UNDEFINED),
    m_closing(false),
    m_positionLimitPrice(0.0),
    m_positionStopPrice(0.0)
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
    // @todo
}

void PositionTrade::cancelClose()
{
    // nothing to do
}

void PositionTrade::modifyTakeProfit(o3d::Double price, o3d::Bool asOrder)
{
    // @todo
}

void PositionTrade::modifyStopLoss(o3d::Double price, o3d::Bool asOrder)
{
    // @todo
}

void PositionTrade::close()
{
    // @todo
    m_filledExitQuantity = m_orderQuantity;
}

void PositionTrade::process(o3d::Double timestamp)
{
    // @todo
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
    return m_exitState == STATE_OPENED;
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

        if (m_entryOrderId.isEmpty()) {
            m_entryOrderId = signal.orderId;
        }

        // @todo
    }
}

void PositionTrade::positionSignal(const PositionSignal &signal)
{
    if (signal.positionId.isValid() && signal.positionId == m_positionId) {
        if (signal.event == signal.OPENED) {
            // @todo

        } else if (signal.event == signal.UPDATED) {
            // @todo

        } else if (signal.event == signal.DELETED) {
            // @todo
        }
    }
}

o3d::Bool PositionTrade::isTargetOrder(const o3d::String &orderId, const o3d::String &refId) const
{
    if (m_entryOrderId.isValid() && orderId == m_entryOrderId) {
        return true;
    }

    if (m_entryRefId.isValid() && refId == m_entryRefId) {
        return true;
    }

    return false;
}

o3d::Bool PositionTrade::isTargetPosition(const o3d::String &positionId, const o3d::String &refId) const
{
    if (m_positionId.isValid() && positionId == m_positionId) {
        return true;
    }

    if (m_entryRefId.isValid() && refId == m_entryRefId) {
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

void PositionTrade::dumps(o3d::Variadic &trades, Market *market) const
{
    // @todo
}

void PositionTrade::loads(const o3d::Variadic &trade)
{
    // @todo
}
