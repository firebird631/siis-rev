/**
 * @brief SiiS tick Cumulative Volume Delta indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-18
 */

#ifndef SIIS_CVD_H
#define SIIS_CVD_H

#include "../../tick.h"

#include "../indicator.h"
#include "../../datacircular.h"

namespace siis {

/**
 * @brief SiiS tick Cumulative Volume Delta indicator.
 * @author Frederic Scherma
 * @date 2024-08-18
 */
class SIIS_API CumulativeVolumeDelta : public Indicator
{
public:

    // TYPE_MOMENTUM_VOLUME
    // CLS_CUMULATIVE

    /**
     * @brief VWap
     * @param name
     * @param timeframe Related bar timeframe or 0
     * @param cvdTimeframe CVD session/timeframe (one of "1d", "1w", "1M")
     * @param depth Max series size. Generally similar to analyser depth.
     * @param sessionFilter If defined, ticks received out of the session are ignored
     */
    CumulativeVolumeDelta(const o3d::String &name, o3d::Double timeframe,
         const o3d::CString &cvdTimeframe="1d",
         o3d::Int32 depth=10,
         o3d::Bool sessionFilter=false);

    CumulativeVolumeDelta(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    ~CumulativeVolumeDelta();

    void setConf(IndicatorConfig conf);

    void setSession(o3d::Double sessionOffset, o3d::Double sessionDuration);

    o3d::Int32 depth() const { return m_depth; }
    o3d::Bool hasSessionFilter() const { return m_sessionFilter; }

    o3d::Bool hasValues() const { return m_cvd.size() > 0; }

    const DataCircular& cvd() const { return m_cvd; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    /**
     * @brief compute Compute a volume profile per tick.
     * @param finalize Once the related bar closed it must finalize the current profile.
     */
    void update(const Tick &tick, o3d::Bool finalize=false);

private:

    o3d::Double m_cvdTimeframe;

    o3d::Double m_sessionOffset;     //!< 0 means starts at 00:00 UTC
    o3d::Double m_sessionDuration;   //!< 0 means full day

    o3d::Int32 m_depth;

    o3d::Bool m_sessionFilter;

    o3d::Double m_openTimestamp;
    o3d::Double m_prevTickPrice;

    DataCircular m_cvd;

    o3d::Double m_prev;
    o3d::Double m_last;

    void finalize();
};

} // namespace siis

#endif // SIIS_CVD_H
