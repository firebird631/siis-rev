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
        o3d::Double timeframe,
        o3d::Double subTimeframe,
        o3d::Int32 depth,
        o3d::Int32 history) :
    m_strategy(strategy),
    m_timeframe(timeframe),
    m_subTimeFrame(subTimeframe),
    m_depth(depth),
    m_history(history),
    m_nextTimestamp(0),
    m_lastSignal(0, 0)
{

}

Analyser::~Analyser()
{

}

o3d::Bool Analyser::isNeedUpdate(o3d::Double timestamp) const
{
    return timestamp >= m_nextTimestamp;
}

void Analyser::log(const o3d::String &channel, const o3d::String &msg)
{
    m_strategy->log(timeframe(), channel, msg);
}
