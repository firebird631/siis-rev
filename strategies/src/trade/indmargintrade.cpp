/**
 * @brief SiiS strategy indivisible position, margin trade model specialization.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/trade/indmargintrade.h"
#include "siis/utils/common.h"

using namespace siis;

IndMarginTrade::IndMarginTrade() :
    Trade(Trade::TYPE_IND_MARGIN, -1.0),
    m_entryState(STATE_NONE),
    m_stopState(STATE_NONE),
    m_limitState(STATE_NONE)
{

}

IndMarginTrade::IndMarginTrade(o3d::Double timeframe) :
    Trade(Trade::TYPE_IND_MARGIN, timeframe),
    m_entryState(STATE_NONE),
    m_stopState(STATE_NONE),
    m_limitState(STATE_NONE)
{

}

IndMarginTrade::~IndMarginTrade()
{

}

void IndMarginTrade::open(
        TraderProxy *trader,
        Market *market,
        o3d::Int32 direction,
        Trade::OrderType orderType,
        o3d::Double orderPrice,
        o3d::Double quantity,
        o3d::Double stopLossPrice,
        o3d::Double takeProfitPrice,
        o3d::Double leverage)
{

}

void IndMarginTrade::remove(TraderProxy *trader)
{

}

void IndMarginTrade::cancelOpen(TraderProxy *trader)
{

}

void IndMarginTrade::cancelClose(TraderProxy *trader)
{

}

void IndMarginTrade::modifyTakeProfit(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder)
{

}

void IndMarginTrade::modifyStopLoss(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder)
{

}

void IndMarginTrade::close(TraderProxy *trader, Market *market)
{

}

o3d::Bool IndMarginTrade::canDelete() const
{
    return false;
}

o3d::Bool IndMarginTrade::isActive() const
{
    return false;
}

o3d::Bool IndMarginTrade::isOpened() const
{
    return false;
}

o3d::Bool IndMarginTrade::isCanceled() const
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

o3d::Bool IndMarginTrade::isOpening() const
{
    return m_entryState == STATE_OPENED || m_entryState == STATE_PARTIALLY_FILLED;
}

o3d::Bool IndMarginTrade::isClosing() const
{
    return false;
}

o3d::Bool IndMarginTrade::isClosed() const
{
    return false;
}

o3d::Bool IndMarginTrade::isEntryTimeout(o3d::Double timestamp, o3d::Double timeout) const
{
    return false;
}

o3d::Bool IndMarginTrade::isExitTimeout(o3d::Double timestamp, o3d::Double timeout) const
{
    return false;
}

o3d::Bool IndMarginTrade::isValid() const
{
    return false;
}

void IndMarginTrade::orderSignal(const OrderSignal &signal)
{

}

void IndMarginTrade::positionSignal(const PositionSignal &signal)
{

}

o3d::Bool IndMarginTrade::isTargetOrder(const o3d::String &orderId, const o3d::String &orderRefId) const
{
    return false;
}

o3d::Bool IndMarginTrade::isTargetPosition(const o3d::String &positionId, const o3d::String &orderRefId) const
{
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

void IndMarginTrade::dumps(o3d::Variadic &trades, Market *market) const
{
    // @todo
}

void IndMarginTrade::loads(const o3d::Variadic &trade)
{
    // @todo
}
