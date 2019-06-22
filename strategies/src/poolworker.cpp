/**
 * @brief SiiS strategy worker.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/poolworker.h"
#include "siis/worker.h"

using namespace siis;

PoolWorker::PoolWorker(o3d::Int32 numWorkers) :
    m_numWorkers(numWorkers),
    m_workers(nullptr)
{
    O3D_ASSERT(m_numWorkers >= 1);
}

PoolWorker::~PoolWorker()
{
    terminate();
}

bool PoolWorker::init()
{
    m_workers = new Worker*[static_cast<size_t>(m_numWorkers)];

    for (o3d::Int32 i = 0; i < m_numWorkers; ++i) {
        m_workers[i] = new Worker(this, i);
    }

    for (o3d::Int32 i = 0; i < m_numWorkers; ++i) {
        m_workers[i]->start();
    }

    return true;
}

void PoolWorker::terminate()
{
    if (m_workers != nullptr) {
        for (o3d::Int32 i = 0; i < m_numWorkers; ++i) {
            m_workers[i]->stop();
        }

        for (o3d::Int32 i = 0; i < m_numWorkers; ++i) {
            o3d::deletePtr(m_workers[i]);
        }

        o3d::deleteArray(m_workers);
    }
}

PoolWorker::Job* PoolWorker::nextJob()
{
    Job *job = nullptr;

    m_mutex.lock();

    if (m_jobs.size()) {
        job = m_jobs.front();
        m_jobs.pop_front();
    }

    m_mutex.unlock();

    return job;
}

void PoolWorker::addJob(Strategy *strategy, o3d::Double timestamp, CountDown *countDown)
{
    O3D_ASSERT(strategy != nullptr);
    O3D_ASSERT(timestamp > 0);

    Job *job = new Job();

    job->strategy = strategy;
    job->timestamp = timestamp;
    job->countDown = countDown;

    m_mutex.lock();
    m_jobs.push_back(job);
    m_mutex.unlock();
}

void PoolWorker::ping()
{
    if (m_workers != nullptr) {
        for (o3d::Int32 i = 0; i < m_numWorkers; ++i) {
            m_workers[i]->ping();
        }
    }
}
