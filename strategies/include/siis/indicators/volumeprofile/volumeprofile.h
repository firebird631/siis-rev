 /**
 * @brief SiiS tick volume profile indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-02
 */

#ifndef SIIS_VOLUMEPROFILE_H
#define SIIS_VOLUMEPROFILE_H

#include "volumeprofiledata.h"
#include "../../tick.h"

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS tick volume profile indicator.
 * @author Frederic Scherma
 * @date 2024-08-02
 */
class SIIS_API VolumeProfile : public Indicator
{
public:

    // TYPE_VOLUME
    // CLS_CUMULATIVE

    VolumeProfile(const o3d::String &name, o3d::Double timeframe,
                  o3d::Int32 historySize=10,
                  o3d::Double sensibility=1.0,
                  o3d::Double valueArea=70.0,
                  o3d::Bool computePeaksAndValleys=true);

    VolumeProfile(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Double sensibility() const { return m_sensibility; }
    o3d::Double valueArea() const { return m_valueArea; }
    o3d::Bool computePeaksAndValleys() const { return m_computePeaksAndValleys; }

    const std::vector<VolumeProfileData> vp() const { return m_vp; }

    const VolumeProfileData& current() const { return m_current; }

    /**
     * @brief compute Compute a volume profile per tick.
     * @param finalize Once the related bar closed it must finalize the current profile.
     */
    void update(o3d::Double timestamp, const Tick &tick, o3d::Bool finalize=false);

private:

    // @todo market tick size, scale, precision (@see rangebar)

    o3d::Int32 m_historySize;
    o3d::Double m_sensibility;

    o3d::Double m_valueArea;
    o3d::Bool m_computePeaksAndValleys;

    VolumeProfileData m_current;

    std::vector<VolumeProfileData> m_vp;
};

} // namespace siis

#endif // SIIS_TICKVOLUMEPROFILE_H
