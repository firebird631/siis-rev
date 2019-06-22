/**
 * @brief SiiS strategy worker.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/worker.h"
#include "siis/poolworker.h"
#include "siis/strategy.h"

#include <o3d/core/application.h>

using namespace siis;

Worker::Worker(PoolWorker *poolWorker, o3d::Int32 id) :
    m_id(id),
    m_poolWorker(poolWorker),
    m_running(false),
    m_ping(false),
    m_thread(this)
{
    O3D_ASSERT(m_poolWorker != nullptr);
}

Worker::~Worker()
{

}

o3d::Int32 Worker::run(void *)
{
    while (m_running) {
        PoolWorker::Job *job = m_poolWorker->nextJob();
        if (job != nullptr) {
            job->strategy->process(job->timestamp);

            if (job->countDown) {
                job->countDown->done();
            }

            o3d::deletePtr(job);   // @todo recycle it
        } else {
            // don't waste the CPU
            // o3d::System::waitMs(1);
        }

        // o3d::System::print(o3d::String("Worker {0}").arg(m_id), "Run");

        if (m_ping) {
            m_ping = false;
            pong();
        }
    }

    return 0;
}

void Worker::ping()
{
    m_ping = true;
}

void Worker::start()
{
    if (!m_running) {
        m_running = true;
        m_thread.start();
    }
}

void Worker::stop()
{
    if (m_running) {
        m_running = false;
        m_thread.stop();
    }
}

#include <ncurses.h>
void Worker::pong()
{
    // @todo need terminal display helper, or use the log and monitor could receive that info
    addstr(o3d::String("Worker {0} pong").arg(m_id).toUtf8().getData());
}
