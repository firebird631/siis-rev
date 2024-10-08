/**
 * @brief SiiS strategy order signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_ORDERSIGNAL_H
#define SIIS_ORDERSIGNAL_H

#include "basesignal.h"
#include "../order.h"
#include "../constants.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief Strategy order signal proxy.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API OrderSignal : public BaseSignal<Order>
{
public:

    enum Event {
        OPENED = 0,
        UPDATED = 1,
        DELETED = 2,
        REJECTED = 3,
        CANCELED = 4,
        TRADED = 5
    };

    OrderSignal(Event _event) :
        BaseSignal(ORDER),
        event(_event),
        completed(false)
    {
    }

    Event event;             //!< type of the event.

    o3d::Bool completed;     //!< true if fully filled (or treated as it)
};

} // namespace siis

#endif // SIIS_ORDERSIGNAL_H
