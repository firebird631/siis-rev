/**
 * @brief SiiS MESA Adaptive Moving Average indicator
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_MAMA_H
#define SIIS_MAMA_H

#include "../indicator.h"
#include "../../constants.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS MESA Adaptive Moving Average indicator.
 * @author Frederic Scherma
 * @date 2019-08-17
 */
class SIIS_API Mama : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OVERLAY

    Mama(const o3d::String &name,
           o3d::Double timeframe,
           o3d::Double fastLimit=0.5,
           o3d::Double slowLimit=0.05);
    Mama(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Double fastLimit() const { return m_fastLimit; }
    o3d::Double slowLimit() const { return m_slowLimit; }

    const DataArray& fama() const { return m_fama; }
    const DataArray& mama() const { return m_mama; }

    o3d::Double lastFama() const { return m_lastFama; }
    o3d::Double prevFama() const { return m_prevFama; }

    o3d::Double lastMama() const { return m_lastMama; }
    o3d::Double prevMama() const { return m_prevMama; }

    /**
     * @brief compute Compute a MAMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price);

    /**
     * @brief lookback Min number of necessary samples.
     * @return 32
     */
    o3d::Int32 lookback() const;

private:

    o3d::Double m_fastLimit;
    o3d::Double m_slowLimit;

    DataArray m_fama;
    DataArray m_mama;

    o3d::Double m_lastFama;
    o3d::Double m_prevFama;

    o3d::Double m_lastMama;
    o3d::Double m_prevMama;
};

} // namespace siis

#endif // SIIS_MAMA_H
