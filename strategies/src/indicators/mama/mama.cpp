/**
 * @brief SiiS MESA Adaptive Moving Average indicator
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/mama/mama.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Mama::Mama(const o3d::String &name, o3d::Double timeframe, o3d::Double fastLimit, o3d::Double slowLimit) :
    Indicator (name, timeframe),
    m_fastLimit(fastLimit),
    m_slowLimit(slowLimit),
    m_lastFama(0.0),
    m_prevFama(0.0),
    m_lastMama(0.0),
    m_prevMama(0.0)
{
}

Mama::Mama(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_fastLimit(0.0),
    m_slowLimit(0.0),
    m_lastFama(0.0),
    m_prevFama(0.0),
    m_lastMama(0.0),
    m_prevMama(0.0)
{
    m_fastLimit = conf.data().get("fastLimit", 0.5).asDouble();
    m_slowLimit = conf.data().get("slowLimit", 0.05).asDouble();
}

void Mama::setConf(IndicatorConfig conf)
{
    m_fastLimit = conf.data().get("fastLimit", 0.5).asDouble();
    m_slowLimit = conf.data().get("slowLimit", 0.05).asDouble();
}

void Mama::compute(o3d::Double timestamp, const DataArray &price)
{
    o3d::Int32 lb = lookback();
    if (price.getSize() <= lb) {
        return;
    }

    m_prevFama = m_lastFama;
    m_prevMama = m_lastMama;

    if (m_fama.getSize() != price.getSize()) {
        m_fama.setSize(price.getSize());
        m_mama.setSize(price.getSize());
    }

    int b, n;
    TA_RetCode res = ::TA_MAMA(0, price.getSize(), price.getData(), m_fastLimit, m_slowLimit, &b, &n,
                               m_mama.getData()+lb, m_fama.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_lastFama = m_fama.getLast();
    m_lastMama = m_mama.getLast();
    done(timestamp);
}

o3d::Int32 Mama::cross() const
{
    if (m_prevMama > m_prevFama && m_lastMama < m_lastFama) {
        return -1;
    } else if (m_prevMama < m_prevFama && m_lastMama > m_lastFama) {
        return 1;
    }

    return 0;
}

o3d::Int32 Mama::trend() const
{
    if (m_lastMama > m_lastFama) {
        return 1;
    } else if (m_lastMama < m_lastFama) {
        return -1;
    }

    return 0;
}

o3d::Int32 Mama::lookback() const
{
    return 32;  // ::TA_MAMA_Lookback(m_fastLimit, m_slowLimit);
}
