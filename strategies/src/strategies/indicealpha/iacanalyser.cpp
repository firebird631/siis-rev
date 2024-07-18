/**
 * @brief SiiS indicealpha strategy C analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#include "iacanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

IaCAnalyser::IaCAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    IaAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_atr("atr", timeframe),
    m_sma("sma", timeframe),
    m_ema("ema", timeframe),
    m_rsi("rsi", timeframe)
{

}

IaCAnalyser::~IaCAnalyser()
{

}

o3d::String IaCAnalyser::typeName() const
{
    return "C";
}

void IaCAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "atr", m_atr);
    configureIndictor(conf, "sma", m_sma);
    configureIndictor(conf, "ema", m_ema);
    configureIndictor(conf, "rsi", m_rsi);

    TimeframeBarAnalyser::init(conf);
}

void IaCAnalyser::terminate()
{

}

void IaCAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_rsi.compute(lastTimestamp, price().price());
    m_sma.compute(lastTimestamp, price().price());
    m_ema.compute(lastTimestamp, price().price());
    m_atr.compute(lastTimestamp, price().high(), price().low(), price().close());
}
