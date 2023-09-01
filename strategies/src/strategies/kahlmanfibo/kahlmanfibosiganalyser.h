/**
 * @brief SiiS strategy KahlmanFibo strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#ifndef SIIS_KAHLMANFIBOSIGANALYSER_H
#define SIIS_KAHLMANFIBOSIGANALYSER_H

#include "siis/analysers/stdanalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/hma3/hma3.h"
#include "siis/indicators/donchian/donchian.h"

namespace siis {

/**
 * @brief SiiS MAADX strategy signal analyser.
 * @author Frederic Scherma
 * @date 2023-04-24
 * Signal analyser
 * @note It is important that price method is HLC.
 */
class SIIS_API KahlmanFiboSigAnalyser : public StdAnalyser
{
public:

    KahlmanFiboSigAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_HL);

    virtual ~KahlmanFiboSigAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual TradeSignal compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 sig() const { return m_sig; }

    o3d::Double takeProfit(o3d::Double profitScale) const;
    o3d::Double stopLoss(o3d::Double lossScale, o3d::Double riskReward) const;

private:

    o3d::Double m_gain;
    o3d::Bool m_kahlman;

    o3d::Double m_trendTimestamp;   //!< timestamp when the trend changes occured

    Donchian m_donchian;

    Hma m_hma;
    Hma3 m_hma3;

    o3d::Int32 m_trend;
    o3d::Int32 m_sig;

    o3d::Int32 m_dfTrend;  //!< donchian + fibo trend (0 mean range)

    o3d::Int32 m_confirmation;

    DataArray m_kHma;
    DataArray m_kHma3;

    void kahlmanHma();
    void kahlmanHma3();

    void donchianFibo();
};

} // namespace siis

#endif // SIIS_KAHLMANFIBOSIGANALYSER_H
