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

    void setUseKahlman(o3d::Bool use);

    inline o3d::Int32 sig() const { return m_sig; }
    inline o3d::Double sigTimestamp() const { return m_sigTimestamp; }

    inline o3d::Int32 trend() const { return m_trend; }
    inline o3d::Double trendTimestamp() const { return m_trendTimestamp; }

    inline o3d::Double lastLoFib() const { return m_lastHiFib; }
    inline o3d::Double lastHiFib() const { return m_lastLoFib; }
    inline o3d::Double lastMed() const { return m_lastMid; }

    o3d::Double takeProfit(o3d::Double profitScale) const;
    o3d::Double stopLoss(o3d::Double lossScale) const;

private:

    class KahlmanFilter
    {
    public:

        KahlmanFilter(o3d::Int32 len, o3d::Double gain = 0.7);

        void resize(o3d::Int32 len);
        void compute(o3d::Double timestamp, const DataArray &price);

        o3d::Int32 len() const { return m_len; }

        const DataArray& kf() const { return m_kf; }

        o3d::Double prev() const { return m_prev; }
        o3d::Double last() const { return m_last; }

    private:

        o3d::Int32 m_len;
        o3d::Double m_gain;
        o3d::Double m_g2Sqrt;

        DataArray m_kf;
        DataArray m_dk;
        DataArray m_smooth;
        DataArray m_velo;

        o3d::Double m_prev;
        o3d::Double m_last;

        o3d::Double m_lastTimestamp;  //!< last compute timestamp
    };

    o3d::Double m_gain;
    o3d::Bool m_kahlman;

    o3d::Double m_trendTimestamp;   //!< timestamp when the trend changes occured
    o3d::Double m_sigTimestamp;     //!< timestamp when a long/short sig occured

    Donchian m_donchian;

    Hma m_hma;
    Hma3 m_hma3;

    o3d::Int32 m_trend;
    o3d::Int32 m_sig;

    o3d::Int32 m_dfTrend;  //!< donchian + fibo trend (0 mean range)

    o3d::Int32 m_confirmation;

    KahlmanFilter m_kHma;
    KahlmanFilter m_kHma3;

    o3d::Double m_lastHiFib;
    o3d::Double m_lastLoFib;
    o3d::Double m_lastMid;

    void kahlmanHma();
    void kahlmanHma3();

    void donchianFibo(o3d::Double timestamp);
};

} // namespace siis

#endif // SIIS_KAHLMANFIBOSIGANALYSER_H
