/**
 * @brief SiiS strategy standard implementation of analyser per timeframe.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/analysers/stdanalyser.h"
#include "siis/market.h"
#include "siis/strategy.h"

using namespace siis;

StdAnalyser::StdAnalyser(Strategy *strategy,
        o3d::Double timeframe,
        o3d::Double subTimeframe,
        o3d::Int32 depth,
        o3d::Int32 history,
        Price::Method priceMethod) :
    Analyser(strategy, timeframe, subTimeframe, depth, history),
    m_ohlcGen(subTimeframe, timeframe),
    m_ohlc(depth),
    m_price("price", timeframe, priceMethod),
    m_volume("volume", timeframe)
{

}

StdAnalyser::~StdAnalyser()
{

}

void StdAnalyser::init(AnalyserConfig /*conf*/)
{

}

void StdAnalyser::prepare(o3d::Double timestamp)
{

}

void StdAnalyser::onTickUpdate(o3d::Double timestamp, const TickArray &ticks)
{
    // generate the ohlc from the last market update
    m_ohlcGen.genFromTicks(ticks, m_ohlc);
}

void StdAnalyser::onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc)
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

o3d::Bool StdAnalyser::process(o3d::Double timestamp, o3d::Double lastTimestamp)
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
        // last OHLC is consolidated then the next timestamp is incremented by timeframe.
        processCompleted(lastInputTimestamp + timeframe());
    } else {
        // last OHLC is not consolidated then the next timestamp is the timestamp of this last OHLC
        processCompleted(lastInputTimestamp);
    }

    // avoid duplicates signals
    if (tradeSignal.type() != TradeSignal::NONE) {
        if (hasLastSignal() &&
            (lastSignal().type() == tradeSignal.type()) &&
            (lastSignal().direction() == tradeSignal.direction()) &&
            (static_cast<o3d::Int32>(lastSignal().baseTime()) == static_cast<o3d::Int32>(tradeSignal.baseTime()))) {
            // (tradeSignal.timestamp() - lastSignal().timestamp() < timeframe() * signalDelayFactor)) {

            // same base time, avoid multiple entries on the same candle, returns no new signal
            return false;
        } else {
            // retains this valid signal
            signalCompleted(tradeSignal);

//            if (tradeSignal.entry()) {
//                log("default", "entry");
//            } else if (tradeSignal.exit()) {
//                log("default", "exit");
//            }

            // returns new signal
            return true;
        }
    } else {
        // no new signal
        return false;
    }
}

o3d::Double StdAnalyser::lastPrice() const
{
    return m_price.close().last();
}
