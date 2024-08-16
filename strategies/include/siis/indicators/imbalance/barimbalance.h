 /**
 * @brief SiiS bar imbalance detector indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-02
 */

#ifndef SIIS_BARIMBALANCE_H
#define SIIS_BARIMBALANCE_H

#include "../indicator.h"
#include "../../dataarray.h"

#include <list>

namespace siis {

class SIIS_API Imbalance
{
public:

    o3d::Double barTimestamp {0};
    o3d::Int32 direction {0};

    o3d::Double lowPrice {0};
    o3d::Double highPrice {0};
};

/**
 * @brief SiiS bar imbalance detector indicator.
 * @author Frederic Scherma
 * @date 2024-08-16
 * Imbalance detection between bars
 * @note Works with both temporal and non-temporal bars.
 * @note Compute incrementally.
 *
 * New imbalance are computed only after a bar close.
 * Previous can be updated at each price move or at bar close.
 */
class SIIS_API BarImbalance : public Indicator
{
public:

    typedef std::list<Imbalance> T_Imbalance;
    typedef T_Imbalance::iterator IT_Imbalance;

    // TYPE_SUPPORT_RESISTANCE
    // CLS_OVERLAY

    /**
     * @brief BarImbalance
     * @param name
     * @param timeframe Related bar timeframe or 0
     * @param depth min 1 Number of bar lookback if lower than given array.
     */
    BarImbalance(const o3d::String &name, o3d::Double timeframe, o3d::Int32 depth=10);

    BarImbalance(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    ~BarImbalance();

    void setConf(IndicatorConfig conf);

    void setSession(o3d::Double sessionOffset, o3d::Double sessionDuration);

    o3d::Int32 depth() const { return m_depth; }

    o3d::Bool hasValues() const { return !m_imbalances.empty(); }

    const T_Imbalance imbalance() const { return m_imbalances; }

    /**
     * @brief compute Compute imbalance for last bars and update previous.
     */
    void compute(o3d::Double timestamp, const DataArray &timestamps, const DataArray &open,
                 const DataArray &high, const DataArray &low, const DataArray &close, o3d::Int32 numLastBars);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Double m_sessionOffset;     //!< 0 means starts at 00:00 UTC
    o3d::Double m_sessionDuration;   //!< 0 means full day

    o3d::Int32 m_depth;

    T_Imbalance m_imbalances;

    T_Imbalance updateImbalances(T_Imbalance& imbalances,
                                 const DataArray &high, const DataArray &low, o3d::Int32 numLastBars);
};

} // namespace siis

#endif // SIIS_BARIMBALANCE_H
