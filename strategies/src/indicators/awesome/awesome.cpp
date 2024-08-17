/**
 * @brief SiiS Bollinger band + Awesome indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-08-17
 */

#include "siis/indicators/awesome/awesome.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Awesome::Awesome(const o3d::String &name, o3d::Double timeframe,
            o3d::Int32 bollingerLen,
            o3d::Double baseMultiplier,
            o3d::Int32 fastMA_Len,
            o3d::Int32 awesomeFastLen,
            o3d::Int32 awesomeSlowLen,
            o3d::Bool useEma,
            o3d::Bool bollingerFilter,
            o3d::Bool squeezeFilter,
            o3d::Int32 squeezeLen,
            o3d::Int32 squeezeThreshold) :
    Indicator (name, timeframe),
    m_bollingerLen(bollingerLen),
    m_baseMultiplier(baseMultiplier),
    m_fastMaLen(fastMA_Len),
    m_awesomeFastLen(awesomeFastLen),
    m_awesomeSlowLen(awesomeSlowLen),
    m_useEma(useEma),
    m_bollingerFilter(bollingerFilter),
    m_squeezeFilter(squeezeFilter),
    m_squeezeLen(squeezeLen),
    m_squeezeThreshold(squeezeThreshold),
    m_signal(0),
    m_filteredSignal(0)
{
}

Awesome::Awesome(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_bollingerLen(0),
    m_baseMultiplier(0.0),
    m_fastMaLen(0),
    m_awesomeFastLen(0),
    m_awesomeSlowLen(0),
    m_useEma(false),
    m_bollingerFilter(false),
    m_squeezeFilter(false),
    m_squeezeLen(0),
    m_squeezeThreshold(0),
    m_signal(0),
    m_filteredSignal(0)
{
    if (conf.data().isObject()) {
        m_bollingerLen = conf.data().get("bollingerLen", 20).asInt();
        m_baseMultiplier = conf.data().get("baseMultiplier", 2).asInt();
        m_fastMaLen = conf.data().get("fastMaLen", 3).asInt();
        m_awesomeFastLen = conf.data().get("awesomeFastLen", 5).asInt();
        m_awesomeSlowLen = conf.data().get("awesomeSlowLen", 34).asInt();
        m_useEma = conf.data().get("useEma", false).asBool();
        m_bollingerFilter = conf.data().get("bollingerFilter", false).asBool();
        m_squeezeFilter = conf.data().get("squeezeFilter", false).asBool();
        m_squeezeLen = conf.data().get("squeezeLen", 100).asInt();
        m_squeezeThreshold = conf.data().get("squeezeThreshold", 50).asInt();
    } else if (conf.data().isArray()) {
        m_bollingerLen = conf.data().get((Json::ArrayIndex)1, 20).asInt();
        m_fastMaLen = conf.data().get((Json::ArrayIndex)2, 3).asInt();
        m_awesomeFastLen = conf.data().get((Json::ArrayIndex)3, 5).asInt();
        m_awesomeSlowLen = conf.data().get((Json::ArrayIndex)4, 34).asInt();
        m_useEma = conf.data().get((Json::ArrayIndex)5, false).asBool();
        m_bollingerFilter = conf.data().get((Json::ArrayIndex)6, false).asBool();
        m_squeezeFilter = conf.data().get((Json::ArrayIndex)7, false).asBool();
        m_squeezeLen = conf.data().get((Json::ArrayIndex)8, 100).asInt();
        m_squeezeThreshold = conf.data().get((Json::ArrayIndex)9, 50).asInt();
    }
}

void Awesome::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_bollingerLen = conf.data().get("bollingerLen", 20).asInt();
        m_baseMultiplier = conf.data().get("baseMultiplier", 2).asInt();
        m_fastMaLen = conf.data().get("fastMaLen", 3).asInt();
        m_awesomeFastLen = conf.data().get("awesomeFastLen", 5).asInt();
        m_awesomeSlowLen = conf.data().get("awesomeSlowLen", 34).asInt();
        m_useEma = conf.data().get("useEma", false).asBool();
        m_bollingerFilter = conf.data().get("bollingerFilter", false).asBool();
        m_squeezeFilter = conf.data().get("squeezeFilter", false).asBool();
        m_squeezeLen = conf.data().get("squeezeLen", 100).asInt();
        m_squeezeThreshold = conf.data().get("squeezeThreshold", 50).asInt();
    } else if (conf.data().isArray()) {
        m_bollingerLen = conf.data().get((Json::ArrayIndex)1, 20).asInt();
        m_fastMaLen = conf.data().get((Json::ArrayIndex)2, 3).asInt();
        m_awesomeFastLen = conf.data().get((Json::ArrayIndex)3, 5).asInt();
        m_awesomeSlowLen = conf.data().get((Json::ArrayIndex)4, 34).asInt();
        m_useEma = conf.data().get((Json::ArrayIndex)5, false).asBool();
        m_bollingerFilter = conf.data().get((Json::ArrayIndex)6, false).asBool();
        m_squeezeFilter = conf.data().get((Json::ArrayIndex)7, false).asBool();
        m_squeezeLen = conf.data().get((Json::ArrayIndex)8, 100).asInt();
        m_squeezeThreshold = conf.data().get((Json::ArrayIndex)9, 50).asInt();
    }
}

