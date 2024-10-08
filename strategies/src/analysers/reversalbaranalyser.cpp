/**
 * @brief SiiS strategy analyser for a reversal-bar series
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-12
 */

#include "siis/analysers/reversalbaranalyser.h"
#include "siis/market.h"
#include "siis/strategy.h"

using namespace siis;

ReversalBarAnalyser::ReversalBarAnalyser(Strategy *strategy,
        const o3d::String &name,
        o3d::Int32 rangeSize,
        o3d::Int32 reversalSize,
        o3d::Int32 depth,
        o3d::Double history,
        Price::Method priceMethod,
        o3d::Double tickScale) :
    Analyser(strategy, name, rangeSize, 0.0, depth, history),
    m_ohlcGen(rangeSize, reversalSize, tickScale),
    m_ohlc(depth),
    m_price("price", 0.0, priceMethod),
    m_volume("volume", 0.0)
{

}

ReversalBarAnalyser::~ReversalBarAnalyser()
{

}

void ReversalBarAnalyser::init(const AnalyserConfig &conf)
{
    o3d::Int32 pricePrecision = strategy()->market()->precisionPrice();
    if (pricePrecision == 0) {
        pricePrecision = 8;
    }

    o3d::Double tickSize = strategy()->market()->stepPrice();
    if (tickSize == 0) {
        tickSize = 0.00000001;
    }

    m_ohlcGen.init(pricePrecision, tickSize);

    // configuration parameters
    if (conf.data().isMember("update-at-close")) {
        setUpdateAtClose(conf.data().get("update-at-close", false).asBool());
    }
}

void ReversalBarAnalyser::prepare(o3d::Double timestamp)
{

}

void ReversalBarAnalyser::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    // generate the ohlc from the last market update
    m_ohlcGen.genFromTicks(ticks, m_ohlc, *this);
}

void ReversalBarAnalyser::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc)
{
    /* not supported */
}

void ReversalBarAnalyser::process(o3d::Double timestamp, o3d::Double lastTimestamp)
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

    if (m_price.consolidated()) {
        // last OHLC is not consolidated then the next timestamp is the timestamp of this last tick
        processCompleted(lastInputTimestamp);
    }

    resetNumLastBars();
}

o3d::Double ReversalBarAnalyser::lastPrice() const
{
    return m_price.close().last();
}
