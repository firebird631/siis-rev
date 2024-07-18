/**
 * @brief SiiS forexalpha strategy D analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_FADANALYSER_H
#define SIIS_FADANALYSER_H

#include "faanalyser.h"

#include "siis/indicators/atr/atr.h"
#include "siis/indicators/sma/sma.h"
#include "siis/indicators/ema/ema.h"
#include "siis/indicators/rsi/rsi.h"
#include "siis/indicators/td9/td9.h"

namespace siis {

/**
 * @brief SiiS forexalpha strategy D analyser.
 * @author Frederic Scherma
 * @date 2019-03-19
 * Prefered for slow timeframe from 2h to 1d. Its a copy from A version plus slow ema/sma.
 */
class SIIS_API FaDAnalyser : public FaAnalyser
{
public:

    FaDAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~FaDAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

private:

    Atr m_atr;
    Sma m_sma;
    Sma m_midSma;
    Sma m_slowSma;
    Ema m_ema;
    Rsi m_rsi;
    Td9 m_td9;
};

} // namespace siis

#endif // SIIS_FADANALYSER_H
