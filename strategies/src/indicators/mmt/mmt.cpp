/**
 * @brief SiiS momentum indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/mmt/mmt.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Mmt::Mmt(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len) :
    Indicator(name, timeframe),
    m_len(len),
    m_prev(0.0),
    m_last(0.0)
{
}

Mmt::Mmt(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_len(0),
    m_prev(0.0),
    m_last(0.0)
{
    m_len = conf.data().get("len", 20).asInt();
}

void Mmt::setConf(IndicatorConfig conf)
{
    m_len = conf.data().get("len", 20).asInt();
}

void Mmt::compute(o3d::Double timestamp, const DataArray &price)
{
    o3d::Int32 lb = lookback();
    if (price.getSize() <= lb) {
        return;
    }

    m_prev = m_last;

    if (m_mmt.getSize() != price.getSize()) {
        m_mmt.setSize(price.getSize());
    }

    int b, n;  // first len data are empty so add offset
    TA_RetCode res = ::TA_MOM(0, price.getSize()-1, price.getData(), m_len, &b, &n, m_mmt.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_last = m_mmt.getLast();
    done(timestamp);
}

o3d::Int32 Mmt::lookback() const
{
    return m_len;  // ::TA_MOM_Lookback(m_len);
}
