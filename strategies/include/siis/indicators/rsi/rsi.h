/**
 * @brief SiiS relative strengh index indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_RSI_H
#define SIIS_RSI_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS relative strengh index indicator.
 * @author Frederic Scherma
 * @date 2019-03-19
 */
class SIIS_API Rsi : public Indicator
{
public:

    // TYPE_MOMENTUM
    // CLS_OSCILLATOR

    Rsi(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=21);
    Rsi(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }

    const DataArray& rsi() const { return m_rsi; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    /**
     * @brief compute Compute a RSI.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price);

    /**
     * @brief lookback Min number of necessary samples.
     * @return len
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_len;
    DataArray m_rsi;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_RSI_H