void Awesome::compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close)
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

        m_bbDev.setSize(size);
        m_bbUpper.setSize(size);
        m_bbLower.setSize(size);

        m_spread.setSize(size);
        m_avgSpread.setSize(size);

        m_bbOffset.setSize(size);
        m_bbSqueeze.setSize(size);

        m_bbSqzUpper.setSize(size);
        m_bbSqzLower.setSize(size);
    }

    // compute bollinger bands
    int b, n;
    TA_RetCode res = ::TA_BBANDS(0, close.getSize()-1, close.getData(), m_bollingerLen,
                                 m_baseMultiplier, m_baseMultiplier,
                                 m_useEma ? TA_MAType_EMA : TA_MAType_SMA,
                                 &b, &n, m_bbUpper.getData()+lb, m_bbBasis.getData()+lb, m_bbLower.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    close.ema(m_fastMaLen, m_fastMA);  // fast MA

    // Calculate Awesome Oscillator
    m_hl2.add(high, low);
    m_hl2 *= 0.5;

    // fast SMA, slow SMA
    m_hl2.sma(m_awesomeFastLen, m_SMA1_hl2);
    m_hl2.sma(m_awesomeSlowLen, m_SMA2_hl2);

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

    // filter the signal
    m_spread = m_bbUpper - m_bbLower;

    // calculate BB spread and average spread
    res = ::TA_SMA(0, m_spread.getSize()-1, close.getData(), m_squeezeLen, &b, &n, m_avgSpread.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    // calculate BB relative %width for Squeeze indication
    m_bbSqueeze = m_spread / m_avgSpread * 100.0;

    // calculate Upper and Lower band painting offsets based on 50% of ATR
    res = ::TA_ATR(0, high.getSize()-1, high.getData(), low.getData(), close.getData(), 14,
                              &b, &n, m_bbOffset.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_bbOffset *= 0.5;

    m_bbSqzUpper = m_bbUpper + m_bbOffset;
    m_bbSqzLower = m_bbLower + m_bbOffset;

    // calc breakouts
    o3d::Int32 cross = m_fastMA.cross(m_bbBasis);

    m_signal = 0;
    m_filteredSignal = 0;

    // and signal
    if (cross < 0 && close.getLast() < m_bbBasis.getLast() && o3d::abs(awesome) == 2) {
        m_signal = -1;

        // filtered signal
        if (m_signal < 0 && (!m_bollingerFilter || close.getLast() > m_bbLower.last()) && (
            !m_squeezeFilter || m_bbSqueeze.last() > m_squeezeThreshold)) {
            m_filteredSignal = -1;
        }
    } else if (cross > 0 && close.getLast() > m_bbBasis.getLast() && o3d::abs(awesome) == 1) {
        m_signal = 1;

        // filtered signal
        if (m_signal > 0 && (!m_bollingerFilter || close.getLast() < m_bbUpper.getLast()) && (
            !m_squeezeFilter || m_bbSqueeze.last() > m_squeezeFilter)) {
            m_filteredSignal = 1;
        }
    }

    done(timestamp);
}

o3d::Int32 Awesome::lookback() const
{
    return m_bollingerLen;
}
