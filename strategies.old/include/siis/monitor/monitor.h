/**
 * @brief SiiS strategy monitor connector interface.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-26
 */

#ifndef SIIS_MONITOR_H
#define SIIS_MONITOR_H

#include "../base.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief SiiS strategy monitor connector interface.
 * @author Frederic Scherma
 * @date 2019-03-26
 */
class SIIS_API Monitor
{
public:

    virtual ~Monitor() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void connect() = 0;
    virtual void disconnect() = 0;

    // @todo publish
    // virtual void publishLog(...) = 0;
    // virtual void publishTrade(...) = 0;
    // virtual void publishOhlc(...) = 0;
    // virtual void publishTick(...) = 0;
};

} // namespace siis

#endif // SIIS_MONITOR_H
