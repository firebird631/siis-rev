/**
 * @brief SiiS SuperTrend range-bar strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#include "supertrendrbconfanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

SuperTrendRbConfAnalyser::SuperTrendRbConfAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, rangeSize, depth, history, priceMethod, 1.0),
    m_confirmation(0)
{

}

SuperTrendRbConfAnalyser::~SuperTrendRbConfAnalyser()
{

}

o3d::String SuperTrendRbConfAnalyser::typeName() const
{
    return "conf";
}

void SuperTrendRbConfAnalyser::init(AnalyserConfig conf)
{
    RangeBarAnalyser::init(conf);

    m_confirmation = 0;
}

void SuperTrendRbConfAnalyser::terminate()
{

}

void SuperTrendRbConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
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
