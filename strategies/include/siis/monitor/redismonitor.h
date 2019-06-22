/**
 * @brief SiiS strategy redis monitor connector implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-26
 */

#ifndef SIIS_REDISMONITOR_H
#define SIIS_REDISMONITOR_H

#include "monitor.h"

namespace siis {

/**
 * @brief SiiS strategy redis monitor connector implementation.
 * @author Frederic Scherma
 * @date 2019-03-26
 */
class SIIS_API RedisMonitor : public Monitor
{
public:

    RedisMonitor();

    virtual ~RedisMonitor() override;

    virtual void start() override;
    virtual void stop() override;

    virtual void connect() override;
    virtual void disconnect() override;

    // @todo publish
    // virtual void publishLog(...) override;
    // virtual void publishTrade(...) override;
    // virtual void publishOhlc(...) override;
    // virtual void publishTick(...) override;
};

} // namespace siis

#endif // SIIS_REDISMONITOR_H
