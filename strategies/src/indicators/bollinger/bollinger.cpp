/**
 * @brief SiiS bollinger bands indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/bollinger/bollinger.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Bollinger::Bollinger(const o3d::String &name,
               o3d::Double timeframe,
               o3d::Int32 len,
               MAType maType,
               o3d::Double numDevUp,
               o3d::Double numDevDn) :
    Indicator (name, timeframe),
    m_len(len),
    m_maType(maType),
    m_numDevUp(numDevUp),
    m_numDevDn(numDevDn),
    m_prevUpper(0.0),
    m_lastUpper(0.0),
    m_prevMiddle(0.0),
    m_lastMiddle(0.0),
    m_prevLower(0.0),
    m_lastLower(0.0)
{
}

Bollinger::Bollinger(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0),
    m_maType(MA_SMA),
    m_numDevUp(0),
    m_numDevDn(0),
    m_prevUpper(0.0),
    m_lastUpper(0.0),
    m_prevMiddle(0.0),
    m_lastMiddle(0.0),
    m_prevLower(0.0),
    m_lastLower(0.0)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 21).asInt();

        m_maType = siis::maTypeFromStr(conf.data().get("maType", "SMA").asCString());
        m_numDevUp = conf.data().get("numDevUp", 2.0).asDouble();
        m_numDevDn = conf.data().get("numDevDn", 2.0).asDouble();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 21).asInt();

        m_numDevUp = 2.0;
        m_numDevDn = 2.0;
    }
}

void Bollinger::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 21).asInt();

        m_maType = siis::maTypeFromStr(conf.data().get("maType", "SMA").asCString());
        m_numDevUp = conf.data().get("numDevUp", 2.0).asDouble();
        m_numDevDn = conf.data().get("numDevDn", 2.0).asDouble();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 21).asInt();

        // keep them from ctor
//        m_numDevUp = 2.0;
//        m_numDevDn = 2.0;
    }
}

void Bollinger::compute(o3d::Double timestamp, const DataArray &price)
{
    o3d::Int32 lb = lookback();
    if (price.getSize() <= lb) {
        return;
    }

    m_prevUpper = m_lastUpper;
    m_prevMiddle = m_lastMiddle;
    m_prevLower = m_lastLower;

    if (m_upper.getSize() != price.getSize()) {
        m_upper.setSize(price.getSize());
        m_middle.setSize(price.getSize());
        m_lower.setSize(price.getSize());
    }

    int b, n;
    TA_RetCode res = ::TA_BBANDS(0, price.getSize()-1, price.getData(), m_len,
                                 m_numDevUp, m_numDevDn,
                                 static_cast<TA_MAType>(m_maType),
                                 &b, &n, m_upper.getData()+lb, m_middle.getData()+lb, m_lower.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_lastUpper = m_upper.getLast();
    m_lastMiddle = m_middle.getLast();
    m_lastLower = m_lower.getLast();

    done(timestamp);
}

o3d::Int32 Bollinger::lookback() const
{
    return ::TA_BBANDS_Lookback(m_len, m_numDevUp, m_numDevDn, static_cast<TA_MAType>(m_maType));
}
