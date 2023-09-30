/**
 * @brief SiiS Supertrend indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#include "siis/indicators/supertrend/supertrend.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

SuperTrend::SuperTrend(const o3d::String &name, o3d::Double timeframe,  o3d::Int32 len, o3d::Double coeff) :
    Indicator (name, timeframe),
    m_len(len),
    m_coeff(coeff),
    m_prev(0.0),
    m_last(0.0)
{
}

SuperTrend::SuperTrend(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0.0),
    m_coeff(0.0),
    m_prev(0.0),
    m_last(0.0)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 14).asInt();
        m_coeff = conf.data().get("coeff", 3.0).asDouble();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 14).asInt();
        m_coeff = conf.data().get((Json::ArrayIndex)2, 3.0).asDouble();
    }
}

void SuperTrend::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 14).asInt();
        m_coeff = conf.data().get("coeff", 3.0).asDouble();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 14).asInt();
        m_coeff = conf.data().get((Json::ArrayIndex)2, 3.0).asDouble();
    }
}

o3d::Int32 SuperTrend::position() const
{
    return m_position.last();
}

o3d::Int32 SuperTrend::positionChange() const
{
    o3d::Int32 size = m_position.getSize();

    if (size > 2) {
        // at least 2+1 (generally empty first value)
        return m_position[size-2] != m_position[size-1] ? m_position[size-1] : 0;
    }

    return 0;
}

void SuperTrend::compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close)
{
    o3d::Int32 lb = lookback();
    if (high.getSize() <= lb) {
        return;
    }

    m_prev = m_last;

    if (m_trend.getSize() != high.getSize()) {
        // finals
        m_trend.setSize(high.getSize());
        m_position.setSize(high.getSize());
        m_up.setSize(high.getSize());
        m_down.setSize(high.getSize());

        // temporary buffers
        m_hl2.setSize(high.getSize());
        m_catr.setSize(high.getSize());
        m_upper.setSize(high.getSize());
        m_lower.setSize(high.getSize());
    }

    m_hl2.add(high, low);
    m_hl2 *= 0.5;

    int b, n;
    TA_RetCode res = ::TA_ATR(0, high.getSize()-1, high.getData(), low.getData(), close.getData(), m_len,
                              &b, &n, m_catr.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_catr.zeroNan();

    m_upper.sub(m_hl2, m_catr);
    m_lower.add(m_hl2, m_catr);

    o3d::Int32 len = high.getSize();

    for (o3d::Int32 i = 1; i < len; ++i) {
        m_up[i] = close[i-1] > m_up[i-1] ? o3d::max(m_upper[i], m_up[i-1]) : m_upper[i];
        m_down[i] = close[i-1] < m_down[i-1] ? o3d::min(m_lower[i], m_down[i-1]) : m_lower[i];
    }

    m_position[0] = 1;   // start with 1

    for (o3d::Int32 i = 1; i < len; ++i) {
        if (close[i] > m_down[i-1]) {
            m_position[i] = 1;
        } else if (close[i] < m_up[i-1]) {
            m_position[i] = -1;
        } else {
            m_position[i] = m_position[i-1];
        }

        m_trend[i] = m_position[i] > 0 ? m_up[i] : m_down[i];
    }

    m_last = m_trend.getLast();
    done(timestamp);
}

o3d::Int32 SuperTrend::lookback() const
{
    return m_len;  // ::TA_ATR_Lookback(m_len);
}
