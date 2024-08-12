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
        const o3d::String &name,
        o3d::Double timeframe,
        o3d::Double sourceTimeframe,
        o3d::Int32 depth,
        o3d::Int32 history,
        Price::Method priceMethod) :
    Analyser(strategy, name, timeframe, 0, depth, history),
    m_ohlcGen(sourceTimeframe, timeframe),
    m_ohlc(depth),
    m_price("price", timeframe, priceMethod),
    m_volume("volume", timeframe)
{

}

TimeframeBarAnalyser::~TimeframeBarAnalyser()
{

}

void TimeframeBarAnalyser::init(const AnalyserConfig &conf)
{
    // configuration parameters
    if (conf.data().isMember("update-at-close")) {
        setUpdateAtClose(conf.data().get("update-at-close", false).asBool());
    }
}

void TimeframeBarAnalyser::prepare(o3d::Double timestamp)
{

}

void TimeframeBarAnalyser::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    // generate the ohlc from the last market update
    o3d::Int32 n = m_ohlcGen.genFromTicks(ticks, m_ohlc, *this);

    // new generated bars
    incNumLastBars(n);

    // plus the current one
    if (ticks.getSize() > 0 && m_ohlcGen.current() != nullptr) {
        incNumLastBars(1);
    }
}

void TimeframeBarAnalyser::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc)
{
    if (timeframe == m_ohlcGen.fromTimeframe()) {
        o3d::Int32 n = m_ohlcGen.genFromOhlc(ohlc, m_ohlc, *this);

        // new generated bars
        incNumLastBars(n);

    } else if (timeframe == m_ohlcGen.toTimeframe()) {
        // a bulk of finaly OHLC (generally initial)
        for (o3d::Int32 i = 0; i < ohlc.getSize(); ++i) {
            m_ohlc.writeElt()->copy(ohlc[i].data());
        }

        // new appended bars
        incNumLastBars(ohlc.getSize());
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

    // m_price.compute(m_ohlc);
    // m_volume.compute(m_ohlc);
    // prefers the faster incremental method
    m_price.computeMinimalist(m_ohlc, m_ohlcGen.current(), numLastBars());
    m_volume.computeMinimalist(m_ohlc, m_ohlcGen.current(), numLastBars());

    // last input data source timestamp as current timestamp limit
    o3d::Double lastInputTimestamp = m_price.lastTimestamp();

    compute(timestamp, lastInputTimestamp);

    if (m_price.consolidated()) {
        // last OHLC is consolidated then the next timestamp is incremented by timeframe.
        processCompleted(lastInputTimestamp + timeframe());
    } else {
        // last OHLC is not consolidated then the next timestamp is the timestamp of this last OHLC
        processCompleted(lastInputTimestamp);
    }

    resetNumLastBars();
}

o3d::Double TimeframeBarAnalyser::lastPrice() const
{
    return m_price.close().last();
}

o3d::String TimeframeBarAnalyser::formatUnit() const
{
    return timeframeToStr(timeframe());
}

o3d::Double TimeframeBarAnalyser::sourceTimeframe() const
{
    return m_ohlcGen.fromTimeframe();
}
