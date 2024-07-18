/**
 * @brief SiiS Ichimoku strategy range filter analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#include "ichimokustrangeanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

IchimokuStRangeAnalyser::IchimokuStRangeAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_slow_ma_high("slow_ma_high", timeframe),
    m_slow_ma_low("slow_ma_high", timeframe),
    m_fast_ma_high("fast_ma_high", timeframe),
    m_fast_ma_low("fast_ma_high", timeframe),
    m_inRange(false)
{

}

IchimokuStRangeAnalyser::~IchimokuStRangeAnalyser()
{

}

o3d::String IchimokuStRangeAnalyser::typeName() const
{
    return "range-filter";
}

void IchimokuStRangeAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "slow_ma_high", m_slow_ma_high);
    configureIndictor(conf, "slow_ma_low", m_slow_ma_low);
    configureIndictor(conf, "fast_ma_high", m_fast_ma_high);
    configureIndictor(conf, "fast_ma_low", m_fast_ma_low);

    TimeframeBarAnalyser::init(conf);
}

void IchimokuStRangeAnalyser::terminate()
{

}

void IchimokuStRangeAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    if (price().consolidated()) {
        m_slow_ma_high.compute(timestamp, price().high());
        m_slow_ma_low.compute(timestamp, price().low());
        m_fast_ma_high.compute(timestamp, price().high());
        m_fast_ma_low.compute(timestamp, price().low());

        if (m_fast_ma_low.last() >= m_slow_ma_low.last() && m_fast_ma_low.last() <= m_slow_ma_high.last()) {
            // fast low between slow MAs
            m_inRange = true;
        } else if (m_fast_ma_high.last() >= m_slow_ma_low.last() && m_fast_ma_high.last() <= m_slow_ma_high.last()) {
            // fast high between slow MAs
            m_inRange = true;
        } else if (m_slow_ma_low.last() >= m_fast_ma_low.last() && m_slow_ma_low.last() <= m_fast_ma_high.last()) {
            // slow low between fast MAs
            m_inRange = true;
        } else if (m_slow_ma_high.last() >= m_fast_ma_low.last() && m_slow_ma_high.last() <= m_fast_ma_high.last()) {
            // slow high between fast MAs
            m_inRange = true;
        } else {
            m_inRange = false;
        }
    }
}
