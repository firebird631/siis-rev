/**
 * @brief SiiS stochastic rsi indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_STOCHRSI_H
#define SIIS_STOCHRSI_H

#include "../indicator.h"
#include "../../dataarray.h"
#include "../../constants.h"

namespace siis {

/**
 * @brief SiiS stochastic rsi indicator.
 * @author Frederic Scherma
 * @date 2019-03-19
 */
class SIIS_API StochRsi : public Indicator
{
public:

    // TYPE_MOMENTUM
    // CLS_OSCILLATOR

    StochRsi(const o3d::String &name, o3d::Double timeframe,
             o3d::Int32 len=21, o3d::Int32 fastK_Len=12, o3d::Int32 fastD_Len=9, MAType fastD_MAType=MA_SMA);
    StochRsi(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }
    o3d::Int32 fastK_Len() const { return m_fastK_Len; }
    o3d::Int32 fastD_Len() const { return m_fastD_Len; }
    MAType fastD_MAType() const { return m_fastD_MAType; }

    const DataArray& fastK() const { return m_fastK; }
    const DataArray& fastD() const { return m_fastD; }

    o3d::Double lastFastK() const { return m_lastFastK; }
    o3d::Double prevFastK() const { return m_prevFastK; }

    o3d::Double lastFastD() const { return m_lastFastD; }
    o3d::Double prevFastD() const { return m_prevFastD; }

    /**
     * @brief compute Compute a VWMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price);

private:

    o3d::Int32 m_len;
    o3d::Int32 m_fastK_Len;
    o3d::Int32 m_fastD_Len;
    MAType m_fastD_MAType;

    DataArray m_fastK;
    DataArray m_fastD;

    o3d::Double m_prevFastK;
    o3d::Double m_lastFastK;

    o3d::Double m_prevFastD;
    o3d::Double m_lastFastD;
};

} // namespace siis

#endif // SIIS_STOCHRSI_H
