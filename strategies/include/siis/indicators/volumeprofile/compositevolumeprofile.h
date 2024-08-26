/**
 * @brief SiiS Composite volume profile indicator based on volume profile indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-26
 */

#ifndef SIIS_COMPOSITEVOLUMEPROFILE_H
#define SIIS_COMPOSITEVOLUMEPROFILE_H

#include "volumeprofile.h"

namespace siis {

/**
 * @brief SiiS Composite volume profile indicator.
 * @author Frederic Scherma
 * @date 2024-08-28
 */
class SIIS_API CompositeVolumeProfile : public Indicator
{
public:

    // TYPE_VOLUME
    // CLS_CUMULATIVE

    /**
     * @brief CumulativeVolumeProfile
     * @param name
     * @param timeframe Related bar timeframe or 0
     * @param length Last number of volume profile to merge
     * @param mergeCurrent Merge volumes of the current (non consolidated) VP.
     */
    CompositeVolumeProfile(const o3d::String &name, o3d::Double timeframe,
                  o3d::Int32 length=10,
                  o3d::Bool mergeCurrent=false);

    CompositeVolumeProfile(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    ~CompositeVolumeProfile();

    void setConf(IndicatorConfig conf);

    o3d::Int32 length() const { return m_length; }

    const VolumeProfileData& vp() const { return m_vp; }

    void setVolumeProfile(VolumeProfile *volumeProfile);

    /**
     * @brief compute Compute a volume profile using the previous n last profiles
     * @note This version recompute the full volume profile. Considers the update method to incrementally
     *       update the profile.
     */
    void composite();

    void update();

    // void updateValueArea();
    // void updatePeaksAndValleys();

    void finalize();

private:

    VolumeProfile *m_pVolumeProfile;

    o3d::Int32 m_length;
    o3d::Bool m_mergeCurrent;

    VolumeProfileData m_vp;
    o3d::Bool m_consolidated;
};

} // namespace siis

#endif // SIIS_COMPOSITEVOLUMEPROFILE_H
