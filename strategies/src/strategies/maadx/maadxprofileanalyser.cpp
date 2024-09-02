/**
 * @brief SiiS MAADX strategy profile analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-18
 */

#include "maadxprofileanalyser.h"

#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"

using namespace siis;

MaAdxProfileAnalyser::MaAdxProfileAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_imbalance("imbalance", timeframe)
{

}

MaAdxProfileAnalyser::~MaAdxProfileAnalyser()
{

}

o3d::String MaAdxProfileAnalyser::typeName() const
{
    return "profile";
}

void MaAdxProfileAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "imbalance", m_imbalance);

    m_imbalance.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    TimeframeBarAnalyser::init(conf);
}

void MaAdxProfileAnalyser::terminate()
{

}

void MaAdxProfileAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    o3d::Bool compute = true;

    if (isUpdateAtclose()) {
        compute = price().consolidated();
    }

    if (compute && m_imbalance.active()) {
        m_imbalance.compute(timestamp, price().timestamp(),
                            price().open(), price().high(), price().low(), price().close(),
                            numLastBars());
    }
}
