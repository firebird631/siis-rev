/**
 * @brief SiiS average true range indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/atr/atr.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Atr::Atr(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len, o3d::Double factor) :
    Indicator (name, timeframe),
    m_len(len),
    m_factor(factor),
    m_prev(0.0),
    m_last(0.0)
{
}

Atr::Atr(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0),
    m_factor(0),
    m_prev(0.0),
    m_last(0.0)
{
    m_len = conf.data().get("len", 20).asInt();
    m_factor = conf.data().get("factor", 3.5).asDouble();
}

void Atr::setConf(IndicatorConfig conf)
{
    m_len = conf.data().get("len", 20).asInt();
    m_factor = conf.data().get("factor", 3.5).asDouble();
}

void Atr::compute(o3d::Double timestamp,
                  const DataArray &high,
                  const DataArray &low,
                  const DataArray &close)
{
    m_prev = m_last;

    if (m_atr.getSize() != high.getSize()) {
        m_atr.setSize(high.getSize());
    }

    int b, n;
    TA_RetCode res = ::TA_ATR(0, high.getSize()-1, high.getData(), low.getData(), close.getData(), m_len,
                              &b, &n, m_atr.getData()+m_len);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == m_len);

    m_last = m_atr.getLast();

    // update stop-loss prices
    o3d::Double prev = m_longStopPrice;
    m_longStopPrice = close[close.getSize()-1] - (m_factor * m_last);
//    if (m_longStopPrice < prev) {
//        m_longStopPrice = prev;
//    }

    prev = m_shortStopPrice;
    m_shortStopPrice = close[close.getSize()-1] + (m_factor * m_last);
//    if (m_shortStopPrice < prev) {
//        m_shortStopPrice = prev;
//    }

    done(timestamp);
}
