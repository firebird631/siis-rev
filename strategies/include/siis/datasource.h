/**
 * @brief SiiS strategy data source.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_DATASOURCE_H
#define SIIS_DATASOURCE_H

#include "base.h"

#include <o3d/core/base.h>

namespace siis {

class SIIS_API DataSource
{
public:

    enum Type {
        TICK = 0,
        OHLC_MID = 1,
        OHLC_BID = 2,
        OHLC_OFR = 3,
        ORDER_BOOK = 4
    };

    Type type;               //!< type of data source
    o3d::Double timeframe;   //!< timeframe for OHLC
    o3d::Int32 depth;        //!< depth for order book

    DataSource(Type _type, o3d::Double _timeframe = 0.0, o3d::Int32 _depth = 0) :
        type(_type),
        timeframe(_timeframe),
        depth(_depth) {}

    DataSource(const DataSource &dup) :
        type(dup.type),
        timeframe(dup.timeframe),
        depth(dup.depth) {}

    DataSource& operator= (const DataSource &dup) {
        type = dup.type;
        timeframe = dup.timeframe;
        depth = dup.depth;

        return *this;
    }
};

} // namespace siis

#endif // SIIS_DATASOURCE_H
