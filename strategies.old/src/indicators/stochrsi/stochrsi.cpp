/**
 * @brief SiiS stochastic rsi indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/stochrsi/stochrsi.h"
#include "siis/utils/common.h"
#include "siis/constants.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

StochRsi::StochRsi(const o3d::String &name, o3d::Double timeframe,
                   o3d::Int32 len, o3d::Int32 fastK_Len, o3d::Int32 fastD_Len, MAType fastD_MAType) :
    Indicator(name, timeframe),
    m_len(len),
    m_fastK_Len(fastK_Len),
    m_fastD_Len(fastD_Len),
    m_fastD_MAType(fastD_MAType),
    m_prevFastK(0.0),
    m_lastFastK(0.0),
    m_prevFastD(0.0),
    m_lastFastD(0.0)
{
}

StochRsi::StochRsi(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_len(0),
    m_fastK_Len(0),
    m_fastD_Len(0),
    m_fastD_MAType(MA_SMA),
    m_prevFastK(0.0),
    m_lastFastK(0.0),
    m_prevFastD(0.0),
    m_lastFastD(0.0)
{
    m_len = conf.data().get("len", 21).asInt();

    m_fastK_Len = conf.data().get("fastK_Len", 12).asInt();
    m_fastD_Len = conf.data().get("fastD_Len", 9).asInt();
    m_fastD_MAType = siis::maTypeFromStr(conf.data().get("fastD_MAType", "SMA").asCString());
}

void StochRsi::setConf(IndicatorConfig conf)
{
    m_len = conf.data().get("len", 21).asInt();

    m_fastK_Len = conf.data().get("fastK_Len", 12).asInt();
    m_fastD_Len = conf.data().get("fastD_Len", 9).asInt();
    m_fastD_MAType = siis::maTypeFromStr(conf.data().get("fastD_MAType", "SMA").asCString());
}

void StochRsi::compute(o3d::Double timestamp, const DataArray &price)
{
    m_prevFastK = m_lastFastK;
    m_prevFastD = m_lastFastD;

    o3d::Int32 size = price.getSize();

    if (m_fastK.getSize() != size) {
        m_fastK.setSize(size);
        m_fastD.setSize(size);
    }

    int b, n;
    TA_RetCode res = ::TA_STOCHRSI(0, price.getSize()-1, price.getData(),
                                   m_len, m_fastK_Len, m_fastD_Len, static_cast<TA_MAType>(m_fastD_MAType),
                                   &b, &n, m_fastK.getData()+m_len, m_fastD.getData()+m_len);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == m_len);  // @todo not clean which one

    m_lastFastK = m_fastK.getLast();
    m_lastFastD = m_fastD.getLast();
    done(timestamp);
}
