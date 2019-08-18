/**
 * @brief SiiS stop and reverse indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/sar/sar.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Sar::Sar(const o3d::String &name, o3d::Double timeframe, o3d::Double accel, o3d::Double max) :
    Indicator (name, timeframe),
    m_accel(accel),
    m_max(max),
    m_prev(0.0),
    m_last(0.0)
{
}

Sar::Sar(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_accel(0.0),
    m_max(0.0),
    m_prev(0.0),
    m_last(0.0)
{
    m_accel = conf.data().get("accel", 0.0).asDouble();
    m_max = conf.data().get("max", 0.0).asDouble();
}

void Sar::setConf(IndicatorConfig conf)
{
    m_accel = conf.data().get("accel", 0.0).asDouble();
    m_max = conf.data().get("max", 0.0).asDouble();
}

void Sar::compute(o3d::Double timestamp, const DataArray &high, const DataArray &low)
{
    o3d::Int32 lb = lookback();
    if (high.getSize() <= lb) {
        return;
    }

    m_prev = m_last;

    if (m_sar.getSize() != high.getSize()) {
        m_sar.setSize(high.getSize());
    }

    int b, n;
    TA_RetCode res = ::TA_SAR(0, high.getSize()-1, high.getData(), low.getData(), m_accel, m_max, &b, &n, m_sar.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_last = m_sar.getLast();
    done(timestamp);
}

o3d::Int32 Sar::lookback() const
{
    return 1;
}
