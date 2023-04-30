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

PositionTrade::PositionTrade() :
    Trade(Trade::TYPE_POSITION, -1.0),
    m_entryState(STATE_UNDEFINED),
    m_stopState(STATE_UNDEFINED),
    m_limitState(STATE_UNDEFINED)
{

}

PositionTrade::PositionTrade(o3d::Double timeframe) :
    Trade(Trade::TYPE_POSITION, timeframe),
    m_entryState(STATE_UNDEFINED),
    m_stopState(STATE_UNDEFINED),
    m_limitState(STATE_UNDEFINED)
{

}

PositionTrade::~PositionTrade()
{

}

void PositionTrade::open(TraderProxy *trader,
        Market *market,
        o3d::Int32 direction,
        Trade::OrderType orderType,
        o3d::Double orderPrice,
        o3d::Double quantity,
        o3d::Double takeProfitPrice,
        o3d::Double stopLossPrice)
{
    // @todo
    m_direction = direction;
    m_orderQuantity = quantity;
    m_takeProfitPrice = takeProfitPrice;
    m_stopLossPrice = stopLossPrice;
}

void PositionTrade::remove(TraderProxy *trader)
{

}

void PositionTrade::cancelOpen(TraderProxy *trader)
{

}

void PositionTrade::cancelClose(TraderProxy *trader)
{

}

void PositionTrade::modifyTakeProfit(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder)
{

}

void PositionTrade::modifyStopLoss(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder)
{

}

void PositionTrade::close(TraderProxy *trader, Market *market)
{
    // @todo
    m_filledExitQuantity = m_orderQuantity;
}

o3d::Bool PositionTrade::canDelete() const
{
    // @todo
    return m_filledExitQuantity >= m_filledEntryQuantity;
}

o3d::Bool PositionTrade::isActive() const
{
    return m_filledExitQuantity < m_filledEntryQuantity;
}

o3d::Bool PositionTrade::isOpened() const
{
    return false;
}

o3d::Bool PositionTrade::isCanceled() const
{
    if (m_entryState == STATE_REJECTED) {
        return true;
    }

    if (m_entryState == STATE_CANCELED && m_filledEntryQuantity <= 0) {
        return true;
    }

    if (m_stopState == STATE_CANCELED && m_filledExitQuantity <= 0) {
        return true;
    }

    if (m_limitState == STATE_CANCELED && m_filledExitQuantity <= 0) {
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
    return false;
}

o3d::Bool PositionTrade::isClosed() const
{
    return false;
}

o3d::Bool PositionTrade::isEntryTimeout(o3d::Double timestamp, o3d::Double timeout) const
{
    return false;
}

o3d::Bool PositionTrade::isExitTimeout(o3d::Double timestamp, o3d::Double timeout) const
{
    return false;
}

o3d::Bool PositionTrade::isValid() const
{
    return false;
}

void PositionTrade::orderSignal(const OrderSignal &signal)
{

}

void PositionTrade::positionSignal(const PositionSignal &signal)
{

}

o3d::Bool PositionTrade::isTargetOrder(const o3d::String &orderId, const o3d::String &orderRefId) const
{
    return false;
}

o3d::Bool PositionTrade::isTargetPosition(const o3d::String &positionId, const o3d::String &orderRefId) const
{
    return false;
}

o3d::String PositionTrade::formatToStr() const
{
    o3d::String result;

    // @todo format

    return result;
}

o3d::String PositionTrade::stateToStr() const
{
    if (m_entryState == STATE_NEW) {
        return "new";
    } else if (m_entryState == STATE_OPENED) {
        return "opened";
    } else if (m_entryState == STATE_REJECTED) {
        return "rejected";
    } else if (m_entryState == STATE_REJECTED && m_filledEntryQuantity > m_filledExitQuantity) {
        return "problem";
    } else if (m_filledEntryQuantity < m_filledExitQuantity && (m_entryState == STATE_PARTIALLY_FILLED || m_entryState == STATE_OPENED)) {
        return "filling";
    } else if (m_filledEntryQuantity > 0.0 && m_filledExitQuantity < m_filledEntryQuantity && m_stopState != STATE_FILLED && m_limitState != STATE_FILLED) {
        return "closing";
    } else if (m_filledEntryQuantity > 0.0 && m_filledExitQuantity >= m_filledEntryQuantity) {
        return "closed";
    } else if (m_filledEntryQuantity >= m_orderQuantity) {
        return "filled";
    } else {
        return "waiting";
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
