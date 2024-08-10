/**
 * @brief SiiS MAADX range-bar strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#ifndef SIIS_PULLBACKRBCONFANALYSER_H
#define SIIS_PULLBACKRBCONFANALYSER_H

#include "siis/analysers/rangebaranalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/adx/adx.h"


namespace siis {

/**
 * @brief SiiS Pullback range-bar strategy confirmation analyser.
 * @author Frederic Scherma
 * @date 2023-05-09
 * Confirmation analyser
 */
class SIIS_API PullbackRbConfAnalyser : public RangeBarAnalyser
{
public:

    PullbackRbConfAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~PullbackRbConfAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 confirmation() const { return m_confirmation; }

private:

    o3d::Int32 m_confirmation;
};

} // namespace siis

#endif // SIIS_PULLBACKRBCONFANALYSER_H
