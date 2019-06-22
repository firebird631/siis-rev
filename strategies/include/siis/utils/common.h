/**
 * @brief SiiS common utils.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#ifndef SIIS_COMMON_H
#define SIIS_COMMON_H

#include "../base.h"
#include "../constants.h"

#include <o3d/core/string.h>

namespace siis {

SIIS_API o3d::Double timeframeFromStr(const o3d::String &tf);

SIIS_API o3d::String timeframeToStr(o3d::Double tf);

SIIS_API o3d::String directionToStr(o3d::Int32 dir);

SIIS_API o3d::String marketContractToStr(o3d::Int32 marketContract);

SIIS_API o3d::String marketUnitToStr(o3d::Int32 marketUnit);

SIIS_API o3d::String marketTypeToStr(o3d::Int32 marketType);

SIIS_API o3d::String maTypeToStr(MAType maType);

SIIS_API MAType maTypeFromStr(const o3d::String maType);

SIIS_API o3d::Double baseTime(o3d::Double timestamp, o3d::Double timeframe);

SIIS_API o3d::String taErrorToStr(o3d::Int32 error);

/**
 * @brief cmpTimeframe Compare 2 timeframe.
 * @return -1 if a is lesser, 0 if equal, 1 if a is greater.
 */
inline SIIS_API o3d::Int32 cmpTimeframe(o3d::Double a, o3d::Double b)
{
    if (o3d::Int32(a) == o3d::Int32(b)) {
        return 0;
    } else if (a < b) {
        return -1;
    } else {
        return 1;
    }
}

} // namespace siis

#endif // SIIS_COMMON_H
