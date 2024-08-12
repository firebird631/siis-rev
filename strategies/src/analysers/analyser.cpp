/**
 * @brief SiiS strategy analyser per timeframe.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/analysers/analyser.h"
#include "siis/strategy.h"

using namespace siis;

Analyser::Analyser(Strategy *strategy,
        const o3d::String &name,
        o3d::Double timeframe,
        o3d::Int32 barSize,
        o3d::Int32 depth,
        o3d::Int32 history) :
    m_strategy(strategy),
    m_name(name),
    m_timeframe(timeframe),
    m_barSize(barSize),
    m_depth(depth),
    m_history(history),
    m_updateAtClose(false),
    m_nextTimestamp(0),
    m_numLastBars(0)
{

}

Analyser::~Analyser()
{

}

void Analyser::updateTick(const Tick& tick, o3d::Bool finalize)
{
    // nothing by default
}

void Analyser::updateBar(const Ohlc &bar, o3d::Bool finalize)
{
    // nothing by default
}

o3d::Bool Analyser::isNeedUpdate(o3d::Double timestamp) const
{
    return timestamp >= m_nextTimestamp;
}

void Analyser::log(const o3d::String &channel, const o3d::String &msg)
{
    m_strategy->log(formatUnit(), channel, msg);
}
