/**
 * @brief SiiS Pullback strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#include "pullbackconfanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

PullbackConfAnalyser::PullbackConfAnalyser(
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

PullbackConfAnalyser::~PullbackConfAnalyser()
{

}

o3d::String PullbackConfAnalyser::typeName() const
{
    return "conf";
}

void PullbackConfAnalyser::init(const AnalyserConfig &conf)
{
    TimeframeBarAnalyser::init(conf);
}

void PullbackConfAnalyser::terminate()
{

}

void PullbackConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
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
