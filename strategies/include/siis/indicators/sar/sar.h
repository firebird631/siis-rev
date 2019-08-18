/**
 * @brief SiiS stop and reverse indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_SAR_H
#define SIIS_SAR_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS stop and reverse indicator.
 * @author Frederic Scherma
 * @date 2019-03-15
 */
class SIIS_API Sar : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OSCILLATOR

    Sar(const o3d::String &name, o3d::Double timeframe, o3d::Double accel=0.0, o3d::Double max=0.0);
    Sar(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Double accel() const { return m_accel; }
    o3d::Double max() const { return m_max; }

    const DataArray& sar() const { return m_sar; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    /**
     * @brief compute Compute a SAR.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &high, const DataArray &low);

    /**
     * @brief lookback Min number of necessary samples.
     * @return 1
     */
    o3d::Int32 lookback() const;

private:

    o3d::Double m_accel;
    o3d::Double m_max;
    DataArray m_sar;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_SAR_H
