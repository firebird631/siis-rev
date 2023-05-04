/**
 * @brief SiiS strategy trade breakeven handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-04
 */

#ifndef SIIS_BREAKEVEN_H
#define SIIS_BREAKEVEN_H

#include "../base.h"
#include "../utils/common.h"

#include <o3d/core/base.h>

namespace siis {

class Market;
class ContextConfig;
class Trade;

/**
 * @brief Strategy trade breakeven handler.
 * @author Frederic Scherma
 * @date 2023-05-04
 */
class SIIS_API Breakeven
{
public:

    Breakeven();

    void init(const Market *market, ContextConfig &conf);

    void update(Trade *trade);

protected:

    o3d::Double m_timeframe;

    o3d::Double m_distance;   //! distance in percent or pip or value
    DistanceType m_distanceType;
    AdjustPolicy m_adjustPolicy;
};

} // namespace siis

#endif // SIIS_BREAKEVEN_H
