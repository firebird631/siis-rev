/**
 * @brief SiiS SuperTrend strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#include "supertrendconfanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

SuperTrendConfAnalyser::SuperTrendConfAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_confirmation(0)
{

}

SuperTrendConfAnalyser::~SuperTrendConfAnalyser()
{

}

o3d::String SuperTrendConfAnalyser::typeName() const
{
    return "conf";
}

void SuperTrendConfAnalyser::init(const AnalyserConfig &conf)
{
    TimeframeBarAnalyser::init(conf);

    m_confirmation = 0;
}

void SuperTrendConfAnalyser::terminate()
{

}

void SuperTrendConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_confirmation = 0;

    if (price().consolidated()) {
        if (price().close().last() > price().close().prev()) {
            m_confirmation = 1;
        } else if (price().close().last() < price().close().prev()) {
            m_confirmation = -1;
        }
    }
}
