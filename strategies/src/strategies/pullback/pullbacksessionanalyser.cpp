/**
 * @brief SiiS Pullback strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#include "pullbacksessionanalyser.h"

#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"

using namespace siis;

PullbackSessionAnalyser::PullbackSessionAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_vp("volumeprofile", timeframe)
{

}

PullbackSessionAnalyser::~PullbackSessionAnalyser()
{

}

o3d::String PullbackSessionAnalyser::typeName() const
{
    return "session";
}

void PullbackSessionAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "vp", m_vp);

    m_vp.init(strategy()->market()->precisionPrice(), strategy()->market()->stepPrice());
    m_vp.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    TimeframeBarAnalyser::init(conf);
}

void PullbackSessionAnalyser::terminate()
{

}

void PullbackSessionAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{

}

void PullbackSessionAnalyser::updateTick(const Tick &tick, o3d::Bool finalize)
{
    m_vp.update(tick, finalize);
}
