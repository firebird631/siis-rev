/**
 * @brief SiiS hull moving average indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/hma/hma.h"
#include "siis/utils/common.h"

#include <o3d/core/math.h>

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Hma::Hma(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len) :
    Indicator (name, timeframe),
    m_len(len),
    m_prev(0.0),
    m_last(0.0)
{
}

Hma::Hma(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0),
    m_prev(0.0),
    m_last(0.0)
{
    m_len = conf.data().get("len", 9).asInt();
}

void Hma::setConf(IndicatorConfig conf)
{
    m_len = conf.data().get("len", 9).asInt();
}

void Hma::compute(o3d::Double timestamp, const DataArray &price)
{
    o3d::Int32 lb = lookback();
    if (price.getSize() <= lb) {
        return;
    }

    m_prev = m_last;

    if (m_hma.getSize() != price.getSize()) {
        m_hma.setSize(price.getSize());
    }

    o3d::Int32 N = m_len;
    o3d::Int32 N_2 = static_cast<o3d::Int32>(m_len*0.5);
    o3d::Int32 N_sqrt = static_cast<o3d::Int32>(o3d::Math::sqrt(m_len));

    o3d::Int32 size = price.getSize();

    if (m_weights.getSize() != size) {
        m_weights.setSize(size);

        m_pv.setSize(size);
        m_tmp1.setSize(size);
        m_tmp2.setSize(size);
        m_tmp3.setSize(size);
    }

    for (o3d::Int32 i = 0; i < size; ++i) {
        // weight arr(1..n)
        m_weights[i] = i+1;
    }

    m_pv.mult(price, m_weights);    // price*weights

    //
    // #1 calculate a WMA with period n / 2 and multiply it by 2
    // hma12 = 2 * SMA(data*weights, N_2) / SMA(weights, N_2)
    //
    // m_pv.sma(N_2, m_tmp2); m_tmp2 *= 2.0;  // 2 * SMA(data*weights, N_2) => tmp2
    // m_weights.sma(N_2, m_tmp3);      // SMA(weights, N_2) => tmp3
    // m_hma12.div(m_tmp2, m_tmp3);

    // or... depends if SIMD optimization (3 loops vs 2 loops). maybe exists SIMD : a = 2.0 * b / c
    m_pv.sma(N_2, m_tmp2);
    m_tmp3.sma(N_2, m_weights);

    for (o3d::Int32 i = 0; i < size; ++i) {
       m_hma12[i] = 2.0 * m_tmp2[i] / m_tmp3[i];
    }

    //
    // #2 calculate a WMA for period n and subtract if from step 1
    // hma12 = hma12 - (SMA(data*weights, N) / SMA(weights, N))
    //

    // m_pv.sma(N, m_tmp2);       // SMA(data*weights, N) => tmp2
    // m_weights.sma(N, m_tmp3);  // SMA(weights, N) => tmp3

    // m_tmp1.div(m_tmp2, m_tmp3);  // SMA(data*weights, N) / SMA(weights, N))
    // m_hma12 -= m_tmp1;

    // or... depends if SIMD optimization (2 loops vs 1 loops). maybe exists SIMD : a -= b / c
    m_pv.sma(N, m_tmp2);       // SMA(data*weights, N) => tmp2
    m_weights.sma(N, m_tmp3);  // SMA(weights, N) => tmp3

    for (o3d::Int32 i = 0; i < size; ++i) {
       m_hma12[i] -= m_tmp2[i] / m_tmp3[i];
    }

    //
    // #3 calculate a WMA with period sqrt(n) using the data from step 2
    // m_hma = (SMA(hma12*weights, N_sqrt) / SMA(weights, N_sqrt))
    //

    m_tmp1.mult(m_hma12, m_weights);  // hma12*weights => tmp1
    m_tmp1.sma(N_sqrt, m_tmp2);       // SMA(hma12*weights, N_sqrt) => tmp2

    m_weights.sma(N_sqrt, m_tmp3);    // SMA(weights, N_sqrt) => tmp3
    m_hma.div(m_tmp2, m_tmp3);

    m_last = m_hma.getLast();
    done(timestamp);
}

o3d::Int32 Hma::lookback() const
{
    return m_len-1;  // ::TA_SMA_Lookback(m_len);
}
