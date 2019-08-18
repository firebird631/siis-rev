/**
 * @brief SiiS simple moving average indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/ema/ema.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Ema::Ema(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len) :
    Indicator (name, timeframe),
    m_len(len),
    m_prev(0.0),
    m_last(0.0)
{
}

Ema::Ema(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0),
    m_prev(0.0),
    m_last(0.0)
{
    m_len = conf.data().get("len", 20).asInt();
}

void Ema::setConf(IndicatorConfig conf)
{
    m_len = conf.data().get("len", 20).asInt();
}

void Ema::compute(o3d::Double timestamp, const DataArray &price)
{
    o3d::Int32 lb = lookback();
    if (price.getSize() <= lb) {
        return;
    }

    m_prev = m_last;

    if (m_ema.getSize() != price.getSize()) {
        m_ema.setSize(price.getSize());
    }

    int b, n;
    TA_RetCode res = ::TA_EMA(0, price.getSize()-1, price.getData(), m_len, &b, &n, m_ema.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_last = m_ema.getLast();
    done(timestamp);
}

o3d::Int32 Ema::lookback() const
{
    return m_len-1;  // ::TA_EMA_Lookback(m_len);
}
