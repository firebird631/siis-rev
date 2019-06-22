/**
 * @brief SiiS strategy asset trade model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/trade/assettrade.h"
#include "siis/utils/common.h"

using namespace siis;

AssetTrade::AssetTrade() :
    Trade(Trade::TYPE_BUY_SELL, -1.0),
    m_buyState(STATE_NONE),
    m_stopState(STATE_NONE),
    m_limitState(STATE_NONE)
{

}

AssetTrade::AssetTrade(o3d::Double timeframe) :
    Trade(Trade::TYPE_BUY_SELL, timeframe),
    m_buyState(STATE_NONE),
    m_stopState(STATE_NONE),
    m_limitState(STATE_NONE)
{

}

AssetTrade::~AssetTrade()
{

}

void AssetTrade::open(
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

void AssetTrade::remove(TraderProxy *trader)
{

}

void AssetTrade::cancelOpen(TraderProxy *trader)
{

}

void AssetTrade::cancelClose(TraderProxy *trader)
{

}

void AssetTrade::modifyTakeProfit(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder)
{

}

void AssetTrade::modifyStopLoss(TraderProxy *trader, Market *market, o3d::Double price, o3d::Bool asOrder)
{

}

void AssetTrade::close(TraderProxy *trader, Market *market)
{

}

o3d::Bool AssetTrade::canDelete() const
{
    return false;
}

o3d::Bool AssetTrade::isActive() const
{
    return false;
}

o3d::Bool AssetTrade::isOpened() const
{
    return false;
}

o3d::Bool AssetTrade::isCanceled() const
{
    if (m_buyState == STATE_REJECTED) {
        return true;
    }

    if (m_buyState == STATE_CANCELED && m_filledEntryQuantity <= 0) {
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

o3d::Bool AssetTrade::isOpening() const
{
    return m_buyState == STATE_OPENED || m_buyState == STATE_PARTIALLY_FILLED;
}

o3d::Bool AssetTrade::isClosing() const
{
    return false;
}

o3d::Bool AssetTrade::isClosed() const
{
    return false;
}

o3d::Bool AssetTrade::isEntryTimeout(o3d::Double timestamp, o3d::Double timeout) const
{
    return false;
}

o3d::Bool AssetTrade::isExitTimeout(o3d::Double timestamp, o3d::Double timeout) const
{
    return false;
}

o3d::Bool AssetTrade::isValid() const
{
    return false;
}

void AssetTrade::orderSignal(const OrderSignal &signal)
{

}

void AssetTrade::positionSignal(const PositionSignal &signal)
{

}

o3d::Bool AssetTrade::isTargetOrder(const o3d::String &orderId, const o3d::String &orderRefId) const
{
    return false;
}

o3d::Bool AssetTrade::isTargetPosition(const o3d::String &positionId, const o3d::String &orderRefId) const
{
    return false;
}

o3d::String AssetTrade::formatToStr() const
{
    o3d::String result;

    // @todo format

    return result;
}

o3d::String AssetTrade::stateToStr() const
{
    if (m_buyState == STATE_NEW) {
        return "new";
    } else if (m_buyState == STATE_OPENED) {
        return "opened";
    } else if (m_buyState == STATE_REJECTED) {
        return "rejected";
    } else if (m_buyState == STATE_REJECTED && m_filledEntryQuantity > m_filledExitQuantity) {
        return "problem";
    } else if (m_filledEntryQuantity < m_filledExitQuantity && (m_buyState == STATE_PARTIALLY_FILLED || m_buyState == STATE_OPENED)) {
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

void AssetTrade::save(Database *db, Market *market)
{
    // @todo
}
