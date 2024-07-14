/**
 * @brief SiiS indicealpha strategy base analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "iaanalyser.h"

using namespace siis;

IaAnalyser::IaAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_lastSignal(timeframe, 0.0)
{

}

IaAnalyser::~IaAnalyser()
{

}
