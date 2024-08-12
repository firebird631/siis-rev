/**
 * @brief SiiS Pullback range-bar strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#ifndef SIIS_PULLBACKRBSESSIONANALYSER_H
#define SIIS_PULLBACKRBSESSIONANALYSER_H

#include "siis/analysers/rangebaranalyser.h"

#include "siis/indicators/volumeprofile/volumeprofile.h"

namespace siis {

/**
 * @brief SiiS Pullbackstrategy range-bar session analyser.
 * @author Frederic Scherma
 * @date 2024-08-09
 * Session analyser
 */
class SIIS_API PullbackRbSessionAnalyser : public RangeBarAnalyser
{
public:

    PullbackRbSessionAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~PullbackRbSessionAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;
    virtual void updateTick(const Tick& tick, o3d::Bool finalize) override;

    inline const VolumeProfile& vp() const { return m_vp; }

private:

    VolumeProfile m_vp;
};

} // namespace siis

#endif // SIIS_PULLBACKRBSESSIONANALYSER_H
