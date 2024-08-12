/**
 * @brief SiiS Ichimoku strategy range filter analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#ifndef SIIS_ICHIMOKUSTRANGEANALYSER_H
#define SIIS_ICHIMOKUSTRANGEANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"
#include "siis/indicators/sma/sma.h"
#include "siis/indicators/ema/ema.h"

namespace siis {

/**
 * @brief SiiS Ichimoku strategy range filter analyser.
 * @author Frederic Scherma
 * @date 2024-07-15
 * Range filter analyser
 */
class SIIS_API IchimokuStRangeAnalyser : public TimeframeBarAnalyser
{
public:

    IchimokuStRangeAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~IchimokuStRangeAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Bool inRange() const { return m_inRange; }

private:

    Sma m_slow_ma_high;
    Sma m_slow_ma_low;

    Ema m_fast_ma_high;
    Ema m_fast_ma_low;

    o3d::Bool m_inRange;
};

} // namespace siis

#endif // SIIS_ICHIMOKUSTSIGANALYSER_H
