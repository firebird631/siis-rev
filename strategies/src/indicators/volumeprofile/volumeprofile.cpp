 /**
 * @brief SiiS tick volume profile indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-02
 */

#include "siis/indicators/volumeprofile/volumeprofile.h"
#include "siis/utils/common.h"

using namespace siis;
using o3d::Logger;
using o3d::Debug;


VolumeProfile::VolumeProfile(const o3d::String &name,
                             o3d::Double timeframe, o3d::Int32 history,
                             o3d::Double sensibility,
                             o3d::Double valueArea,
                             o3d::Bool computePeaksAndValleys) :
    Indicator(name, timeframe)
{

}

VolumeProfile::VolumeProfile(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe)
{

}

void VolumeProfile::setConf(IndicatorConfig conf)
{

}

void VolumeProfile::update(o3d::Double timestamp, const Tick &tick, o3d::Bool finalize)
{

}

