 /**
 * @brief SiiS tick VWAP indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-18
 */

#ifndef SIIS_VWAP_H
#define SIIS_VWAP_H

#include "vwapdata.h"
#include "../../tick.h"

#include "../indicator.h"
#include "../../dataarray.h"

#include <deque>

namespace siis {

/**
 * @brief SiiS tick VWAP indicator.
 * @author Frederic Scherma
 * @date 2024-08-18
 */
class SIIS_API VWap: public Indicator
{
public:

    typedef std::vector<std::pair<o3d::Double, std::pair<o3d::Double, o3d::Double>>> T_VolumeByPrice;
    typedef T_VolumeByPrice::iterator IT_VolumeByPrice;

    // TYPE_AVERAGE_PRICE
    // CLS_INDEX

    /**
     * @brief VWap
     * @param name
     * @param timeframe Related bar timeframe or 0
     * @param vwapTimeframe VWAP session/timeframe (one of "1d", "1w", "1M")
     * @param historySize min 1
     * @param sessionFilter If defined, ticks received out of the session are ignored
     */
    VWap(const o3d::String &name, o3d::Double timeframe,
         const o3d::CString &vwapTimeframe,
         o3d::Int32 historySize=2,
         o3d::Bool sessionFilter=false);

    VWap(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    ~VWap();

    void setConf(IndicatorConfig conf);

    void setSession(o3d::Double sessionOffset, o3d::Double sessionDuration);

    o3d::Int32 historySize() const { return m_historySize; }
    o3d::Bool hasSessionFilter() const { return m_sessionFilter; }

    o3d::Bool hasValues() const { return !m_vwap.empty(); }

    const std::deque<VWapData*> vwap() const { return m_vwap; }

    o3d::Bool hasCurrent() const { return m_pCurrent != nullptr; }

    const VWapData* current() const { return m_pCurrent; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    /**
     * @brief previous A previous VWAP session
     * @param at Negative index or positive works
     * @return
     * @exception IndexOutOfRange
     */
    const VWapData* previous(o3d::Int32 n) const;

    o3d::Double vwapAt(o3d::Int32 n) const;
    o3d::Double stdDevAt(o3d::Int32 n, o3d::Int32 stdDevNum) const;

    /**
     * @brief compute Compute a volume profile per tick.
     * @param finalize Once the related bar closed it must finalize the current profile.
     */
    void update(const Tick &tick, o3d::Bool finalize=false);

    void finalize();

private:

    o3d::Double m_vwapTimeframe;
    o3d::Int32 m_numStdDev;

    o3d::Double m_sessionOffset;     //!< 0 means starts at 00:00 UTC
    o3d::Double m_sessionDuration;   //!< 0 means full day

    o3d::Int32 m_historySize;

    o3d::Bool m_sessionFilter;

    VWapData *m_pCurrent;

    o3d::Double m_openTimestamp;

    o3d::Double m_pvs;
    o3d::Double m_volumes;
    o3d::Double m_volume_dev;
    o3d::Double m_dev2;

    std::deque<VWapData*> m_vwap;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_VWAP_H
