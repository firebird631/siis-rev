/**
 * @brief SiiS Pullback strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#ifndef SIIS_PULLBACKSESSIONANALYSER_H
#define SIIS_PULLBACKSESSIONANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

#include "siis/indicators/volumeprofile/volumeprofile.h"

namespace siis {

/**
 * @brief SiiS Pullbackstrategy session analyser.
 * @author Frederic Scherma
 * @date 2024-08-09
 * Session analyser
 */
class SIIS_API PullbackSessionAnalyser : public TimeframeBarAnalyser
{
public:

    PullbackSessionAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~PullbackSessionAnalyser() override;

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

#endif // SIIS_PULLBACKSESSIONANALYSER_H
