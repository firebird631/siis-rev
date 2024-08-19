/**
 * @brief SiiS forexalpha strategy B analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "fabanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

FaBAnalyser::FaBAnalyser(Strategy *strategy,
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

FaBAnalyser::~FaBAnalyser()
{

}

o3d::String FaBAnalyser::typeName() const
{
    return "B";
}

void FaBAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "atr", m_atr);
    configureIndicator(conf, "sma", m_sma);
    configureIndicator(conf, "ema", m_ema);
    configureIndicator(conf, "rsi", m_rsi);

    TimeframeBarAnalyser::init(conf);
}

void FaBAnalyser::terminate()
{

}

void FaBAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_rsi.compute(lastTimestamp, price().price());
    m_sma.compute(lastTimestamp, price().price());
    m_ema.compute(lastTimestamp, price().price());
    m_atr.compute(lastTimestamp, price().high(), price().low(), price().close());

    o3d::Int32 lvl1Signal = 0;

    if (m_ema.last() < m_sma.last()) {
        // bear trend
        if (m_rsi.last() > 50) {
            lvl1Signal = -1;
        } else if (m_rsi.last() < 20) {
            lvl1Signal = 1;
        }
    } else {
        // bull trend
        if (m_rsi.last() > 80) {
            lvl1Signal = -1;
        } else if (m_rsi.last() < 60) {
            lvl1Signal = 1;
        }
    }
}
