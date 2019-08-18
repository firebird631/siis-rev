/**
 * @brief SiiS stochastic indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/stoch/stoch.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Stoch::Stoch(const o3d::String &name, o3d::Double timeframe,
             o3d::Int32 fastK_Len, o3d::Int32 slowK_Len, MAType slowK_MAType,
             o3d::Int32 slowD_Len, MAType slowD_MAType) :
    Indicator(name, timeframe),
    m_fastK_Len(fastK_Len),
    m_slowK_Len(slowK_Len),
    m_slowK_MAType(slowK_MAType),
    m_slowD_Len(slowD_Len),
    m_slowD_MAType(slowD_MAType),
    m_prevSlowK(0.0),
    m_lastSlowK(0.0),
    m_prevSlowD(0.0),
    m_lastSlowD(0.0)
{
}

Stoch::Stoch(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_fastK_Len(0),
    m_slowK_Len(0),
    m_slowK_MAType(MA_SMA),
    m_slowD_Len(0),
    m_slowD_MAType(MA_SMA),
    m_prevSlowK(0.0),
    m_lastSlowK(0.0),
    m_prevSlowD(0.0),
    m_lastSlowD(0.0)
{
    m_fastK_Len = conf.data().get("fastK_Len", 9).asInt();
    m_slowK_Len = conf.data().get("slowK_Len", 12).asInt();
    m_slowK_MAType = siis::maTypeFromStr(conf.data().get("slowK_MAType", "SMA").asCString());

    m_slowD_Len = conf.data().get("slowD_Len", 3).asInt();
    m_slowD_MAType = siis::maTypeFromStr(conf.data().get("slowD_MAType", "SMA").asCString());
}

void Stoch::setConf(IndicatorConfig conf)
{
    m_fastK_Len = conf.data().get("fastK_Len", 9).asInt();
    m_slowK_Len = conf.data().get("slowK_Len", 12).asInt();
    m_slowK_MAType = siis::maTypeFromStr(conf.data().get("slowK_MAType", "SMA").asCString());

    m_slowD_Len = conf.data().get("slowD_Len", 3).asInt();
    m_slowD_MAType = siis::maTypeFromStr(conf.data().get("slowD_MAType", "SMA").asCString());
}

void Stoch::compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close)
{
    o3d::Int32 lb = lookback();
    if (high.getSize() <= lb) {
        return;
    }

    m_prevSlowK = m_lastSlowK;
    m_prevSlowD = m_lastSlowD;

    o3d::Int32 size = high.getSize();

    if (m_slowK.getSize() != size) {
        m_slowK.setSize(size);
        m_slowD.setSize(size);
    }

    int b, n;  // first len data are empty so add offset
    TA_RetCode res = ::TA_STOCH(0, high.getSize()-1, high.getData(), low.getData(), close.getData(),
                                m_fastK_Len, m_slowK_Len, static_cast<TA_MAType>(m_slowK_MAType),
                                m_slowD_Len, static_cast<TA_MAType>(m_slowD_MAType),
                                &b, &n, m_slowK.getData()+lb, m_slowD.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_lastSlowK = m_slowK.getLast();
    m_lastSlowD = m_slowD.getLast();
    done(timestamp);
}

o3d::Int32 Stoch::lookback() const
{
    return ::TA_STOCH_Lookback(m_fastK_Len, m_slowK_Len,static_cast<TA_MAType>(m_slowK_MAType),
                                m_slowD_Len, static_cast<TA_MAType>(m_slowD_MAType));
}
