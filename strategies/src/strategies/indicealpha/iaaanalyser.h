/**
 * @brief SiiS indicealpha strategy A analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#ifndef SIIS_IAAANALYSER_H
#define SIIS_IAAANALYSER_H

#include "iaanalyser.h"

#include "siis/indicators/atr/atr.h"
#include "siis/indicators/sma/sma.h"
#include "siis/indicators/ema/ema.h"
#include "siis/indicators/rsi/rsi.h"
#include "siis/indicators/td9/td9.h"

namespace siis {

/**
 * @brief SiiS indicealpha strategy A analyser.
 * @author Frederic Scherma
 * @date 2019-06-03
 * Prefered for common timeframe from 3m to 4h.
 */
class SIIS_API IaAAnalyser : public IaAnalyser
{
public:

    IaAAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~IaAAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

private:

    Atr m_atr;
    Sma m_sma;
    Ema m_ema;
    Rsi m_rsi;
    Td9 m_td9;
};

} // namespace siis

#endif // SIIS_IAAANALYSER_H
