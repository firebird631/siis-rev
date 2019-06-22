/**
 * @brief SiiS strategy broker/connector status signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-21
 */

#ifndef SIIS_STATUSSIGNAL_H
#define SIIS_STATUSSIGNAL_H

#include "basesignal.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief Strategy order signal proxy.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API StatusSignal : public BaseSignal
{
public:

    enum Event {
        CONN_ACQUIRED = 0,
        CONN_LOST = 1
    };

    StatusSignal(Event _event) :
        BaseSignal(STATUS),
        event(_event),
        timestamp(TIMESTAMP_UNDEFINED)
    {
    }

    Event event;
    o3d::Double timestamp;
};

} // namespace siis

#endif // SIIS_STATUSSIGNAL_H
