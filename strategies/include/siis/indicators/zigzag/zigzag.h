/**
 * @brief SiiS ZigZag indicator
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-08-17
 */

#ifndef SIIS_ZIGZAG_H
#define SIIS_ZIGZAG_H

#include "../indicator.h"
#include "../../constants.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief ZigZag indicator.
 * @author Frederic Scherma
 * @date 2019-08-17
 */
class SIIS_API ZigZag : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OVERLAY

    ZigZag(const o3d::String &name,
           o3d::Double timeframe,
           o3d::Double threshold);
    ZigZag(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Double threshold() const { return m_threshold; }

    const DataArray& lower() const { return m_lower; }
    const DataArray& higher() const { return m_higher; }

    /**
     * @brief compute Compute an Hilbert sine wave.
     * @param open Open array of price.
     * @param open High array of price.
     * @param open Low array of price.
     * @param open Close array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &open, const DataArray &high, const DataArray &low, const DataArray &close);

private:

    o3d::Double m_threshold;

    DataArray m_lower;
    DataArray m_higher;
};

} // namespace siis

#endif // SIIS_ZIGZAG_H
