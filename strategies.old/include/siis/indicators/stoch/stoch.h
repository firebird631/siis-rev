/**
 * @brief SiiS stochastic indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_STOCH_H
#define SIIS_STOCH_H

#include "../indicator.h"
#include "../../dataarray.h"
#include "../../constants.h"

namespace siis {

/**
 * @brief SiiS stochastic indicator.
 * @author Frederic Scherma
 * @date 2019-03-19
 */
class SIIS_API Stoch : public Indicator
{
public:

    // TYPE_MOMENTUM
    // CLS_OSCILLATOR

    Stoch(const o3d::String &name, o3d::Double timeframe,
          o3d::Int32 fastK_Len=9, o3d::Int32 slowK_Len=12, MAType slowK_MAType=MA_SMA,
          o3d::Int32 slowD_Len=3, MAType slowD_MAType=MA_SMA);
    Stoch(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 fastK_Len() const { return m_fastK_Len; }
    o3d::Int32 slowK_Len() const { return m_slowK_Len; }
    MAType slowK_MAType() const { return m_slowK_MAType; }

    o3d::Int32 slowD_Len() const { return m_slowD_Len; }
    MAType slowD_MAType() const { return m_slowD_MAType; }

    const DataArray& slowK() const { return m_slowK; }
    const DataArray& slowD() const { return m_slowD; }

    o3d::Double lastSlowK() const { return m_lastSlowK; }
    o3d::Double prevSlowK() const { return m_prevSlowK; }

    o3d::Double lastSlowD() const { return m_lastSlowD; }
    o3d::Double prevSlowD() const { return m_prevSlowD; }

    /**
     * @brief compute Compute a VWMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close);

private:

    o3d::Int32 m_fastK_Len;
    o3d::Int32 m_slowK_Len;
    MAType m_slowK_MAType;

    o3d::Int32 m_slowD_Len;
    MAType m_slowD_MAType;

    DataArray m_slowK;
    DataArray m_slowD;

    o3d::Double m_prevSlowK;
    o3d::Double m_lastSlowK;

    o3d::Double m_prevSlowD;
    o3d::Double m_lastSlowD;
};

} // namespace siis

#endif // SIIS_STOCH_H
