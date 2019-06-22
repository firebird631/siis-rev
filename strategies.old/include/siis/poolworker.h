/**
 * @brief SiiS strategy worker.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_POOLWORKER_H
#define SIIS_POOLWORKER_H

#include <o3d/core/mutex.h>
#include "worker.h"

#include <deque>

namespace siis {

class Strategy;

/**
 * @brief Pool of worker (parallelized jobs executions).
 * @author Frederic Scherma
 * @date 2019-03-05
 * @todo recycle new Job*
 */
class PoolWorker
{
public:

    struct CountDown
    {
        o3d::Int32 count{0};
        o3d::WaitCondition condition;
        o3d::FastMutex mutex;

        void wait()
        {
            mutex.lock();
            while (count > 0) {
                condition.wait(mutex);
            }
            mutex.unlock();
        }

        void done()
        {
            mutex.lock();
            --count;
            if (count <= 0) {
                condition.wakeAll();
            }
            mutex.unlock();
        }
    };

    struct Job
    {
        CountDown *countDown{nullptr};
        Strategy *strategy;
        o3d::Double timestamp;
    };

    PoolWorker(o3d::Int32 numWorker=8);
    virtual ~PoolWorker();

    bool init();
    void terminate();

    o3d::Int32 getNumWorkers() const { return m_numWorkers; }

    Job* nextJob();
    void addJob(Strategy *strategy, o3d::Double timestamp, CountDown *countDown = nullptr);

    void ping();

private:

    o3d::Int32 m_numWorkers;
    Worker **m_workers;

    std::deque<Job*> m_jobs;

    o3d::FastMutex m_mutex;
};

} // namespace siis

#endif // SIIS_POOLWORKER_H
