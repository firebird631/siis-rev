/**
 * @brief SiiS strategy analyser for a range-bar series
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-12
 */

#include "siis/analysers/rangebaranalyser.h"
#include "siis/market.h"
#include "siis/strategy.h"

using namespace siis;

RangeBarAnalyser::RangeBarAnalyser(Strategy *strategy,
        const o3d::String &name,
        o3d::Int32 rangeSize,
        o3d::Int32 depth,
        o3d::Int32 history,
        Price::Method priceMethod,
        o3d::Double tickScale) :
    Analyser(strategy, name, 0.0, rangeSize, depth, history),
    m_ohlcGen(rangeSize, tickScale),
    m_ohlc(depth),
    m_price("price", 0.0, priceMethod),
    m_volume("volume", 0.0)
{

}

RangeBarAnalyser::~RangeBarAnalyser()
{

}

void RangeBarAnalyser::init(const AnalyserConfig &conf)
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

void RangeBarAnalyser::prepare(o3d::Double timestamp)
{

}

void RangeBarAnalyser::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
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

void RangeBarAnalyser::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc)
{
    // a bulk of bars (generally initial)
    for (o3d::Int32 i = 0; i < ohlc.getSize(); ++i) {
        m_ohlc.writeElt()->copy(ohlc[i].data());
    }

    // new appended bars
    incNumLastBars(ohlc.getSize());
}

void RangeBarAnalyser::process(o3d::Double timestamp, o3d::Double lastTimestamp)
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
        // last OHLC is now consolidated then the next timestamp is the timestamp of this last tick
        processCompleted(lastInputTimestamp);
    }

    resetNumLastBars();
}

o3d::Double RangeBarAnalyser::lastPrice() const
{
    return m_price.close().last();
}

o3d::String RangeBarAnalyser::formatUnit() const
{
    return o3d::String::print("%irb", barSize());
}
