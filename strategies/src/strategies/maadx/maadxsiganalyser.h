/**
 * @brief SiiS MAADX strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_MAADXSIGANALYSER_H
#define SIIS_MAADXSIGANALYSER_H

#include "siis/analysers/stdanalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/adx/adx.h"


namespace siis {

/**
 * @brief SiiS MAADX strategy signal analyser.
 * @author Frederic Scherma
 * @date 2023-04-24
 * Signal analyser
 * @note It is important that price method is HLC.
 */
class SIIS_API MaAdxSigAnalyser : public StdAnalyser
{
public:

    MaAdxSigAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_HLC);

    virtual ~MaAdxSigAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual TradeSignal compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

private:

    Hma m_slow_h_ma;
    Hma m_slow_m_ma;
    Hma m_slow_l_ma;

    Adx m_adx;
};

} // namespace siis

#endif // SIIS_MAADXSIGANALYSER_H
