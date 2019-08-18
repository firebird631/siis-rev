/**
 * @brief SiiS SineWave Hilbert transform indicator
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-08-17
 */

#ifndef SIIS_SINEWAVE_H
#define SIIS_SINEWAVE_H

#include "../indicator.h"
#include "../../constants.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SineWave Hilbert transform indicator.
 * @author Frederic Scherma
 * @date 2019-08-17
 */
class SIIS_API SineWave : public Indicator
{
public:

    // TYPE_TREND
    // CLS_CYCLE

    SineWave(const o3d::String &name,
           o3d::Double timeframe);
    SineWave(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    const DataArray& sine() const { return m_sine; }
    const DataArray& leadSine() const { return m_leadSine; }

    o3d::Double lastSine() const { return m_lastSine; }
    o3d::Double prevSine() const { return m_prevSine; }

    o3d::Double lastLeadSine() const { return m_lastLeadSine; }
    o3d::Double prevLeadSine() const { return m_prevLeadSine; }

    /**
     * @brief compute Compute an Hilbert sine wave.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price);

    /**
     * @brief Is sine goes upper (1) or lower (-1) or none (0) of the lead sine.
     */
    o3d::Int32 cross() const;

    /**
     * @brief lookback Min number of necessary samples.
     * @return 63
     */
    o3d::Int32 lookback() const;

private:

    DataArray m_sine;
    DataArray m_leadSine;

    o3d::Double m_lastSine;
    o3d::Double m_prevSine;

    o3d::Double m_lastLeadSine;
    o3d::Double m_prevLeadSine;
};

} // namespace siis

#endif // SIIS_SINEWAVE_H
