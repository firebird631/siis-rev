/**
 * @brief SiiS bollinger bands indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_BBANDS_H
#define SIIS_BBANDS_H

#include "../indicator.h"
#include "../../constants.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS bollinger bands indicator.
 * @author Frederic Scherma
 * @date 2019-03-15
 */
class SIIS_API BBands : public Indicator
{
public:

    // TYPE_VOLATILITY
    // CLS_INDEX

    BBands(const o3d::String &name,
           o3d::Double timeframe,
           o3d::Int32 len,
           MAType maType=MA_SMA,
           o3d::Double numDevUp=0.0,
           o3d::Double numDevDn=0.0);
    BBands(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }
    MAType maType() const { return m_maType; }
    o3d::Double numDevUp() const { return m_numDevUp; }
    o3d::Double numDevDn() const { return m_numDevDn; }

    const DataArray& upper() const { return m_upper; }
    const DataArray& middle() const { return m_middle; }
    const DataArray& lower() const { return m_lower; }

    o3d::Double lastUpper() const { return m_lastUpper; }
    o3d::Double prevUpper() const { return m_prevUpper; }

    o3d::Double lastLower() const { return m_lastLower; }
    o3d::Double prevLower() const { return m_prevLower; }

    o3d::Double lastMiddle() const { return m_lastMiddle; }
    o3d::Double prevMiddle() const { return m_prevMiddle; }

    /**
     * @brief compute Compute a SMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price);

private:

    o3d::Int32 m_len;
    MAType m_maType;
    o3d::Double m_numDevUp;
    o3d::Double m_numDevDn;

    DataArray m_upper;
    DataArray m_middle;
    DataArray m_lower;

    o3d::Double m_prevUpper;
    o3d::Double m_lastUpper;

    o3d::Double m_prevMiddle;
    o3d::Double m_lastMiddle;

    o3d::Double m_prevLower;
    o3d::Double m_lastLower;
};

} // namespace siis

#endif // SIIS_BBANDS_H
