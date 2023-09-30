/**
 * @brief Strategy trade dynnamic stop-loss or in profit handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-04
 */

#ifndef SIIS_DYNAMICSTOPLOSS_H
#define SIIS_DYNAMICSTOPLOSS_H

#include "entryexit.h"

namespace siis {

/**
 * @brief Strategy trade dynnamic stop-loss or in profit handler.
 * @author Frederic Scherma
 * @date 2023-05-04
 * Inherit from this model for a custom policy and overrides update and init.
 */
class SIIS_API DynamicStopLoss : public EntryExit
{
public:

    DynamicStopLoss();

    void init(const Market *market, const ContextConfig &conf);

    void updateΤrade(o3d::Double timestamp, o3d::Double lastTimestamp, Trade *trade) const;

    o3d::Bool checkMinDistance(Trade *trade) const;

private:

    o3d::Double m_breakevenDistance;

    PriceType m_breakevenPriceType;
    DistanceType m_breakevenDistanceType;
};

} // namespace siis

#endif // SIIS_DYNAMICSTOPLOSS_H
