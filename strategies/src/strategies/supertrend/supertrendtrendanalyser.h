/**
 * @brief SiiS SuperTrend strategy trend analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#ifndef SIIS_SUPERTRENDTRENDANALYSER_H
#define SIIS_SUPERTRENDTRENDANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/hma3/hma3.h"
#include "siis/indicators/kahlmanfilter/kahlmanfilter.h"
#include "siis/indicators/supertrend/supertrend.h"

namespace siis {

/**
 * @brief SiiS SuperTrend strategy trend analyser.
 * @author Frederic Scherma
 * @date 2023-09-30
 * Trend analyser
 * @note It is important that price method is HLC.
 */
class SIIS_API SuperTrendTrendAnalyser : public TimeframeBarAnalyser
{
public:

    SuperTrendTrendAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_HLC);

    virtual ~SuperTrendTrendAnalyser() override;

    void setUseKahlman(o3d::Bool use);

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 trend() const { return m_trend; }

private:

    o3d::Double m_gain;
    o3d::Bool m_kahlman;

    o3d::Double m_trendTimestamp;   //!< timestamp when the trend changes occured

    Hma m_hma;
    Hma3 m_hma3;
    SuperTrend m_superTrend;

    o3d::Int32 m_trend;

    KahlmanFilter m_kHma;
    KahlmanFilter m_kHma3;

    void kahlmanHma();
    void kahlmanHma3();
};

} // namespace siis

#endif // SIIS_SUPERTRENDTRENDANALYSER_H
