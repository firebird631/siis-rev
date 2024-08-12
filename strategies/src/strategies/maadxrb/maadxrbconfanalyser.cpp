/**
 * @brief SiiS MAADX range-bar strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadxrbconfanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

MaAdxRbConfAnalyser::MaAdxRbConfAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, rangeSize, depth, history, priceMethod),
    m_confirmation(0)
{

}

MaAdxRbConfAnalyser::~MaAdxRbConfAnalyser()
{

}

o3d::String MaAdxRbConfAnalyser::typeName() const
{
    return "conf";
}

void MaAdxRbConfAnalyser::init(const AnalyserConfig &conf)
{
    RangeBarAnalyser::init(conf);

    m_confirmation = 0;
}

void MaAdxRbConfAnalyser::terminate()
{

}

void MaAdxRbConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
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
