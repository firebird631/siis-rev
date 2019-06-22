/**
 * @brief SiiS forexalpha strategy C analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_FACANALYSER_H
#define SIIS_FACANALYSER_H

#include "siis/analysers/stdanalyser.h"

#include "siis/indicators/atr/atr.h"
#include "siis/indicators/sma/sma.h"
#include "siis/indicators/ema/ema.h"
#include "siis/indicators/rsi/rsi.h"
#include "siis/indicators/td9/td9.h"

namespace siis {

/**
 * @brief SiiS forexalpha strategy C analyser.
 * @author Frederic Scherma
 * @date 2019-03-19
 * Prefered for long timeframe like weekly or many days. Only compute the indicator, does not emit signal.
 */
class SIIS_API FaCAnalyser : public StdAnalyser
{
public:

    FaCAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~FaCAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual TradeSignal compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

private:

    Atr m_atr;
    Sma m_sma;
    Ema m_ema;
    Rsi m_rsi;
};

} // namespace siis

#endif // SIIS_FACANALYSER_H
