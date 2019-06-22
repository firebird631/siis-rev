/**
 * @brief SiiS strategy strategy optimizer process handler.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-28
 */

#ifndef SIIS_OPTIMIZER_H
#define SIIS_OPTIMIZER_H

#include "siis/handler.h"

#include <o3d/core/configfile.h>
#include <o3d/core/mutex.h>
#include <o3d/core/thread.h>
#include <o3d/core/stringmap.h>

namespace siis {

class Displayer;
class Monitor;
class Database;
class Optimizer;
class Strategy;
class TraderProxy;

/**
 * @brief SiiS strategy machine learning process handler.
 * @author Frederic Scherma
 * @date 2019-03-28
 */
class Optimization : public Handler, public o3d::Runnable
{
public:

    Optimization();
    virtual ~Optimization() override;

    virtual void init(
            Displayer *displayer,
            Config *config,
            StrategyCollection *collection,
            PoolWorker *poolWorker,
            Database *database,
            Cache *cache) override;

    virtual void terminate() override;

    virtual void start() override;
    virtual void stop() override;

    virtual void sync() override;

    virtual o3d::Double timestamp() const override;

    virtual const TraderProxy* traderProxy() const override;
    virtual TraderProxy* traderProxy() override;

    virtual void setPaperMode(o3d::Bool active) override;

    virtual void onTick(const o3d::String &marketId, const Tick &tick) override;
    virtual void onOhlc(const o3d::String &marketId, Ohlc::Type ohlcType, const Ohlc &ohlc) override;

    virtual Market* market(const o3d::String &marketId) override;
    virtual const Market* market(const o3d::String &marketId) const override;

    virtual Strategy* strategy(const o3d::String &marketId) override;
    virtual const Strategy* strategy(const o3d::String &marketId) const override;

    virtual Database* database() override;
    virtual Cache* cache() override;

    virtual void log(o3d::Double timeframe, const o3d::String &marketId, const o3d::String &channel, const o3d::String &msg) override;

private:

    virtual o3d::Int32 run(void *) override;

    o3d::Double m_timestamp;
    o3d::FastMutex m_mutex;
    o3d::Thread m_thread;
    o3d::Bool m_running;

    o3d::Double m_fromTs;
    o3d::Double m_toTs;
    o3d::Double m_curTs;
    o3d::Double m_timestep;

    struct OptimizerElt
    {
        class Market *market;
        class TickStream *tickStream;
        class Strategy *strategy;
        class Optimizer *optimizer;
        // class OhlcStream *ohlcStream;
    };

    o3d::StringMap<OptimizerElt> m_optimizers;

    Displayer *m_displayer;

    class Connector *m_connector;
    TraderProxy *m_traderProxy;

    class Monitor *m_monitor;

    PoolWorker *m_poolWorker;

    Database *m_database;
    Cache *m_cache;
};

} // namespace siis

#endif // SIIS_OPTIMIZER_H
