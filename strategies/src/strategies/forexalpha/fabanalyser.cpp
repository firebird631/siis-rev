/**
 * @brief SiiS forexalpha strategy B analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "fabanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

FaBAnalyser::FaBAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    FaAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_atr("atr", timeframe),
    m_sma("sma", timeframe),
    m_ema("ema", timeframe),
    m_rsi("rsi", timeframe)
{

}

FaBAnalyser::~FaBAnalyser()
{

}

void FaBAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "atr", m_atr);
    configureIndictor(conf, "sma", m_sma);
    configureIndictor(conf, "ema", m_ema);
    configureIndictor(conf, "rsi", m_rsi);

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
