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
class EntryExitConfig;
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

    void init(const Market *market, const EntryExitConfig &conf);

    /**
     * @brief update To be called at each processing pass before trade manager update.
     * @param timestamp Current timestamp.
     * @param lastTimestamp Previous processed timestamp.
     */
    void update(o3d::Double timestamp, o3d::Double lastTimestamp);

    /**
     * @brief consolidated True if the timeframe is consolidate during this processing pass.
     * Always true if no timeframe defined.
     */
    o3d::Bool consolidated() const { return m_timeframe > 0.0 ? m_consolidated : true; }

    o3d::Double distance() const { return m_distance; }
    DistanceType distanceType() const { return m_distanceType; }

    o3d::Double offset() const { return m_offset; }
    DistanceType offsetType() const { return m_offsetType; }

    PriceType priceType() const { return m_priceType; }
    AdjustPolicy adjustPolicy() const { return m_adjustPolicy; }

    o3d::Double lastClosedTimestamp() const { return m_lastClosedTimestamp; }

protected:

    PriceType m_priceType;
    o3d::Double m_timeframe;  

    o3d::Double m_distance;   //! distance in percent or pip or value
    DistanceType m_distanceType;
    AdjustPolicy m_adjustPolicy;

    o3d::Double m_offset;     //! offset in percent or pip or value
    DistanceType m_offsetType;

    o3d::Bool m_consolidated;
    o3d::Double m_lastClosedTimestamp;
};

} // namespace siis

#endif // SIIS_ENTRYEXIT_H
