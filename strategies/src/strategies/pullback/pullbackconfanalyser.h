/**
 * @brief SiiS MAADX strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#ifndef SIIS_PULLBACKCONFANALYSER_H
#define SIIS_PULLBACKCONFANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/adx/adx.h"


namespace siis {

/**
 * @brief SiiS Pullback strategy confirmation analyser.
 * @author Frederic Scherma
 * @date 2023-05-09
 * Confirmation analyser
 */
class SIIS_API PullbackConfAnalyser : public TimeframeBarAnalyser
{
public:

    PullbackConfAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~PullbackConfAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 confirmation() const { return m_confirmation; }

private:

    o3d::Int32 m_confirmation;
};

} // namespace siis

#endif // SIIS_PULLBACKCONFANALYSER_H
