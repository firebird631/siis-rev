/**
 * @brief SiiS Pullback strategy support/resistance analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#ifndef SIIS_PULLBACKSRANALYSER_H
#define SIIS_PULLBACKSRANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

#include "siis/indicators/pivotpoint/pivotpoint.h"

namespace siis {

/**
 * @brief SiiS Pullback strategy support/resistance analyser.
 * @author Frederic Scherma
 * @date 2023-04-24
 * Support/resistance analyser
 */
class SIIS_API PullbackSRAnalyser : public TimeframeBarAnalyser
{
public:

    PullbackSRAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~PullbackSRAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    o3d::Int32 breakoutDirection() const { return m_breakoutDirection; }
    o3d::Double breakoutPrice() const { return m_breakoutPrice; }
    o3d::Int32 srLevel() const { return m_srLevel; }

private:

    PivotPoint m_pivotpoint;

    o3d::Int32 m_breakoutDirection;
    o3d::Double m_breakoutPrice;
    o3d::Int32 m_srLevel;
};

} // namespace siis

#endif // SIIS_PULLBACKSRANALYSER_H
