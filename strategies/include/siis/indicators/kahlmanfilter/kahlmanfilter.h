/**
 * @brief SiiS Kahlman filter indicator (that can be used over another indicator ouput).
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#ifndef SIIS_KAHLMANFILTER_H
#define SIIS_KAHLMANFILTER_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS Kahlman filter indicator (that can be used over another indicator ouput).
 * @author Frederic Scherma
 * @date 2023-09-30
 */
class SIIS_API KahlmanFilter : public Indicator
{
public:

    // TYPE_FILTER
    // CLS_FILTER

    KahlmanFilter(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=7, o3d::Double gain=0.7);
    KahlmanFilter(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }

    const DataArray& kf() const { return m_kf; }

    o3d::Double prev() const { return m_prev; }
    o3d::Double last() const { return m_last; }

    void resize(o3d::Int32 len);

    /**
     * @brief compute Compute a Kahlman filter.
     * @param price A close or any other array of price or indicator output.
     */
    void compute(o3d::Double timestamp, const DataArray &price);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_len;
    o3d::Double m_gain;
    o3d::Double m_g2Sqrt;

    DataArray m_kf;
    DataArray m_dk;
    DataArray m_smooth;
    DataArray m_velo;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_KAHLMANFILTER_H
