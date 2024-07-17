/**
 * @brief SiiS Ichimoku indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#include "siis/indicators/ichimoku/ichimoku.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Ichimoku::Ichimoku(const o3d::String &name,
                   o3d::Double timeframe,
                   o3d::Int32 tenkanLen,
                   o3d::Int32 kijunLen,
                   o3d::Int32 senkouSpanBLen) :
    Indicator (name, timeframe),
    m_tenkanLen(tenkanLen),
    m_kijunLen(kijunLen),
    m_senkouSpanBLen(senkouSpanBLen),
    m_prevTenkan(0.0),
    m_prevKijun(0.0),
    m_prevSsa(0.0),
    m_prevSsb(0.0),
    m_prevChikou(0.0),
    m_lastTenkan(0.0),
    m_lastKijun(0.0),
    m_lastSsa(0.0),
    m_lastSsb(0.0),
    m_lastChikou(0.0)
{

}

Ichimoku::Ichimoku(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_tenkanLen(9),
    m_kijunLen(26),
    m_senkouSpanBLen(52),
    m_prevTenkan(0.0),
    m_prevKijun(0.0),
    m_prevSsa(0.0),
    m_prevSsb(0.0),
    m_prevChikou(0.0),
    m_lastTenkan(0.0),
    m_lastKijun(0.0),
    m_lastSsa(0.0),
    m_lastSsb(0.0),
    m_lastChikou(0.0)
{
    if (conf.data().isObject()) {
        m_tenkanLen = conf.data().get("tenkanLen", 9).asInt();
        m_kijunLen = conf.data().get("kijunLen", 26).asInt();
        m_senkouSpanBLen = conf.data().get("senkouSpanBLen", 52).asInt();
    } else if (conf.data().isArray()) {
        m_tenkanLen = conf.data().get((Json::ArrayIndex)1, 9).asInt();
        m_kijunLen = conf.data().get((Json::ArrayIndex)2, 26).asInt();
        m_senkouSpanBLen = conf.data().get((Json::ArrayIndex)3, 52).asInt();
    }
}

void Ichimoku::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_tenkanLen = conf.data().get("tenkanLen", 9).asInt();
        m_kijunLen = conf.data().get("kijunLen", 26).asInt();
        m_senkouSpanBLen = conf.data().get("senkouSpanBLen", 52).asInt();
    } else if (conf.data().isArray()) {
        m_tenkanLen = conf.data().get((Json::ArrayIndex)1, 9).asInt();
        m_kijunLen = conf.data().get((Json::ArrayIndex)2, 26).asInt();
        m_senkouSpanBLen = conf.data().get((Json::ArrayIndex)3, 52).asInt();
    }
}

void Ichimoku::compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close)
{
    o3d::Int32 lb = lookback();
    if (close.getSize() <= lb) {
        return;
    }

    m_prevTenkan = m_lastTenkan;
    m_prevKijun = m_lastKijun;
    m_prevSsa = m_lastSsa;
    m_prevSsb = m_lastSsb;
    m_prevChikou = m_lastChikou;

    // tenkan-sen - conversion line (window of 9)
    if (m_tenkan.getSize() != close.getSize()) {
        m_tenkan.setSize(close.getSize());
        m_kijun.setSize(close.getSize());
        m_ssa.setSize(close.getSize());
        m_ssb.setSize(close.getSize());
        m_chikou.setSize(close.getSize());
    }

    lb = ::TA_MIDPRICE_Lookback(m_tenkanLen);
    int b, n;
    TA_RetCode res = ::TA_MIDPRICE(0, close.getSize()-1, high.getData(), low.getData(), m_tenkanLen, &b, &n, m_tenkan.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    // kijun-sen - base-line (window of 9)
    lb = ::TA_MIDPRICE_Lookback(m_kijunLen);
    res = ::TA_MIDPRICE(0, close.getSize()-1, high.getData(), low.getData(), m_kijunLen, &b, &n, m_kijun.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    // senkou span A - leading span A

    // must be considered as shifted in future (26)
    m_ssa = (m_tenkan + m_kijun) * 0.5;

    // senkou span B - leading span B
    lb = ::TA_MIDPRICE_Lookback(m_senkouSpanBLen);
    res = ::TA_MIDPRICE(0, close.getSize()-1, high.getData(), low.getData(), m_senkouSpanBLen, &b, &n, m_ssb.getData()+lb);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == lb);

    m_lastTenkan = m_tenkan.last();
    m_lastKijun = m_kijun.last();
    m_lastSsa = m_ssa.last();
    m_lastSsb = m_ssb.last();
    m_lastChikou = m_chikou.last();

    done(timestamp);
}

o3d::Int32 Ichimoku::lookback() const
{
    // @todo
    return m_senkouSpanBLen - 1 + m_lastTenkan - 1;
}
