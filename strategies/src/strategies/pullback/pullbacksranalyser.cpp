/**
 * @brief SiiS Pullback strategy support/resistance analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#include "pullbacksranalyser.h".h"

#include "siis/config/strategyconfig.h"

using namespace siis;

PullbackSRAnalyser::PullbackSRAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_pivotpoint("pivotpoint", timeframe)
{

}

PullbackSRAnalyser::~PullbackSRAnalyser()
{

}

void PullbackSRAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "pivotpoint", m_pivotpoint);

    StdAnalyser::init(conf);
}

void PullbackSRAnalyser::terminate()
{

}

TradeSignal PullbackSRAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    m_pivotpoint.compute(timestamp, price().open(), price().high(), price().low(), price().close());

    return signal;
}
