/**
 * @brief SiiS MAADX strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_MAADXSIGANALYSER_H
#define SIIS_MAADXSIGANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

#include "siis/indicators/sma/sma.h"
#include "siis/indicators/hma/hma.h"
#include "siis/indicators/adx/adx.h"
#include "siis/indicators/cumulativevolumedelta/cvd.h"


namespace siis {

/**
 * @brief SiiS MAADX strategy signal analyser.
 * @author Frederic Scherma
 * @date 2023-04-24
 * Signal analyser
 */
class SIIS_API MaAdxSigAnalyser : public TimeframeBarAnalyser
{
public:

    MaAdxSigAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~MaAdxSigAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;
    virtual void updateTick(const Tick& tick, o3d::Bool finalize) override;

    inline o3d::Double adx() const { return m_adx.last(); }
    inline o3d::Int32 sig() const { return m_sig; }
    inline o3d::Int32 trend() const { return m_trend; }

    inline const CumulativeVolumeDelta& cvd() const { return m_cvd; }
    inline o3d::Int32 cvdTrend() const { return m_cvdTrend; }
    inline o3d::Int32 cvdCross() const { return m_cvdCross; }

    o3d::Double takeProfit(o3d::Double profitScale) const;
    o3d::Double stopLoss(o3d::Double lossScale, o3d::Double riskReward) const;

private:

    Hma m_fast_h_ma;
    Hma m_fast_m_ma;
    Hma m_fast_l_ma;

    Adx m_adx;

    CumulativeVolumeDelta m_cvd;
    Hma m_cvd_ma;

    o3d::Int32 m_trend;
    o3d::Int32 m_sig;
    o3d::Int32 m_cvdTrend;
    o3d::Int32 m_cvdCross;

    o3d::Int32 m_confirmation;
};

} // namespace siis

#endif // SIIS_MAADXSIGANALYSER_H
