/**
 * @brief SiiS momentum indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_MMT_H
#define SIIS_MMT_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS simple moving average indicator.
 * @author Frederic Scherma
 * @date 2019-03-15
 */
class SIIS_API Mmt : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OSCILLATOR

    Mmt(const o3d::String &name, o3d::Double timeframe, o3d::Int32 len=20);
    Mmt(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 len() const { return m_len; }

    const DataArray& mmt() const { return m_mmt; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    /**
     * @brief compute Compute a SMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price);

private:

    o3d::Int32 m_len;
    DataArray m_mmt;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_MMT_H
