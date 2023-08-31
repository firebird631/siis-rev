/**
 * @brief SiiS hull moving average three indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#include "siis/indicators/hma3/hma3.h"
#include "siis/utils/common.h"

#include <o3d/core/math.h>

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Hma3::Hma3(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len) :
    Indicator (name, timeframe),
    m_len(len),
    m_prev(0.0),
    m_last(0.0)
{
}

Hma3::Hma3(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_len(0),
    m_prev(0.0),
    m_last(0.0)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 9).asInt();
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 9).asInt();
    }
}

void Hma3::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_len = conf.data().get("len", 9).asInt();
        // o3d::System::print(o3d::String("{} {0} {1}").arg(name()).arg(m_len), "");
    } else if (conf.data().isArray()) {
        m_len = conf.data().get((Json::ArrayIndex)1, 9).asInt();
        // o3d::System::print(o3d::String("[] {0} {1}").arg(name()).arg(m_len), "");
    }
}

void Hma3::compute(o3d::Double timestamp, const DataArray &price)
{
    // this version is 2x fast than below
    o3d::Int32 lb = lookback();
    if (price.getSize() <= lb) {
        return;
    }

    m_prev = m_last;

    if (m_hma3.getSize() != price.getSize()) {
        m_hma3.setSize(price.getSize());
    }

    o3d::Int32 N = o3d::max(2, m_len);
    o3d::Int32 N_2 = o3d::max(2, static_cast<o3d::Int32>(m_len*0.5));
    o3d::Int32 N_3 = o3d::max(2, static_cast<o3d::Int32>(m_len*0.333333334));

    o3d::Int32 size = price.getSize();

    if (m_tmp1.getSize() != size) {
        m_hma12.setSize(size);
        m_tmp1.setSize(size);
    }

    int b, n;
    lb = N_3 - 1;
    TA_RetCode res = ::TA_WMA(0, price.getSize()-1, price.getData(), N_3, &b, &n, m_hma12.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_hma12 *= 3;

    lb = N_2 - 1;
    res = ::TA_WMA(0, price.getSize()-1, price.getData(), N_2, &b, &n, m_tmp1.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_hma12 -= m_tmp1;
    for (o3d::Int32 i = 0; i < N_2-1; ++i) {
       m_hma12[i] = m_hma12[N_2-1];
    }

    lb = N - 1;
    res = ::TA_WMA(0, price.getSize()-1, price.getData(), N, &b, &n, m_tmp1.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_hma12 -= m_tmp1;
    for (o3d::Int32 i = 0; i < N-1; ++i) {
       m_hma12[i] = m_hma12[N-1];
    }

    lb = N - 1;
    res = ::TA_WMA(0, m_hma12.getSize()-1, m_hma12.getData(), N, &b, &n, m_hma3.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_hma3.nan((N-1) + (N-1));

//    printf("->%i # %i %i %i\n", price.getSize(), N, N_2, N_sqrt);
//    for (int i = 0; i < m_hma.getSize(); ++i) {
//        //printf("(%i) %f/ %f/ %f/ %f, ", N, price[i], m_hma12[i], m_tmp1[i], m_hma[i]);
//        printf("%f, ", N, m_hma3[i]);
//    }
//    printf("\n");}

    m_last = m_hma3.getLast();
    done(timestamp);
}

o3d::Int32 Hma3::lookback() const
{
//    o3d::Int32 N_2 = static_cast<o3d::Int32>(o3d::max(2, static_cast<o3d::Int32>(m_len*0.5));
//    o3d::Int32 N_3 = static_cast<o3d::Int32>(o3d::max(2, static_cast<o3d::Int32>(m_len*0.333333334));
//    return m_len-1 + N_2-1 + N_3-1;  // ::TA_SMA_Lookback(m_len) + ::TA_SMA_Lookback(N_2) + ::TA_SMA_Lookback(N_3);
    return m_len-1 + m_len-1;
}
