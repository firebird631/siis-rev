/**
 * @brief SiiS strategy worker.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_WORKER_H
#define SIIS_WORKER_H

#include <o3d/core/mutex.h>
#include <o3d/core/thread.h>
#include <o3d/core/runnable.h>

namespace siis {

class PoolWorker;

/**
 * @brief Worker for the pool executor.
 * @author Frederic Scherma
 * @date 2019-03-05
 */
class Worker : public o3d::Runnable
{
public:

    Worker(PoolWorker *poolWorker, o3d::Int32 id);
    virtual ~Worker();

    virtual o3d::Int32 run(void *);

    void ping();

    void start();
    void stop();

private:

    o3d::Int32 m_id;

    PoolWorker *m_poolWorker;
    bool m_running;
    bool m_ping;

    o3d::FastMutex m_mutex;
    o3d::Thread m_thread;

    void pong();
};

} // namespace siis

#endif // SIIS_WORKER_H
