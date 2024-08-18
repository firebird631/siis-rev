/**
 * @brief SiiS MAADX range-bar strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#include "maadxrbsessionanalyser.h"

#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"

using namespace siis;

MaAdxRbSessionAnalyser::MaAdxRbSessionAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, rangeSize, depth, history, priceMethod),
    m_vp("volumeprofile", rangeSize),
    m_cvd("cvd", rangeSize, "1d")
{

}

MaAdxRbSessionAnalyser::~MaAdxRbSessionAnalyser()
{

}

o3d::String MaAdxRbSessionAnalyser::typeName() const
{
    return "session";
}

void MaAdxRbSessionAnalyser::init(const AnalyserConfig &conf)
{
    configureIndictor(conf, "vp", m_vp);
    configureIndictor(conf, "cvd", m_cvd);

    m_vp.init(strategy()->market()->precisionPrice(), strategy()->market()->stepPrice());
    m_vp.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    m_cvd.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    RangeBarAnalyser::init(conf);
}

void MaAdxRbSessionAnalyser::terminate()
{

}

void MaAdxRbSessionAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{

}

void MaAdxRbSessionAnalyser::updateTick(const Tick &tick, o3d::Bool finalize)
{
    m_vp.update(tick, finalize);
    m_cvd.update(tick, finalize);
}
