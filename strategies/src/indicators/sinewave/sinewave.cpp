/**
 * @brief SiiS SineWave Hilbert transform indicator
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-08-17
 */

#include "siis/indicators/sinewave/sinewave.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

SineWave::SineWave(const o3d::String &name, o3d::Double timeframe) :
    Indicator (name, timeframe),
    m_lastSine(0.0),
    m_prevSine(0.0),
    m_lastLeadSine(0.0),
    m_prevLeadSine(0.0)
{
}

SineWave::SineWave(const o3d::String &name, o3d::Double timeframe, IndicatorConfig /*conf*/) :
    Indicator (name, timeframe),
    m_lastSine(0.0),
    m_prevSine(0.0),
    m_lastLeadSine(0.0),
    m_prevLeadSine(0.0)
{
}

void SineWave::setConf(IndicatorConfig /*conf*/)
{
}

void SineWave::compute(o3d::Double timestamp, const DataArray &price)
{
    o3d::Int32 lb = lookback();
    if (price.getSize() <= lb) {
        return;
    }

    m_prevSine = m_lastSine;
    m_prevLeadSine = m_lastLeadSine;

    if (m_sine.getSize() != price.getSize()) {
        m_sine.setSize(price.getSize());
        m_leadSine.setSize(price.getSize());
    }

    int b, n;  // first len data are empty so add offset
    TA_RetCode res = ::TA_HT_SINE(0, price.getSize(), price.getData(), &b, &n, m_sine.getData()+lb, m_leadSine.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    m_lastSine = m_sine.getLast();
    m_lastLeadSine = m_leadSine.getLast();
    done(timestamp);
}

o3d::Int32 SineWave::cross() const
{
    if (m_prevSine > m_prevLeadSine && m_lastSine < m_lastLeadSine) {
        return -1;
    } else if (m_prevSine < m_prevLeadSine && m_lastSine > m_prevLeadSine) {
        return 1;
    }

    return 0;
}

o3d::Int32 SineWave::lookback() const
{
    return 63;  // ::TA_HT_SINE_Lookback();
}
