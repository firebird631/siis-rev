/**
 * @brief SiiS MAADX range-bar strategy profile analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-18
 */

#include "maadxrbprofileanalyser.h"

#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"

using namespace siis;

MaAdxRbProfileAnalyser::MaAdxRbProfileAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, rangeSize, depth, history, priceMethod),
    m_imbalance("imbalance", rangeSize)
{

}

MaAdxRbProfileAnalyser::~MaAdxRbProfileAnalyser()
{

}

o3d::String MaAdxRbProfileAnalyser::typeName() const
{
    return "profile";
}

void MaAdxRbProfileAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "imbalance", m_imbalance);

    m_imbalance.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    RangeBarAnalyser::init(conf);
}

void MaAdxRbProfileAnalyser::terminate()
{

}

void MaAdxRbProfileAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    o3d::Bool compute = true;

    if (isUpdateAtclose()) {
        compute = price().consolidated();
    }

    if (compute) {
        m_imbalance.compute(timestamp, price().timestamp(),
                            price().open(), price().high(), price().low(), price().close(),
                            numLastBars());
    }
}
