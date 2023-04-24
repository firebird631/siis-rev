/**
 * @brief SiiS directional moving index indicator.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_DMI_H
#define SIIS_DMI_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS directional moving index indicator.
 * @author Frederic Scherma
 * @date 2023-04-24
 */
class SIIS_API Dmi : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OSCILLATOR

    Dmi(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=14);
    Dmi(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }

    const DataArray& dmi_m() const { return m_dmi_m; }
    const DataArray& dmi_p() const { return m_dmi_p; }

    o3d::Double last_m() const { return m_last_m; }
    o3d::Double prev_m() const { return m_prev_m; }

    o3d::Double last_p() const { return m_last_p; }
    o3d::Double prev_p() const { return m_prev_p; }

    /**
     * @brief compute Compute an ADX.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_len;
    DataArray m_dmi_p;
    DataArray m_dmi_m;

    o3d::Double m_prev_m;
    o3d::Double m_last_m;

    o3d::Double m_prev_p;
    o3d::Double m_last_p;
};

} // namespace siis

#endif // SIIS_DMI_H
