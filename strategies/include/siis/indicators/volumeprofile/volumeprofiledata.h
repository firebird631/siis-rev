 /**
 * @brief SiiS tick volume profile data model.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-02
 */

#ifndef SIIS_VOLUMEPROFILEDATA_H
#define SIIS_VOLUMEPROFILEDATA_H

#include "../../dataarray.h"

#include <unordered_map>

namespace siis {

/**
 * @brief SiiS volume profile data model.
 * @author Frederic Scherma
 * @date 2024-08-02
 */
class SIIS_API VolumeProfileData
{
public:

    o3d::Double valPric {0};
    o3d::Double vahPrice {0};

    o3d::Double pocPrice {0};

    std::unordered_map<o3d::Double, std::pair<o3d::Double, o3d::Double>> bins;

    DataArray peaks;
    DataArray valleys;
};

} // namespace siis

#endif // SIIS_VOLUMEPROFILEDATA_H
