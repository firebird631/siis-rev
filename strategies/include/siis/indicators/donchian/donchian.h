/**
 * @brief SiiS donchian channels indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_DONCHIAN_H
#define SIIS_DONCHIAN_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS simple moving average indicator.
 * @author Frederic Scherma
 * @date 2019-03-15
 */
class SIIS_API Donchian : public Indicator
{
public:

    // TYPE_VOLATILITY
    // CLS_INDEX

    Donchian(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=14);
    Donchian(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }

    const DataArray& upper() const { return m_upper; }
    const DataArray& lower() const { return m_lower; }

    o3d::Double lastUpper() const { return m_lastUpper; }
    o3d::Double prevUpper() const { return m_prevUpper; }

    o3d::Double lastLower() const { return m_lastLower; }
    o3d::Double prevLower() const { return m_prevLower; }

    /**
     * @brief compute Compute a SMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &high, const DataArray &low);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_len;
    DataArray m_upper;
    DataArray m_lower;

    o3d::Double m_prevUpper;
    o3d::Double m_lastUpper;

    o3d::Double m_prevLower;
    o3d::Double m_lastLower;
};

} // namespace siis

#endif // SIIS_DONCHIAN_H
