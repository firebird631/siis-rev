/**
 * @brief SiiS strategy cache connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-08
 */

#ifndef SIIS_CACHE_H
#define SIIS_CACHE_H

#include "siis/base.h"

#include <o3d/core/string.h>
#include <o3d/core/thread.h>

namespace siis {

class TradeCache;

/**
 * @brief Strategy cache connector interface.
 * @author Frederic Scherma
 * @date 2019-06-08
 */
class SIIS_API Cache : public o3d::Runnable
{
public:

    static Cache* builder(
            const o3d::String &db,
            const o3d::String &host,
            o3d::UInt32 port,
            const o3d::String &name,
            const o3d::String &user,
            const o3d::String &pwd);

    Cache();

    virtual ~Cache() = 0;

    virtual void init() = 0;
    virtual void terminate() = 0;

    void start();
    void stop();

    TradeCache* trade();

protected:

    o3d::Thread m_thread;
    o3d::Bool m_running;

    TradeCache *m_trade;
};

} // namespace siis

#endif // SIIS_CACHE_H
