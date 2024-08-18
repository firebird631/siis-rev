/**
 * @brief SiiS MAADX range-bar strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#ifndef SIIS_MAADXRBSESSIONANALYSER_H
#define SIIS_MAADXRBSESSIONANALYSER_H

#include "siis/analysers/rangebaranalyser.h"

#include "siis/indicators/cumulativevolumedelta/cvd.h"
#include "siis/indicators/volumeprofile/volumeprofile.h"

namespace siis {

/**
 * @brief SiiS MAADX range-bar strategy session analyser.
 * @author Frederic Scherma
 * @date 2024-08-09
 * Session analyser
 */
class SIIS_API MaAdxRbSessionAnalyser : public RangeBarAnalyser
{
public:

    MaAdxRbSessionAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~MaAdxRbSessionAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;
    virtual void updateTick(const Tick& tick, o3d::Bool finalize) override;

    inline const VolumeProfile& vp() const { return m_vp; }
    inline const CumulativeVolumeDelta& cvd() const { return m_cvd; }

private:

    VolumeProfile m_vp;
    CumulativeVolumeDelta m_cvd;
};

} // namespace siis

#endif // SIIS_MAADXRBSESSIONANALYSER_H
