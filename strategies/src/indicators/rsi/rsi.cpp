/**
 * @brief SiiS relative strengh index indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/rsi/rsi.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Rsi::Rsi(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len) :
    Indicator (name, timeframe),
    m_len(len),
    m_prev(0.0),
    m_last(0.0)
{
}

Rsi::Rsi(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0),
    m_prev(0.0),
    m_last(0.0)
{
    m_len = conf.data().get("len", 21).asInt();
}

void Rsi::setConf(IndicatorConfig conf)
{
    m_len = conf.data().get("len", 21).asInt();
}

void Rsi::compute(o3d::Double timestamp, const DataArray &price)
{
    o3d::Int32 lb = lookback();
    if (price.getSize() <= lb) {
        return;
    }

    m_prev = m_last;

    if (m_rsi.getSize() != price.getSize()) {
        m_rsi.setSize(price.getSize());
    }

    int b, n;  // first len data are empty so add offset
    TA_RetCode res = ::TA_RSI(0, price.getSize(), price.getData(), m_len, &b, &n, m_rsi.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_last = m_rsi.getLast();
    done(timestamp);
}

o3d::Int32 Rsi::lookback() const
{
    return m_len;  // ::TA_RSI_Lookback(m_len);
}
