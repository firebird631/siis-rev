/**
 * @brief SiiS Ichimoku strategy confirmation analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#ifndef SIIS_ICHIMOKUSTCONFANALYSER_H
#define SIIS_ICHIMOKUSTCONFANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

namespace siis {

/**
 * @brief SiiS Ichimoku strategy confirmation analyser.
 * @author Frederic Scherma
 * @date 2024-07-15
 * Confirmation analyser
 */
class SIIS_API IchimokuStConfAnalyser : public TimeframeBarAnalyser
{
public:

    IchimokuStConfAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~IchimokuStConfAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 confirmation() const { return m_confirmation; }

private:

    o3d::Int32 m_confirmation;
};

} // namespace siis

#endif // SIIS_SUPERTRENDRBCONFANALYSER_H
