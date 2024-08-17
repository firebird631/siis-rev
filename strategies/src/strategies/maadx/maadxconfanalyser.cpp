/**
 * @brief SiiS MAADX strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadxconfanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

MaAdxConfAnalyser::MaAdxConfAnalyser(
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

MaAdxConfAnalyser::~MaAdxConfAnalyser()
{

}

o3d::String MaAdxConfAnalyser::typeName() const
{
    return "conf";
}

void MaAdxConfAnalyser::init(const AnalyserConfig &conf)
{
    TimeframeBarAnalyser::init(conf);

    m_confirmation = 0;
}

void MaAdxConfAnalyser::terminate()
{

}

void MaAdxConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_confirmation = 0;

    o3d::Bool compute = true;

    if (isUpdateAtclose()) {
        compute = price().consolidated();
    }

    if (compute) {
        if (price().close().last() > price().close().prev()) {
            m_confirmation = 1;
        } else if (price().close().last() < price().close().prev()) {
            m_confirmation = -1;
        }
    }
}
