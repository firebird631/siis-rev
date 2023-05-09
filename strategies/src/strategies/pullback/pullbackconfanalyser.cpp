/**
 * @brief SiiS MAADX strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#include "pullbackconfanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

PullbackConfAnalyser::PullbackConfAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_confirmation(0)
{

}

PullbackConfAnalyser::~PullbackConfAnalyser()
{

}

void PullbackConfAnalyser::init(AnalyserConfig conf)
{
    StdAnalyser::init(conf);
}

void PullbackConfAnalyser::terminate()
{

}

TradeSignal PullbackConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    m_confirmation = 0;

    if (price().consolidated()) {
        if (price().close().last() > price().close().prev()) {
            m_confirmation = 1;
        } else if (price().close().last() < price().close().prev()) {
            m_confirmation = -1;
        }
    }

    return signal;
}
