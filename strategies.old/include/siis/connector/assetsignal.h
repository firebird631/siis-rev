/**
 * @brief SiiS strategy asset signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-21
 */

#ifndef SIIS_ASSETSIGNAL_H
#define SIIS_ASSETSIGNAL_H

#include "basesignal.h"
#include "../constants.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief Strategy order signal proxy.
 * @author Frederic Scherma
 * @date 2019-03-21
 * @todo Could have offuscated detail in ratio and not as real value if the client want to keep confidentaility
 * But in that case we have to have a flag, and to managed the ordered quantity in ratio, and then the connector
 * support that too.
 */
class SIIS_API AssetSignal : public BaseSignal
{
public:

    enum Event{
        UPDATED = 0
    };

    AssetSignal(Event _event) :
        BaseSignal(ASSET),
        event(_event),
        timestamp(TIMESTAMP_UNDEFINED),
        freeQuantity(QUANTITY_UNDEFINED),
        lockedQuantity(QUANTITY_UNDEFINED)
    {
    }

    Event event;
    o3d::Double timestamp;

    o3d::String symbol;
    o3d::Double freeQuantity;
    o3d::Double lockedQuantity;
};

} // namespace siis

#endif // SIIS_ASSETSIGNAL_H
