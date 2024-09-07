/**
 * @brief SiiS MAADX strategy trend analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_MAADXTRENDANALYSER_H
#define SIIS_MAADXTRENDANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/vwap/vwap.h"


namespace siis {

/**
 * @brief SiiS MAADX strategy trend analyser.
 * @author Frederic Scherma
 * @date 2023-04-24
 * Trend analyser
 */
class SIIS_API MaAdxTrendAnalyser : public TimeframeBarAnalyser
{
public:

    MaAdxTrendAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~MaAdxTrendAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;
    virtual void updateTick(const Tick& tick, o3d::Bool finalize) override;

    inline o3d::Double lastMaHigh() const { return m_slow_h_ma.last(); }
    inline o3d::Double lastMaLow() const { return m_slow_l_ma.last(); }

    inline o3d::Int32 trend() const { return m_trend; }
    inline o3d::Int32 sig() const { return m_sig; }
    inline o3d::Int32 vwapTrend() const { return m_vwapTrend; }

private:

    Hma m_slow_h_ma;
    Hma m_slow_m_ma;
    Hma m_slow_l_ma;

    VWap m_vwap;

    o3d::Int32 m_trend;
    o3d::Int32 m_cross;
    o3d::Int32 m_sig;
    o3d::Int32 m_vwapTrend;
};

} // namespace siis

#endif // SIIS_MAADXTRENDANALYSER_H
