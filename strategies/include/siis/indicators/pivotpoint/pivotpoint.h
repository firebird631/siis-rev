/**
 * @brief SiiS pivot point indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_PIVOTPOINT_H
#define SIIS_PIVOTPOINT_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS pivot point indicator.
 * @author Frederic Scherma
 * @date 2019-03-19
 * @ref https://www.fidelity.com/learning-center/trading-investing/technical-analysis/technical-indicator-guide/pivot-points-resistance-support
 * @ref https://www.andlil.com/les-points-pivots-dans-le-trading-430.html
 * @ref https://www.andlil.com/points-pivots-de-camarilla-440.html
 * @ref https://www.andlil.com/point-pivot-de-woodie-445.html
 * Supports and resistances detection using pivot point method.
 *
 * Classical:
 * Pivot = (H + B + C) / 3
 * S1 = (2 x Pivot) - H
 * S2 = Pivot - (H - B)
 * S3 = B - 2x (H - Pivot)
 * R1 = (2 x Pivot) - B
 * R2 = Pivot + (H - B)
 * R3 = H + 2x (Pivot - B)
 *
 * Camarilla:
 * Pivot = C
 * S1 = C - (H - L) * 1.1/12
 * S2 = C - (H - L) * 1.1/6
 * S3 = C - (H - L) * 1.1/4
 * R3 = C + (H - L) * 1.1/4
 * R2 = C + (H - L) * 1.1/6
 * R1 = C + (H - L) * 1.1/12
 *
 * Woodie:
 * Pivot = (H + L + 2 x C) / 4
 * Like as classicial
 */
class SIIS_API PivotPoint : public Indicator
{
public:

    // TYPE_VOLATILITY
    // CLS_OVERLAY

    enum Method {
        CLASSICAL = 0,
        CLASSICAL_OHLC = 1,
        CLASSICAL_OHL = 2,
        CAMARILLA = 3,
        WOODIE = 4
    };

    PivotPoint(const o3d::String &name, o3d::Double timeframe, Method method=CLASSICAL);
    PivotPoint(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    Method method() const { return m_method; }

    const DataArray& pivot() const { return m_pivot; }

    const DataArray& support(o3d::Int32 i) const;
    const DataArray& resistance(o3d::Int32 i) const;

    /**
     * @brief compute Compute a SMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp,
                 const DataArray &open, const DataArray &high,
                 const DataArray &low, const DataArray &close);

    /**
     * @brief lookback Min number of necessary samples.
     * @return 1
     */
    o3d::Int32 lookback() const;

private:

    Method m_method;

    DataArray m_tmp1;

    DataArray m_pivot;
    DataArray m_support[3];
    DataArray m_resistance[3];
};

} // namespace siis

#endif // SIIS_PIVOTPOINT_H
