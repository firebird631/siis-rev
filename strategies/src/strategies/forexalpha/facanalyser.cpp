/**
 * @brief SiiS forexalpha strategy C analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "facanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

FaCAnalyser::FaCAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    FaAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_atr("atr", timeframe),
    m_sma("sma", timeframe),
    m_ema("ema", timeframe),
    m_rsi("rsi", timeframe)
{

}

FaCAnalyser::~FaCAnalyser()
{

}

o3d::String FaCAnalyser::typeName() const
{
    return "C";
}

void FaCAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "atr", m_atr);
    configureIndictor(conf, "sma", m_sma);
    configureIndictor(conf, "ema", m_ema);
    configureIndictor(conf, "rsi", m_rsi);

    TimeframeBarAnalyser::init(conf);
}

void FaCAnalyser::terminate()
{

}

void FaCAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_rsi.compute(lastTimestamp, price().price());
    m_sma.compute(lastTimestamp, price().price());
    m_ema.compute(lastTimestamp, price().price());
    m_atr.compute(lastTimestamp, price().high(), price().low(), price().close());
}
