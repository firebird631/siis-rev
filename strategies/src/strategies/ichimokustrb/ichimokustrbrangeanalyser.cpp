/**
 * @brief SiiS Ichimoku strategy range filter analyser range-bar.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#include "ichimokustrbrangeanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

IchimokuStRbRangeAnalyser::IchimokuStRbRangeAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 barSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, barSize, depth, history, priceMethod, 1.0),
    m_slow_ma_high("slow_ma_high", barSize),
    m_slow_ma_low("slow_ma_high", barSize),
    m_fast_ma_high("fast_ma_high", barSize),
    m_fast_ma_low("fast_ma_high", barSize),
    m_inRange(false)
{

}

IchimokuStRbRangeAnalyser::~IchimokuStRbRangeAnalyser()
{

}

o3d::String IchimokuStRbRangeAnalyser::typeName() const
{
    return "range-filter";
}

void IchimokuStRbRangeAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "slow_ma_high", m_slow_ma_high);
    configureIndictor(conf, "slow_ma_low", m_slow_ma_low);
    configureIndictor(conf, "fast_ma_high", m_fast_ma_high);
    configureIndictor(conf, "fast_ma_low", m_fast_ma_low);

    RangeBarAnalyser::init(conf);
}

void IchimokuStRbRangeAnalyser::terminate()
{

}

void IchimokuStRbRangeAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
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
