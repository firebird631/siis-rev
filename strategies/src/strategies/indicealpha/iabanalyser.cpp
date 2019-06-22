/**
 * @brief SiiS indicealpha strategy B analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#include "iabanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

IaBAnalyser::IaBAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_atr("atr", timeframe),
    m_sma("sma", timeframe),
    m_ema("ema", timeframe),
    m_rsi("rsi", timeframe)
{

}

IaBAnalyser::~IaBAnalyser()
{

}

void IaBAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "atr", m_atr);
    configureIndictor(conf, "sma", m_sma);
    configureIndictor(conf, "ema", m_ema);
    configureIndictor(conf, "rsi", m_rsi);

    StdAnalyser::init(conf);
}

void IaBAnalyser::terminate()
{

}

TradeSignal IaBAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

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

    if (signal.valid()) {
        // keep signal conditions for machine learning
        // @todo how to, another method storeConditions() when to do it ? we want to store any possible
        // valid signals, even thoose non traded because of somes criterias like max trade etc
        // 'price': prices[-1],
        // 'rsi': self.rsi.last,
        // 'stochrsi': self.stochrsi.last_k,
        // 'bollinger': (self.bollingerbands.last_bottom, self.bollingerbands.last_ma, self.bollingerbands.last_top),
    }

    return signal;
}
