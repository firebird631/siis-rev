/**
 * @brief SiiS donchian channel indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/donchian/donchian.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Donchian::Donchian(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len) :
    Indicator (name, timeframe),
    m_len(len),
    m_prevUpper(0.0),
    m_lastUpper(0.0),
    m_prevLower(0.0),
    m_lastLower(0.0)
{
}

Donchian::Donchian(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0),
    m_prevUpper(0.0),
    m_lastUpper(0.0),
    m_prevLower(0.0),
    m_lastLower(0.0)
{
    m_len = conf.data().get("len", 14).asInt();
}

void Donchian::setConf(IndicatorConfig conf)
{
    m_len = conf.data().get("len", 14).asInt();
}

void Donchian::compute(o3d::Double timestamp, const DataArray &high, const DataArray &low)
{
    if (high.getSize() <= 0) {
        return;
    }

    m_prevUpper = m_lastUpper;
    m_prevLower = m_lastLower;

    if (m_upper.getSize() != high.getSize()) {
        m_upper.setSize(high.getSize());
    }

    int b, n;
    TA_RetCode res = ::TA_MAX(0, high.getSize()-1, high.getData(), m_len, &b, &n, m_upper.getData());
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    res = ::TA_MIN(0, low.getSize()-1, low.getData(), m_len, &b, &n, m_lower.getData());
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == 0);

    m_lastUpper = m_upper.getLast();
    m_lastLower = m_lower.getLast();
    done(timestamp);
}

o3d::Int32 Donchian::lookback() const
{
    return 0;
}
