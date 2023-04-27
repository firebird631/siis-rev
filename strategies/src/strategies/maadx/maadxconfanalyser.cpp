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
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_confirmation(0)
{

}

MaAdxConfAnalyser::~MaAdxConfAnalyser()
{

}

void MaAdxConfAnalyser::init(AnalyserConfig conf)
{
    StdAnalyser::init(conf);
}
static int n=0;
void MaAdxConfAnalyser::terminate()
{
printf(">>> %i\n", n);
}

TradeSignal MaAdxConfAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);
++n;
    m_confirmation = 0;

    // @todo or one more candle
    if (price().consolidated()) {
        if (price().close().getLast() > price().open().getLast()) {
            m_confirmation = 1;
        } else if (price().close().getLast() < price().open().getLast()) {
            m_confirmation = -1;
        }
    }

    return signal;
}
