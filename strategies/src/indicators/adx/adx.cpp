/**
 * @brief  SiiS average directional index indicator.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "siis/indicators/adx/adx.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Adx::Adx(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len) :
    Indicator (name, timeframe),
    m_len(len),
    m_prev(0.0),
    m_last(0.0)
{
}

Adx::Adx(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0),
    m_prev(0.0),
    m_last(0.0)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 5).asInt();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 5).asInt();
    }
}

void Adx::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 5).asInt();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 5).asInt();
    }
}

void Adx::compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close)
{
    o3d::Int32 lb = lookback();
    if (high.getSize() <= lb) {
        return;
    }

    m_prev = m_last;

    if (m_adx.getSize() != high.getSize()) {
        m_adx.setSize(high.getSize());
    }

    int b, n;
    TA_RetCode res = ::TA_ADX(0, high.getSize()-1, high.getData(), low.getData(), close.getData(), m_len, &b, &n, m_adx.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_last = m_adx.getLast();
    done(timestamp);
}

o3d::Int32 Adx::lookback() const
{
    return m_len-1;  // ::TA_ADX_Lookback(m_len);
}
