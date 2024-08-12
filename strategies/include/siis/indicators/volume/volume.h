/**
 * @brief SiiS strategy volume indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#ifndef SIIS_VOLUME_H
#define SIIS_VOLUME_H

#include "../indicator.h"
#include "../../ohlc.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS strategy volume indicator.
 * @author Frederic Scherma
 * @date 2019-03-15
 */
class SIIS_API Volume : public Indicator
{
public:

    // TYPE_TREND
    // CLS_INDEX

    Volume(const o3d::String &name, o3d::Double timeframe);
    Volume(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    const DataArray& volume() const { return m_volume; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    o3d::Double min() const { return m_min; }
    o3d::Double max() const { return m_max; }

    void compute(const OhlcCircular &ohlc);

    /**
     * @brief computeMinimalist Similar as compute but only update the last bar if num bars is 1
     * @param ohlc
     * If num bars is greater than 1 it then call the default compute method.
     * @note This is an optimized compute version without trade-off.
     */
    void computeMinimalist(const OhlcCircular &ohlc, const Ohlc *current, o3d::Int32 numBars);

private:

    DataArray m_volume;

    o3d::Double m_min;
    o3d::Double m_max;

    o3d::Double m_prev;
    o3d::Double m_last;

    void computeLast(const Ohlc *current);
};

} // namespace siis

#endif // SIIS_VOLUME_H
