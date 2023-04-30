/**
 * @brief SiiS strategy indivisible position, margin trade model specialization.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/trade/indmargintrade.h"
#include "siis/utils/common.h"
#include "siis/connector/traderproxy.h"
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

void IndMarginTrade::open(
        Strategy *strategy,
        o3d::Int32 direction,
        o3d::Double orderPrice,
        o3d::Double quantity,
        o3d::Double takeProfitPrice,
        o3d::Double stopLossPrice)
{
    m_strategy = strategy;

    // @todo
    m_direction = direction;
    m_orderQuantity = quantity;
    m_takeProfitPrice = takeProfitPrice;
    m_stopLossPrice = stopLossPrice;

    Order *entryOrder = traderProxy()->newOrder();
    entryOrder->direction = direction;

}

void IndMarginTrade::remove()
{

}

void IndMarginTrade::cancelOpen()
{

}

void IndMarginTrade::cancelClose()
{

}

void IndMarginTrade::modifyTakeProfit(o3d::Double price, o3d::Bool asOrder)
{

}

void IndMarginTrade::modifyStopLoss(o3d::Double price, o3d::Bool asOrder)
{

}

void IndMarginTrade::close()
{

}

void IndMarginTrade::process(o3d::Double timestamp)
{
    if (m_filledEntryQuantity)

    if (m_stopLossPrice > 0.0 && !m_stop.orderId.isValid()) {
        // if (m_filledExitQuantity < m_filledEntryQuantity)
    }

    if (m_takeProfitPrice > 0.0 && !m_limit.orderId.isValid()) {

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

}

void IndMarginTrade::positionSignal(const PositionSignal &signal)
{

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

void IndMarginTrade::dumps(o3d::Variadic &trades, Market *market) const
{
    // @todo
}

void IndMarginTrade::loads(const o3d::Variadic &trade)
{
    // @todo
}
