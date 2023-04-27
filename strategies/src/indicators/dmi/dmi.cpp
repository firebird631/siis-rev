/**
 * @brief  SiiS directional moving index indicator.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "siis/indicators/dmi/dmi.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Dmi::Dmi(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len) :
    Indicator (name, timeframe),
    m_len(len),
    m_prev_m(0.0),
    m_last_m(0.0),
    m_prev_p(0.0),
    m_last_p(0.0)
{
}

Dmi::Dmi(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0),
    m_prev_m(0.0),
    m_last_m(0.0),
    m_prev_p(0.0),
    m_last_p(0.0)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 14).asInt();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 14).asInt();
    }
}

void Dmi::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 14).asInt();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 14).asInt();
    }
}

void Dmi::compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close)
{
    o3d::Int32 lb = lookback();
    if (high.getSize() <= lb) {
        return;
    }

    m_prev_m = m_last_m;
    m_prev_p = m_last_p;

    if (m_dmi_m.getSize() != high.getSize()) {
        m_dmi_m.setSize(high.getSize());
        m_dmi_p.setSize(high.getSize());
    }

    int b, n;
    TA_RetCode res = ::TA_MINUS_DI(0, high.getSize()-1, high.getData(), low.getData(), close.getData(), m_len, &b, &n, m_dmi_m.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    res = ::TA_PLUS_DI(0, high.getSize()-1, high.getData(), low.getData(), close.getData(), m_len, &b, &n, m_dmi_p.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_last_m = m_dmi_m.getLast();
    m_last_p = m_dmi_p.getLast();
    done(timestamp);
}

o3d::Int32 Dmi::lookback() const
{
    return ::TA_PLUS_DI_Lookback(m_len);  // ::TA_MINUS_DI_Lookback(m_len);
}
