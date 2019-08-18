/**
 * @brief SiiS hull moving average indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_HMA_H
#define SIIS_HMA_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS hull moving average indicator.
 * @author Frederic Scherma
 * @date 2019-03-15
 * @ref https://www.fidelity.com/learning-center/trading-investing/technical-analysis/technical-indicator-guide/hull-moving-average
 */
class SIIS_API Hma : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OVERLAY

    Hma(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=9);
    Hma(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }

    const DataArray& hma() const { return m_hma; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    /**
     * @brief compute Compute a SMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price);
    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_len;
    DataArray m_hma;

    o3d::Double m_prev;
    o3d::Double m_last;

    DataArray m_weights;
    DataArray m_hma12;

    DataArray m_pv;
    DataArray m_tmp1;
    DataArray m_tmp2;
    DataArray m_tmp3;
};

} // namespace siis

#endif // SIIS_HMA_H
