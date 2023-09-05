 /**
 * @brief SiiS strategy KahlmanFibo strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#include "kahlmanfiboconfanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

KahlmanFiboConfAnalyser::KahlmanFiboConfAnalyser(
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

KahlmanFiboConfAnalyser::~KahlmanFiboConfAnalyser()
{

}

void KahlmanFiboConfAnalyser::init(AnalyserConfig conf)
{
    StdAnalyser::init(conf);

    m_confirmation = 0;
}

void KahlmanFiboConfAnalyser::terminate()
{

}

TradeSignal KahlmanFiboConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    m_confirmation = 0;

    if (1) { // price().consolidated()) {
        if (price().close().last() > price().close().prev()) {
            m_confirmation = 1;
        } else if (price().close().last() < price().close().prev()) {
            m_confirmation = -1;
        }
    }

    return signal;
}
