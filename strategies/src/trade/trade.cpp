/**
 * @brief SiiS strategy trade model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/trade/trade.h"
#include "siis/utils/common.h"

using namespace siis;

Trade::Trade(Type type, o3d::Double timeframe) :
    m_id(-1),
    m_type(type),
    m_timeframe(timeframe),
    m_timestamp(0),
    m_direction(0),
    m_openTimeStamp(0),
    m_exitTimeStamp(0),
    m_orderQuantity(0),
    m_filledEntryQuantity(0),
    m_filledExitQuantity(0),
    m_profitLossRate(0)
{

}

Trade::~Trade()
{

}

void Trade::init(o3d::Double timeframe)
{
    m_id = -1;
    m_timeframe = timeframe;
    m_timestamp = 0;
    m_direction = 0;
    m_openTimeStamp = 0;
    m_exitTimeStamp = 0;
    m_orderQuantity = 0;
    m_filledEntryQuantity = 0;
    m_filledExitQuantity = 0;
    m_profitLossRate = 0;

    m_stats.init();
}

o3d::Bool Trade::isActive() const
{
    return m_filledEntryQuantity > 0.0 && m_filledExitQuantity < m_filledEntryQuantity;
}

void Trade::addCondition(const o3d::String &name, o3d::Double v1, o3d::Double v2, o3d::Double v3, o3d::Double v4)
{
    m_stats.conditions.push_back(TradeCondition(name, v1, v2, v3, v4));
}

void Trade::addOperation(TradeOperation *tradeOp)
{

}

void Trade::removeOperation(o3d::Int32 id)
{

}

void Trade::updateStats(o3d::Double lastPrice, o3d::Double timestamp)
{
    if (isActive()) {
        if (m_direction > 0) {
            if (lastPrice > m_stats.bestPrice) {
                m_stats.bestPrice = lastPrice;
                m_stats.bestPriceTimestamp = timestamp;
            }

            if (lastPrice < m_stats.worstPrice || m_stats.worstPrice <= 0.0) {
                m_stats.worstPrice = lastPrice;
                m_stats.worstPriceTimestamp = timestamp;
            }
        } else if (m_direction < 0) {
            if (lastPrice < m_stats.bestPrice || m_stats.bestPrice <= 0.0) {
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
}

void TradeStats::dumps() const
{
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
//            'entry-fees': 0.0,
//            'exit-fees': 0.0,
//            'conditions': {}
}
