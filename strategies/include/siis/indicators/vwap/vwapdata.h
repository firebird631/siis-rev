 /**
 * @brief SiiS tick VWAP data model.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-18
 */

#ifndef SIIS_VWAPDATA_H
#define SIIS_VWAPDATA_H

#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS VWAP data model.
 * @author Frederic Scherma
 * @date 2024-08-18
 */
class SIIS_API VWapData
{
public:

    o3d::Double timestamp {0};     //!< open timestamp
    o3d::Double timeframe {0};     //!< or duration

    DataArray vwap;

    std::vector<DataArray> plusStdDev;
    std::vector<DataArray> minusStdDev;

    VWapData(o3d::Double vwapTimeframe, o3d::Double timestamp, o3d::Int32 numStdDev);

    const DataArray& stdDevAt(o3d::Int32 stdDev) const;
};

} // namespace siis

#endif // SIIS_VWAPDATA_H
