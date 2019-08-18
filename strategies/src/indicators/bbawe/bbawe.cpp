/**
 * @brief SiiS Bollinger band + Awesome indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-08-17
 */

#include "siis/indicators/bbawe/bbawe.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

BBawe::BBawe(const o3d::String &name, o3d::Double timeframe,
            o3d::Int32 bb_Len,
            o3d::Double baseMultiplier,
            o3d::Int32 fastMA_Len,
            o3d::Int32 awesomeFast_Len,
            o3d::Int32 awesomeSlow_Len,
            o3d::Bool useEma) :
    Indicator (name, timeframe),
    m_bb_Len(bb_Len),
    m_baseMultiplier(baseMultiplier),
    m_fastMA_Len(fastMA_Len),
    m_awesomeFast_Len(awesomeFast_Len),
    m_awesomeSlow_Len(awesomeSlow_Len),
    m_useEma(useEma),
    m_signal(0)
{
}

BBawe::BBawe(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_bb_Len(0),
    m_baseMultiplier(0.0),
    m_fastMA_Len(0),
    m_awesomeFast_Len(0),
    m_awesomeSlow_Len(0),
    m_useEma(false),
    m_signal(0)
{
    m_bb_Len = conf.data().get("bb_Len", 20).asInt();
    m_baseMultiplier = conf.data().get("baseMultiplier", 2).asInt();
    m_fastMA_Len = conf.data().get("fastMA_Len", 3).asInt();
    m_awesomeFast_Len = conf.data().get("awesomeFast_Len", 5).asInt();
    m_awesomeSlow_Len = conf.data().get("awesomeSlow_Len", 34).asInt();
    m_useEma = conf.data().get("useEma", false).asBool();
}

void BBawe::setConf(IndicatorConfig conf)
{
    m_bb_Len = conf.data().get("bb_Len", 20).asInt();
    m_baseMultiplier = conf.data().get("baseMultiplier", 2).asInt();
    m_fastMA_Len = conf.data().get("fastMA_Len", 3).asInt();
    m_awesomeFast_Len = conf.data().get("awesomeFast_Len", 5).asInt();
    m_awesomeSlow_Len = conf.data().get("awesomeSlow_Len", 34).asInt();
    m_useEma = conf.data().get("useEma", false).asBool();
}

void BBawe::compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &price)
{
    o3d::Int32 lb = lookback();
    if (high.getSize() <= lb) {
        return;
    }

    o3d::Int32 size = high.getSize();

    if (m_bbBasis.getSize() != size) {
        m_bbBasis.setSize(size);
        m_fastMA.setSize(size);

        m_hl2.setSize(size);
        m_SMA1_hl2.setSize(size);
        m_SMA2_hl2.setSize(size);
        m_SMA1_SMA2.setSize(size);
    }

    // Breakout Indicator Inputs
    if (m_useEma) {
        price.ema(m_bb_Len, m_bbBasis);  // BB basis as EMA
    } else {
        price.sma(m_bb_Len, m_bbBasis);  // BB basis as SMA
    }

    price.ema(m_fastMA_Len, m_fastMA);  // fast MA

    // Calculate Awesome Oscillator
    m_hl2.add(high, low);
    m_hl2 *= 0.5;

    // fast SMA, slow SMA
    m_hl2.sma(m_awesomeFast_Len, m_SMA1_hl2);
    m_hl2.sma(m_awesomeSlow_Len, m_SMA2_hl2);

    m_SMA1_SMA2.sub(m_SMA1_hl2, m_SMA2_hl2);

    // Calculate direction of awesome (0, 1 or 2 with the sign of the last)
    o3d::Int32 awesome = 0;

    if (m_SMA1_SMA2.getLast() >= 0.0) {
        if (m_SMA1_SMA2.getLast() > m_SMA1_SMA2[size-2]) {
            awesome = 1;
        } else {
            awesome = 2;
        }
    } else {
        if (m_SMA1_SMA2.getLast() > m_SMA1_SMA2[size-2]) {
            awesome = -1;
        } else {
            awesome = -2;
        }
    }

    // Calc breakouts
    o3d::Int32 cross = m_fastMA.cross(m_bbBasis);

    if (cross < 0 && price.getLast() < m_bbBasis.getLast() && o3d::abs(awesome) == 2) {
        m_signal = 1;
    } else if (cross > 0 && price.getLast() > m_bbBasis.getLast() && o3d::abs(awesome) == 1) {
        m_signal = -1;
    } else {
        m_signal = 0;
    }

    done(timestamp);
}

o3d::Int32 BBawe::lookback() const
{
    return m_bb_Len;
}
