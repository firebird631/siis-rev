/**
 * @brief SiiS strategy KahlmanFibo strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#ifndef SIIS_KAHLMANFIBOTRENDANALYSER_H
#define SIIS_KAHLMANFIBOTRENDANALYSER_H

#include "siis/analysers/stdanalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/hma3/hma3.h"
#include "siis/indicators/donchian/donchian.h"

namespace siis {

/**
 * @brief SiiS MAADX strategy higher trend analyser.
 * @author Frederic Scherma
 * @date 2023-09-04
 * Signal analyser
 * @note It is important that price method is HLC.
 */
class SIIS_API KahlmanFiboTrendAnalyser : public StdAnalyser
{
public:

    KahlmanFiboTrendAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_HL);

    virtual ~KahlmanFiboTrendAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual TradeSignal compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    void setUseKahlman(o3d::Bool use);

    inline o3d::Int32 trend() const { return m_trend; }
    inline o3d::Double trendTimestamp() const { return m_trendTimestamp; }

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

    Hma m_hma;
    Hma3 m_hma3;

    o3d::Int32 m_trend;

    KahlmanFilter m_kHma;
    KahlmanFilter m_kHma3;

    void kahlmanHma();
    void kahlmanHma3();
};

} // namespace siis

#endif // SIIS_KAHLMANFIBOTRENDANALYSER_H
