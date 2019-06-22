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

private:

    DataArray m_volume;

    o3d::Double m_min;
    o3d::Double m_max;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_VOLUME_H
