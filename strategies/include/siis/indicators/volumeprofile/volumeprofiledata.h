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

    typedef std::unordered_map<o3d::Double, std::pair<o3d::Double, o3d::Double>> T_BinHashMap;
    typedef T_BinHashMap::iterator IT_BinHashMap;
    typedef T_BinHashMap::const_iterator CIT_BinHashMap;

    o3d::Double timestamp {0};     //!< open timestamp
    o3d::Double timeframe {0};     //!< or duration

    o3d::Double sensibility {1};

    o3d::Double valPrice {0};
    o3d::Double vahPrice {0};

    o3d::Double pocPrice {0};
    o3d::Double pocVolume {0};

    T_BinHashMap bins;

    DataArray peaks;
    DataArray valleys;
};

} // namespace siis

#endif // SIIS_VOLUMEPROFILEDATA_H
