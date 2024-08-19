/**
 * @brief SiiS Pullback range-bar strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#include "pullbackrbsessionanalyser.h"

#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"

using namespace siis;

PullbackRbSessionAnalyser::PullbackRbSessionAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, rangeSize, depth, history, priceMethod),
    m_vp("volumeprofile", 0.0)
{

}

PullbackRbSessionAnalyser::~PullbackRbSessionAnalyser()
{

}

o3d::String PullbackRbSessionAnalyser::typeName() const
{
    return "session";
}

void PullbackRbSessionAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "vp", m_vp);

    m_vp.init(strategy()->market()->precisionPrice(), strategy()->market()->stepPrice());
    m_vp.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    RangeBarAnalyser::init(conf);
}

void PullbackRbSessionAnalyser::terminate()
{

}

void PullbackRbSessionAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{

}

void PullbackRbSessionAnalyser::updateTick(const Tick &tick, o3d::Bool finalize)
{
    m_vp.update(tick, finalize);
}
