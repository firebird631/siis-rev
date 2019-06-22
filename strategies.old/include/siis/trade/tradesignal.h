/**
 * @brief SiiS strategy trade signal model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#ifndef SIIS_TRADESIGNAL_H
#define SIIS_TRADESIGNAL_H

#include "../base.h"
#include "../utils/common.h"

#include <list>

namespace siis {

/**
 * @brief Strategy trade signal model.
 * @author Frederic Scherma
 * @date 2019-03-15
 */
class SIIS_API TradeSignal
{
public:

    enum Type {
        NONE = 0,
        ENTRY = 1,
        EXIT = 2,
    };

    TradeSignal(o3d::Double timeframe, o3d::Double timestamp) :
        m_type(NONE),
        m_timeframe(timeframe),
        m_timestamp(timestamp),
        m_direction(0),
        m_price(0),
        m_stopLossPrice(0),
        m_takeProfitPrice(0)
    {
    }

    TradeSignal(const TradeSignal &dup) :
        m_type(dup.m_type),
        m_timeframe(dup.m_timeframe),
        m_timestamp(dup.m_timestamp),
        m_direction(dup.m_direction),
        m_price(dup.m_price),
        m_stopLossPrice(dup.m_stopLossPrice),
        m_takeProfitPrice(dup.m_takeProfitPrice)
    {
    }

    TradeSignal& operator= (const TradeSignal &dup)
    {
        m_type = dup.m_type;
        m_timeframe = dup.m_timeframe;
        m_timestamp = dup.m_timestamp;
        m_direction = dup.m_direction;
        m_price = dup.m_price;
        m_stopLossPrice = dup.m_stopLossPrice;
        m_takeProfitPrice = dup.m_takeProfitPrice;

        return *this;
    }

    o3d::Double baseTime() const { return siis::baseTime(m_timestamp, m_timeframe); }

    Type type() const { return m_type; }
    o3d::Bool valid() const { return m_type != NONE; }
    o3d::Bool entry() const { return m_type == ENTRY; }
    o3d::Bool exit() const { return m_type == EXIT; }
    void setType(Type type) { m_type = type; }
    void setEntry() { m_type = ENTRY; }
    void setExit() { m_type = EXIT; }

    o3d::Double timeframe() const { return m_timeframe; }
    o3d::Double tf() const { return m_timeframe; }

    o3d::Double timestamp() const { return m_timestamp; }
    o3d::Double ts() const { return m_timestamp; }

    o3d::Int32 direction() const { return m_direction; }
    o3d::Int32 dir() const { return m_direction; }
    o3d::Int32 d() const { return m_direction; }
    void setDirection(o3d::Int32 d) { m_direction = d; }
    void setLong() { m_direction = LONG; }
    void setBuy() { m_direction = LONG; }
    void setShort() { m_direction = SHORT; }
    void setSell() { m_direction = SHORT; }

    o3d::Double price() const { return m_price; }
    o3d::Double p() const { return m_price; }
    void setPrice(o3d::Double p) { m_price = p; }

    o3d::Double stopLossPrice() const { return m_stopLossPrice; }
    o3d::Double stopLoss() const { return m_stopLossPrice; }
    o3d::Double stop() const { return m_stopLossPrice; }
    o3d::Double sl() const { return m_stopLossPrice; }
    void setStopLossPrice(o3d::Double p) { m_stopLossPrice = p; }
    void setStopLoss(o3d::Double p) { m_stopLossPrice = p; }
    void setStop(o3d::Double p) { m_stopLossPrice = p; }

    o3d::Double takeProfitPrice() const { return m_takeProfitPrice; }
    o3d::Double takeProfit() const { return m_takeProfitPrice; }
    o3d::Double limit() const { return m_takeProfitPrice; }
    o3d::Double tp() const { return m_takeProfitPrice; }
    void setTakeProfitPrice(o3d::Double p) { m_takeProfitPrice = p; }
    void setTakeProfit(o3d::Double p) { m_takeProfitPrice = p; }
    void setLimit(o3d::Double p) { m_takeProfitPrice = p; }

    /**
     * @brief addCondition Add a condition related to the emission of this signal.
     */
    // void addCondition(Condition* condition) { m_conditions.push_back(condition); }

private:

    Type m_type;

    o3d::Double m_timeframe;
    o3d::Double m_timestamp;

    o3d::Int32 m_direction;

    o3d::Double m_price;
    o3d::Double m_stopLossPrice;
    o3d::Double m_takeProfitPrice;

    // std::list<Condition*> m_conditions;
};

} // namespace siis

#endif // SIIS_TRADESIGNAL_H
