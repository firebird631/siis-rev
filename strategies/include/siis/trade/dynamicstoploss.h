/**
 * @brief Strategy trade dynnamic stop-loss or in profit handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-04
 */

#ifndef SIIS_DYNAMICSTOPLOSS_H
#define SIIS_DYNAMICSTOPLOSS_H

#include "../base.h"
#include "../utils/common.h"

#include <o3d/core/base.h>

namespace siis {

class Market;
class ContextConfig;
class Trade;

/**
 * @brief Strategy trade dynnamic stop-loss or in profit handler.
 * @author Frederic Scherma
 * @date 2023-05-04
 */
class SIIS_API DynamicStopLoss
{
public:

    DynamicStopLoss();

    void init(const Market *market, ContextConfig &conf);

    void update(Trade *trade);

protected:

    o3d::Double m_timeframe;

    o3d::Double m_distance;   //! distance in percent or pip or value
    DistanceType m_distanceType;
    AdjustPolicy m_adjustPolicy;
};

} // namespace siis

#endif // SIIS_DYNAMICSTOPLOSS_H
