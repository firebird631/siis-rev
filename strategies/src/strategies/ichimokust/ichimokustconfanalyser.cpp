/**
 * @brief SiiS Ichimoku strategy confirmation analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#include "ichimokustconfanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

IchimokuStConfAnalyser::IchimokuStConfAnalyser(
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

IchimokuStConfAnalyser::~IchimokuStConfAnalyser()
{

}

o3d::String IchimokuStConfAnalyser::typeName() const
{
    return "conf";
}

void IchimokuStConfAnalyser::init(const AnalyserConfig &conf)
{
    TimeframeBarAnalyser::init(conf);

    m_confirmation = 0;
}

void IchimokuStConfAnalyser::terminate()
{

}

void IchimokuStConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
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
