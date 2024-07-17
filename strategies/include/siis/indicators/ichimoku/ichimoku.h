/**
 * @brief SiiS Ichimoku indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#ifndef SIIS_ICHIMOKU_H
#define SIIS_ICHIMOKU_H

#include "../indicator.h"
#include "../../constants.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS Ichimoku indicator.
 * @author Frederic Scherma
 * @date 2024-07-15
 */
class SIIS_API Ichimoku : public Indicator
{
public:

    // TYPE_MOMENTUM_SUPPORT_RESISTANCE_TREND
    // CLS_INDEX

    Ichimoku(const o3d::String &name,
           o3d::Double timeframe,
           o3d::Int32 tenkanLen=9,
           o3d::Int32 kijunLen=26,
           o3d::Int32 senkouSpanBLen=52);
    Ichimoku(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 tenkanLen() const { return m_tenkanLen; }
    o3d::Int32 kijunLen() const { return m_kijunLen; }
    o3d::Int32 senkouSpanBLen() const { return m_senkouSpanBLen; }

    const DataArray& tenkan() const { return m_tenkan; }
    const DataArray& kijun() const { return m_kijun; }
    const DataArray& ssa() const { return m_ssa; }
    const DataArray& ssb() const { return m_ssb; }
    const DataArray& chikou() const { return m_chikou; }

    o3d::Double lastTenkan() const { return m_lastTenkan; }
    o3d::Double prevTenkan() const { return m_prevTenkan; }

    o3d::Double lastKijun() const { return m_lastKijun; }
    o3d::Double prevKijun() const { return m_prevKijun; }

    o3d::Double lastSsa() const { return m_lastSsa; }
    o3d::Double prevSsa() const { return m_prevSsa; }

    o3d::Double lastSsb() const { return m_lastSsb; }
    o3d::Double prevSsb() const { return m_prevSsb; }

    o3d::Double lastChikou() const { return m_lastChikou; }
    o3d::Double prevChikou() const { return m_prevChikou; }

    /**
     * @brief compute Compute an Ichimoku.
     * @param high High prices.
     * @param low Low prices.
     * @param close Close prices.
     */
    void compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_tenkanLen;
    o3d::Int32 m_kijunLen;
    o3d::Int32 m_senkouSpanBLen;

    DataArray m_tenkan;
    DataArray m_kijun;
    DataArray m_ssa;
    DataArray m_ssb;
    DataArray m_chikou;

    o3d::Double m_prevTenkan;
    o3d::Double m_prevKijun;
    o3d::Double m_prevSsa;
    o3d::Double m_prevSsb;
    o3d::Double m_prevChikou;

    o3d::Double m_lastTenkan;
    o3d::Double m_lastKijun;
    o3d::Double m_lastSsa;
    o3d::Double m_lastSsb;
    o3d::Double m_lastChikou;
};

} // namespace siis

#endif // SIIS_ICHIMOKU_H
