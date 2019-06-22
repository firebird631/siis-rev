/**
 * @brief SiiS moving average convergence divergence indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/macd/macd.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Macd::Macd(const o3d::String &name, o3d::Double timeframe, o3d::Int32 fastLen, o3d::Int32 slowLen, o3d::Int32 signalLen) :
    Indicator (name, timeframe),
    m_fastLen(fastLen),
    m_slowLen(slowLen),
    m_signalLen(signalLen),
    m_prev(0.0),
    m_last(0.0)
{
}

Macd::Macd(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_fastLen(0),
    m_slowLen(0),
    m_signalLen(0),
    m_prev(0.0),
    m_last(0.0)
{
     m_fastLen = conf.data().get("fastLen", 26).asInt();
     m_slowLen = conf.data().get("slowLen", 12).asInt();
     m_signalLen = conf.data().get("signalLen", 9).asInt();
}

void Macd::setConf(IndicatorConfig conf)
{
    m_fastLen = conf.data().get("fastLen", 26).asInt();
    m_slowLen = conf.data().get("slowLen", 12).asInt();
    m_signalLen = conf.data().get("signalLen", 9).asInt();
}

void Macd::compute(o3d::Double timestamp, const DataArray &price)
{
    m_prev = m_last;

    if (m_macd.getSize() != price.getSize()) {
        m_macd.setSize(price.getSize());
        m_signal.setSize(price.getSize());
        m_hist.setSize(price.getSize());
    }

    int b, n;
    TA_RetCode res = ::TA_MACD(0, price.getSize()-1, price.getData(),
                               m_fastLen, m_slowLen, m_signalLen,
                               &b, &n, m_macd.getData(), m_signal.getData(), m_hist.getData());
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == m_slowLen-1);  // @todo which one ?

    m_last = m_macd.getLast();
    done(timestamp);
}
