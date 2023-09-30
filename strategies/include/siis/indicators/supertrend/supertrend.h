/**
 * @brief SiiS Supertrend indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#ifndef SIIS_SUPERTREND_H
#define SIIS_SUPERTREND_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS Supertrend indicator.
 * @author Frederic Scherma
 * @date 2023-09-30
 */
class SIIS_API SuperTrend : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OSCILLATOR

    SuperTrend(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=14, o3d::Double coeff=3.0);
    SuperTrend(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }
    o3d::Double coeff() const { return m_coeff; }

    const DataArray& trend() const { return m_trend; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    o3d::Int32 position() const;
    o3d::Int32 positionChange() const;

    /**
     * @brief compute Compute a SuperTrend.
     */
    void compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_len;
    o3d::Double m_coeff;

    DataArray m_hl2;
    DataArray m_catr;
    DataArray m_upper;
    DataArray m_lower;

    DataArray m_trend;
    DataArray m_position;
    DataArray m_up;
    DataArray m_down;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_SUPERTREND_H
