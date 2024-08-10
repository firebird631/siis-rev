/**
 * @brief SiiS Pullback range-bar strategy support/resistance analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#ifndef SIIS_PULLBACKRBSRANALYSER_H
#define SIIS_PULLBACKRBSRANALYSER_H

#include "siis/analysers/rangebaranalyser.h"

#include "siis/indicators/pivotpoint/pivotpoint.h"

namespace siis {

/**
 * @brief SiiS Pullback range-bar strategy support/resistance analyser.
 * @author Frederic Scherma
 * @date 2023-04-24
 * Support/resistance analyser
 */
class SIIS_API PullbackRbSRAnalyser : public RangeBarAnalyser
{
public:

    PullbackRbSRAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~PullbackRbSRAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(AnalyserConfig conf) override;
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

#endif // SIIS_PULLBACKRBSRANALYSER_H
