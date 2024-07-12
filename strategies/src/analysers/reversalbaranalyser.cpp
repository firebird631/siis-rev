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
        o3d::Int32 rangeSize,
        o3d::Int32 reversalSize,
        o3d::Int32 depth,
        o3d::Int32 history,
        Price::Method priceMethod,
        o3d::Double tickScale) :
    Analyser(strategy, rangeSize, 0.0, 0.0, depth, history),
    m_ohlcGen(rangeSize, reversalSize, tickScale),
    m_ohlc(depth),
    m_price("price", 0.0, priceMethod),
    m_volume("volume", 0.0)
{

}

ReversalBarAnalyser::~ReversalBarAnalyser()
{

}

void ReversalBarAnalyser::init(AnalyserConfig /*conf*/)
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
}

void ReversalBarAnalyser::prepare(o3d::Double timestamp)
{

}

void ReversalBarAnalyser::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    // generate the ohlc from the last market update
    m_ohlcGen.genFromTicks(ticks, m_ohlc);
}

void ReversalBarAnalyser::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc)
{
    /* not supported */
}

o3d::Bool ReversalBarAnalyser::process(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    o3d::Int32 n = m_ohlc.size();

    if (n < depth()) {
        // not enought samples
        return false;
    }

    if ((*m_ohlc.cbegin())->timestamp() <= 0.0) {
        // not enought samples
        return false;
    }

    m_price.compute(m_ohlc);
    m_volume.compute(m_ohlc);

    // last input data source timestamp as current timestamp limit
    o3d::Double lastInputTimestamp = m_price.lastTimestamp();

    TradeSignal tradeSignal = compute(timestamp, lastInputTimestamp);

    // @todo this doesnt work because each time a candle is closed a new one is opened
    if (m_price.consolidated()) {
        // last OHLC is not consolidated then the next timestamp is the timestamp of this last tick
        processCompleted(lastInputTimestamp);
    }

    // avoid duplicates signals
    if (tradeSignal.type() != TradeSignal::NONE) {
        // retains this valid signal
        signalCompleted(tradeSignal);

//        if (tradeSignal.entry()) {
//            log("default", "entry");
//        } else if (tradeSignal.exit()) {
//            log("default", "exit");
//        }

        // returns new signal
        return true;
    } else {
        // no new signal
        return false;
    }
}

o3d::Double ReversalBarAnalyser::lastPrice() const
{
    return m_price.close().last();
}
