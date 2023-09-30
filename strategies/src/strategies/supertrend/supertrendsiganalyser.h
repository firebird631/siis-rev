/**
 * @brief SiiS strategy SuperTrend strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#ifndef SIIS_SUPERTRENDSIGANALYSER_H
#define SIIS_SUPERTRENDSIGANALYSER_H

#include "siis/analysers/stdanalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/hma3/hma3.h"
#include "siis/indicators/kahlmanfilter/kahlmanfilter.h"
#include "siis/indicators/supertrend/supertrend.h"

namespace siis {

/**
 * @brief SiiS SuperTrend strategy signal trend analyser.
 * @author Frederic Scherma
 * @date 2023-09-04
 * Signal analyser
 * @note It is important that price method is HLC.
 */
class SIIS_API SuperTrendSigAnalyser : public StdAnalyser
{
public:

    SuperTrendSigAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_HL);

    virtual ~SuperTrendSigAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual TradeSignal compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    void setUseKahlman(o3d::Bool use);

    inline o3d::Int32 trend() const { return m_trend; }
    inline o3d::Double trendTimestamp() const { return m_trendTimestamp; }

    o3d::Int32 sig() const;

    o3d::Double takeProfit(o3d::Int32 direction, o3d::Double profitScale) const;
    o3d::Double stopLoss(o3d::Int32 direction, o3d::Double riskScale, o3d::Double onePipMeans) const;

    o3d::Double dynamicStopLoss(o3d::Int32 direction, o3d::Double curStopLoss, o3d::Double distance) const;

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

#endif // SIIS_SUPERTRENDSIGANALYSER_H
