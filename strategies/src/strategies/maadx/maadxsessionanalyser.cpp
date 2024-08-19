/**
 * @brief SiiS MAADX strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#include "maadxsessionanalyser.h"

#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"

using namespace siis;

MaAdxSessionAnalyser::MaAdxSessionAnalyser(
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

MaAdxSessionAnalyser::~MaAdxSessionAnalyser()
{

}

o3d::String MaAdxSessionAnalyser::typeName() const
{
    return "session";
}

void MaAdxSessionAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "vp", m_vp);

    m_vp.init(strategy()->market()->precisionPrice(), strategy()->market()->stepPrice());
    m_vp.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    TimeframeBarAnalyser::init(conf);
}

void MaAdxSessionAnalyser::terminate()
{

}

void MaAdxSessionAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{

}

void MaAdxSessionAnalyser::updateTick(const Tick &tick, o3d::Bool finalize)
{
    m_vp.update(tick, finalize);
}
