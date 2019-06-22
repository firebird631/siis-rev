/**
 * @brief SiiS strategy position signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_POSITIONSIGNAL_H
#define SIIS_POSITIONSIGNAL_H

#include "basesignal.h"
#include "../position.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief Strategy position signal proxy.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API PositionSignal : public BaseSignal<Position>
{
public:

    enum Event {
        OPENED = 0,
        UPDATED = 1,
        DELETED = 2
    };

    PositionSignal(Event _event) :
        BaseSignal(POSITION),
        event(_event)
    {
    }

    Event event;             //!< type of the event.
};

} // namespace siis

#endif // SIIS_POSITIONSIGNAL_H
