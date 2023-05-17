/**
 * @brief SiiS strategy asset trade model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/trade/assettrade.h"
#include "siis/utils/common.h"
#include "siis/connector/traderproxy.h"
#include "siis/market.h"

using namespace siis;

AssetTrade::AssetTrade(TraderProxy *proxy) :
    Trade(proxy, Trade::TYPE_BUY_SELL, -1.0),
    m_buyState(STATE_UNDEFINED),
    m_stopState(STATE_UNDEFINED),
    m_limitState(STATE_UNDEFINED)
{

}

AssetTrade::~AssetTrade()
{

}

void AssetTrade::init(o3d::Double timeframe)
{
    Trade::init(timeframe);
}

void AssetTrade::open(
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

    // m_openTimeStamp = trader->createOrder()
}

void AssetTrade::remove()
{

}

void AssetTrade::cancelOpen()
{

}

void AssetTrade::cancelClose()
{

}

void AssetTrade::modifyTakeProfit(o3d::Double price, ModifierType mod)
{

}

void AssetTrade::modifyStopLoss(o3d::Double price, ModifierType mod)
{

}

void AssetTrade::close(TradeStats::ExitReason reason)
{
    // @todo
    m_filledExitQuantity = m_orderQuantity;
}

void AssetTrade::process(o3d::Double timestamp)
{

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

void AssetTrade::dumps(o3d::Variadic &trades, Market *market) const
{
    // @todo
}

void AssetTrade::loads(const o3d::Variadic &trade)
{
    // @todo
}
