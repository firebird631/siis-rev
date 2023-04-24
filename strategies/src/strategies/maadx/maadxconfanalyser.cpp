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
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod)
{

}

MaAdxConfAnalyser::~MaAdxConfAnalyser()
{

}

void MaAdxConfAnalyser::init(AnalyserConfig conf)
{
    StdAnalyser::init(conf);
}

void MaAdxConfAnalyser::terminate()
{

}

TradeSignal MaAdxConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    return signal;
}
