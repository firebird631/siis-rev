/**
 * @brief SiiS average directional index indicator.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_ADX_H
#define SIIS_ADX_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS average directional index indicator.
 * @author Frederic Scherma
 * @date 2023-04-24
 */
class SIIS_API Adx : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OSCILLATOR

    Adx(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=5);
    Adx(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }

    const DataArray& adx() const { return m_adx; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

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
    DataArray m_adx;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_ADX_H
