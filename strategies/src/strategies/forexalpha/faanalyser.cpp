/**
 * @brief SiiS forexalpha strategy base analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "faanalyser.h"

using namespace siis;

FaAnalyser::FaAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_lastSignal(timeframe, 0.0)
{

}

FaAnalyser::~FaAnalyser()
{

}
