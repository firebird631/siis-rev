/**
 * @brief SiiS volume weighted moving average indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_VWMA_H
#define SIIS_VWMA_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS volume weighted moving average indicator.
 * @author Frederic Scherma
 * @date 2019-03-19
 */
class SIIS_API Vwma : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OVERLAY

    Vwma(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=9);
    Vwma(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }

    const DataArray& sma() const { return m_vwma; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    /**
     * @brief compute Compute a VWMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price, const DataArray &volume);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_len;
    DataArray m_vwma;

    o3d::Double m_prev;
    o3d::Double m_last;

    DataArray m_tmp1;
    DataArray m_volume;
    DataArray m_pvs;
    DataArray m_vs;
};

} // namespace siis

#endif // SIIS_VWMA_H
