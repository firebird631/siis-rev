/**
 * @brief SiiS SuperTrend range-bar strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#ifndef SIIS_SUPERTRENDRBCONFANALYSER_H
#define SIIS_SUPERTRENDRBCONFANALYSER_H

#include "siis/analysers/rangebaranalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/hma3/hma3.h"
#include "siis/indicators/supertrend/supertrend.h"


namespace siis {

/**
 * @brief SiiS SuperTrend range-bar strategy confirmation analyser.
 * @author Frederic Scherma
 * @date 2023-09-30
 * Confirmation analyser
 */
class SIIS_API SuperTrendRbConfAnalyser : public RangeBarAnalyser
{
public:

    SuperTrendRbConfAnalyser(
            Strategy *strategy,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~SuperTrendRbConfAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual TradeSignal compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 confirmation() const { return m_confirmation; }

private:

    o3d::Int32 m_confirmation;
};

} // namespace siis

#endif // SIIS_SUPERTRENDRBCONFANALYSER_H
