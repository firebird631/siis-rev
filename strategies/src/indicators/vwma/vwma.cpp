/**
 * @brief SiiS volume weighted moving average indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/vwma/vwma.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Vwma::Vwma(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len) :
    Indicator(name, timeframe),
    m_len(len),
    m_prev(0.0),
    m_last(0.0)
{
}

Vwma::Vwma(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_len(0),
    m_prev(0.0),
    m_last(0.0)
{
    m_len = conf.data().get("len", 20).asInt();
}

void Vwma::setConf(IndicatorConfig conf)
{
    m_len = conf.data().get("len", 20).asInt();
}

void Vwma::compute(o3d::Double timestamp, const DataArray &price, const DataArray &volume)
{
    m_prev = m_last;

    o3d::Int32 size = price.getSize();

    if (m_vwma.getSize() != size) {
        m_vwma.setSize(size);

        m_tmp1.setSize(size);
        m_volume.setSize(size);
        m_pvs.setSize(size);
        m_vs.setSize(size);
    }

    // cannot deal with zero volume, then set it to 1 will have no effect on the result, juste give a price
    for (o3d::Int32 i = 0; i < size; ++i) {
        m_volume[i] = volume[i] > 0.0 ? volume[i] : 1.0;
    }

    // SMA(price * volume, N) => pvs
    m_tmp1.mult(price, m_volume);
    m_pvs.sma(m_len, m_tmp1);

    // SMA(volume, N) => vs
    m_vs.sma(m_len, m_volume);

    m_vwma.div(m_pvs, m_vs);

    m_last = m_vwma.getLast();
    done(timestamp);
}
