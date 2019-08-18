/**
 * @brief SiiS Bollinger band + Awesome indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-08-17
 */

#ifndef SIIS_BBAWE_H
#define SIIS_BBAWE_H

#include "../indicator.h"
#include "../../constants.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS Bollinger band + Awesome indicator.
 * @author Frederic Scherma
 * @date 2019-08-17
 */
class SIIS_API BBawe : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OSCILLATOR

    BBawe(const o3d::String &name,
           o3d::Double timeframe,
           o3d::Int32 bb_Len=20,
           o3d::Double baseMultiplier=2.0,
           o3d::Int32 fastMA_Len=3.0,
           o3d::Int32 awesomeFast_Len=5.0,
           o3d::Int32 awesomeSlow_Len=34.0,
           o3d::Bool useEma=false);
    BBawe(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 bbLen() const { return m_bb_Len; }
    o3d::Double baseMultiplier() const { return m_baseMultiplier; }
    o3d::Double fastMA_Len() const { return m_fastMA_Len; }
    o3d::Double awesomeFast_Len() const { return m_awesomeFast_Len; }
    o3d::Double awesomeSlow_Len() const { return m_awesomeSlow_Len; }
    o3d::Bool useEma() const { return m_useEma; }

    o3d::Int32 signal() const { return m_signal; }

    /**
     * @brief compute Compute the indicator.
     * @param high High price array.
     * @param low Low price array.
     * @param price Close or any other like HL2, HL3, HLC3, OHLC4 price array.
     */
    void compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &price);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_bb_Len;
    o3d::Int32 m_baseMultiplier;
    o3d::Int32 m_fastMA_Len;
    o3d::Int32 m_awesomeFast_Len;
    o3d::Int32 m_awesomeSlow_Len;
    o3d::Bool m_useEma;

    o3d::Int32 m_signal;

    DataArray m_bbBasis;
    DataArray m_fastMA;
    DataArray m_hl2;
    DataArray m_SMA1_hl2;
    DataArray m_SMA2_hl2;
    DataArray m_SMA1_SMA2;
};

} // namespace siis

#endif // SIIS_BBAWE_H
