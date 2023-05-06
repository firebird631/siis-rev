/**
 * @brief SiiS strategy trade entry/exit handler base model.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-06
 */

#ifndef SIIS_ENTRYEXIT_H
#define SIIS_ENTRYEXIT_H

#include "../base.h"
#include "../utils/common.h"

#include <o3d/core/base.h>

namespace siis {

class Market;
class ContextConfig;
class Trade;

/**
 * @brief SiiS strategy trade entry/exit handler base model.
 * @author Frederic Scherma
 * @date 2023-05-06
 */
class SIIS_API EntryExit
{
public:

    EntryExit();

    void init(const Market *market, ContextConfig &conf);

    // void update(o3d::Double timestamp, o3d::Double lastTimestamp, Trade *trade);

    o3d::Bool consolidated(o3d::Double timestamp, o3d::Double lastTimestamp);

protected:

    o3d::Double m_timeframe;

    o3d::Double m_distance;   //! distance in percent or pip or value
    DistanceType m_distanceType;
    AdjustPolicy m_adjustPolicy;

    o3d::Bool m_consolidated;
    o3d::Double m_lastClosedTimestamp;
};

} // namespace siis

#endif // SIIS_ENTRYEXIT_H
