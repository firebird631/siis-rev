/**
 * @brief SiiS strategy standard implementation of analyser per timeframe.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/analysers/timeframebaranalyser.h"
#include "siis/market.h"
#include "siis/strategy.h"

using namespace siis;

TimeframeBarAnalyser::TimeframeBarAnalyser(Strategy *strategy,
        o3d::Double timeframe,
        o3d::Double subTimeframe,
        o3d::Int32 depth,
        o3d::Int32 history,
        Price::Method priceMethod) :
    Analyser(strategy, timeframe, subTimeframe, 0, depth, history),
    m_ohlcGen(subTimeframe, timeframe),
    m_ohlc(depth),
    m_price("price", timeframe, priceMethod),
    m_volume("volume", timeframe)
{

}

TimeframeBarAnalyser::~TimeframeBarAnalyser()
{

}

void TimeframeBarAnalyser::init(AnalyserConfig /*conf*/)
{

}

void TimeframeBarAnalyser::prepare(o3d::Double timestamp)
{

}

void TimeframeBarAnalyser::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    // generate the ohlc from the last market update
    m_ohlcGen.genFromTicks(ticks, m_ohlc);
}

void TimeframeBarAnalyser::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc)
{
    if (timeframe == m_ohlcGen.fromTimeframe()) {
        m_ohlcGen.genFromOhlc(ohlc, m_ohlc);
    } else if (timeframe == m_ohlcGen.toTimeframe()) {
        // a bulk of finaly OHLC (generally initial)
        for (o3d::Int32 i = 0; i < ohlc.getSize(); ++i) {
            m_ohlc.writeElt()->copy(ohlc[i].data());
        }
    }
}

void TimeframeBarAnalyser::process(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    o3d::Int32 n = m_ohlc.size();

    if (n < depth()) {
        // not enought samples
        return;
    }

    if ((*m_ohlc.cbegin())->timestamp() <= 0.0) {
        // not enought samples
        return;
    }

    m_price.compute(m_ohlc);
    m_volume.compute(m_ohlc);

    // last input data source timestamp as current timestamp limit
    o3d::Double lastInputTimestamp = m_price.lastTimestamp();

    // @todo this doesnt work because each time a candle is closed a new one is opened
    if (m_price.consolidated()) {
        // last OHLC is consolidated then the next timestamp is incremented by timeframe.
        processCompleted(lastInputTimestamp + timeframe());
    } else {
        // last OHLC is not consolidated then the next timestamp is the timestamp of this last OHLC
        processCompleted(lastInputTimestamp);
    }
}

o3d::Double TimeframeBarAnalyser::lastPrice() const
{
    return m_price.close().last();
}
