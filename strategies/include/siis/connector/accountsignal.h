/**
 * @brief SiiS strategy account balance signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-21
 */

#ifndef SIIS_ACCOUNTSIGNAL_H
#define SIIS_ACCOUNTSIGNAL_H

#include "basesignal.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief Strategy account balance signal proxy.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API AccountSignal : public BaseSignal<o3d::NullClass>
{
public:

    enum Event{
        UPDATED = 0
    };

    AccountSignal(Event _event) :
        BaseSignal(ACCOUNT),
        event(_event),
        timestamp(TIMESTAMP_UNDEFINED),
        freeMargin(QUANTITY_UNDEFINED),
        reservedMargin(QUANTITY_UNDEFINED),
        marginFactor(QUANTITY_UNDEFINED)
    {
    }

    Event event;
    o3d::Double timestamp;

    o3d::Double freeMargin;
    o3d::Double reservedMargin;
    o3d::Double marginFactor;
};

} // namespace siis

#endif // SIIS_ACCOUNTSIGNAL_H
