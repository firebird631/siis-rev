/**
 * @brief SiiS hull moving average three indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#ifndef SIIS_HMA3_H
#define SIIS_HMA3_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS hull moving average three indicator.
 * @author Frederic Scherma
 * @date 2019-03-15
 * @ref https://www.fidelity.com/learning-center/trading-investing/technical-analysis/technical-indicator-guide/hull-moving-average
 * It is a variation with : hma3 = wma(wma(x, len/3)*3 - wma(x, len/2) - wma(x, len), len)
 */
class SIIS_API Hma3 : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OVERLAY

    Hma3(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=9);
    Hma3(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }

    const DataArray& hma3() const { return m_hma3; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    /**
     * @brief compute Compute a HMA3.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price);
    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_len;
    DataArray m_hma3;

    o3d::Double m_prev;
    o3d::Double m_last;

    DataArray m_hma12;
    DataArray m_tmp1;
};

} // namespace siis

#endif // SIIS_HMA3_H
