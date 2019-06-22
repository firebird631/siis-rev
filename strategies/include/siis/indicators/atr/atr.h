/**
 * @brief SiiS average true range indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_ATR_H
#define SIIS_ATR_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief Siis average true range indicator.
 * @author Frederic Scherma
 * @date 2019-03-19
 */
class SIIS_API Atr : public Indicator
{
public:

    Atr(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=14, o3d::Double factor=3.5);
    Atr(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }
    o3d::Double factor() const { return m_factor; }

    const DataArray& atr() const { return m_atr; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    o3d::Double stopPrice(o3d::Int32 direction) const
    {
        if (direction > 0) {
            return m_longStopPrice;
        } else if (direction < 0) {
            return m_shortStopPrice;
        } else {
            return 0.0;
        }
    }

    /**
     * @brief compute Compute an ATR.
     */
    void compute(o3d::Double timestamp,
                 const DataArray &high,
                 const DataArray &low,
                 const DataArray &close);

private:

    o3d::Int32 m_len;
    o3d::Double m_factor;
    DataArray m_atr;

    o3d::Double m_prev;
    o3d::Double m_last;

    o3d::Double m_longStopPrice;
    o3d::Double m_shortStopPrice;
};

} // namespace siis

#endif // SIIS_ATR_H
