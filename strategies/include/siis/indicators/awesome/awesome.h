/**
 * @brief SiiS Awesome indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-08-17
 */

#ifndef SIIS_AWESOME_H
#define SIIS_AWESOME_H

#include "../indicator.h"
#include "../../constants.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS Awesome indicator.
 * @author Frederic Scherma
 * @date 2019-08-17
 * @see https://www.tradingview.com/script/QYCToBoN-Bollinger-Awesome-Alert-R1-by-JustUncleL/
 */
class SIIS_API Awesome : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OSCILLATOR

    Awesome(const o3d::String &name,
            o3d::Double timeframe,
            o3d::Int32 bollingerLen=20,
            o3d::Double baseMultiplier=2.0,  // std dev multiplier, basic is 2
            o3d::Int32 fastMA_Len=3.0,
            o3d::Int32 awesomeFastLen=5.0,
            o3d::Int32 awesomeSlowLen=34.0,
            o3d::Bool bollingerFilter=false,
            o3d::Bool squeezeFilter=false,
            o3d::Bool useEma=false,
            o3d::Int32 squeezeLen=100,         // min 5
            o3d::Int32 squeezeThreshold=50);   // in percent 0..9

    Awesome(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 bollingerLen() const { return m_bollingerLen; }
    o3d::Double baseMultiplier() const { return m_baseMultiplier; }
    o3d::Double fastMA_Len() const { return m_fastMaLen; }
    o3d::Double awesomeFast_Len() const { return m_awesomeFastLen; }
    o3d::Double awesomeSlow_Len() const { return m_awesomeSlowLen; }
    o3d::Bool useEma() const { return m_useEma; }
    o3d::Bool hasBollingerFilter() const { return m_bollingerFilter; }
    o3d::Bool hasSqueezeFilter() const { return m_squeezeFilter; }
    o3d::Int32 squeezeLen() const { return m_squeezeLen; }
    o3d::Int32 squeezeThresholdLen() const { return m_squeezeThreshold; }

    o3d::Int32 signal() const { return m_signal; }

    /**
     * @brief compute Compute the indicator.
     * @param high High price array.
     * @param low Low price array.
     * @param close Close price array.
     */
    void compute(o3d::Double timestamp, const DataArray &high, const DataArray &low, const DataArray &close);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_bollingerLen;
    o3d::Int32 m_baseMultiplier;
    o3d::Int32 m_fastMaLen;
    o3d::Int32 m_awesomeFastLen;
    o3d::Int32 m_awesomeSlowLen;
    o3d::Bool m_useEma;
    o3d::Bool m_bollingerFilter;
    o3d::Bool m_squeezeFilter;
    o3d::Int32 m_squeezeLen;
    o3d::Int32 m_squeezeThreshold;

    o3d::Int32 m_signal;
    o3d::Int32 m_filteredSignal;

    DataArray m_bbBasis;
    DataArray m_bbDev;
    DataArray m_bbUpper;
    DataArray m_bbLower;
    DataArray m_spread;
    DataArray m_avgSpread;
    DataArray m_bbSqueeze;
    DataArray m_bbOffset;
    DataArray m_bbSqzUpper;
    DataArray m_bbSqzLower;
    DataArray m_fastMA;
    DataArray m_hl2;
    DataArray m_SMA1_hl2;
    DataArray m_SMA2_hl2;
    DataArray m_SMA1_SMA2;
};

} // namespace siis

#endif // SIIS_AWESOME_H
