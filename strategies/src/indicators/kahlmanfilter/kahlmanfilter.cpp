/**
 * @brief SiiS Kahlman filter indicator (that can be used over another indicator ouput).
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#include "siis/indicators/kahlmanfilter/kahlmanfilter.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

#include <o3d/core/math.h>

#include <cmath>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

KahlmanFilter::KahlmanFilter(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len, o3d::Double gain) :
    Indicator(name, timeframe),
    m_len(len),
    m_gain(0.0),
    m_g2Sqrt(0.0),
    m_prev(0.0),
    m_last(0.0)
{
    m_gain = gain;
    m_g2Sqrt = o3d::Math::sqrt(gain*2.0);
}

KahlmanFilter::KahlmanFilter(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_len(0),
    m_gain(0.0),
    m_prev(0.0),
    m_last(0.0)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 7).asInt();
        m_gain = conf.data().get("gain", 0.7).asDouble();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 7).asInt();
        m_gain = conf.data().get((Json::ArrayIndex)2, 0.7).asDouble();
    }

    m_g2Sqrt = o3d::Math::sqrt(m_gain*2.0);
}

void KahlmanFilter::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 7).asInt();
        m_gain = conf.data().get("gain", 0.7).asDouble();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 7).asInt();
        m_gain = conf.data().get((Json::ArrayIndex)2, 0.7).asDouble();
    }

    m_g2Sqrt = o3d::Math::sqrt(m_gain*2.0);
}

void KahlmanFilter::resize(o3d::Int32 len)
{
    if (len != m_kf.getSize()) {
        m_len = len;

        m_kf.setSize(len);
        m_dk.setSize(len);
        m_smooth.setSize(len);
        m_velo.setSize(len);
    }
}

void KahlmanFilter::compute(o3d::Double timestamp, const DataArray &price)
{
    o3d::Int32 lb = lookback();
    if (price.getSize() <= lb) {
        return;
    }

    m_prev = m_last;

    if (m_len != price.getSize()) {
        resize(price.getSize());
    }

    for (o3d::Int32 i = 0; i < m_len; ++i) {
        m_dk[i] = price[i] - (i == 0 || std::isnan(m_kf[i-1]) ? price[i] : m_kf[i-1]);
        m_smooth[i] = (i == 0 || std::isnan(m_kf[i-1]) ? price[i] : m_kf[i-1]) + m_dk[i] * m_g2Sqrt;
        m_velo[i] = (i == 0 || std::isnan(m_velo[i-1]) ? 0.0 : m_velo[i-1]) + (m_gain * m_dk[i]);
        m_kf[i] = m_smooth[i] + m_velo[i];
    }

    m_last = m_kf.last();
    done(timestamp);
}

o3d::Int32 KahlmanFilter::lookback() const
{
    return 1;
}
