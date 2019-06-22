/**
 * @brief SiiS strategy handler processing interface.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-10
 */

#ifndef SIIS_HANDLER_H
#define SIIS_HANDLER_H

#include "base.h"
#include "config/config.h"

#include "tick.h"
#include "ohlc.h"

namespace siis {

class Displayer;
class StrategyCollection;
class PoolWorker;
class Database;
class Cache;
class Market;
class Strategy;
class TraderProxy;

/**
 * @brief SiiS strategy handler processing interface.
 * @author Frederic Scherma
 * @date 2019-03-10
 */
class SIIS_API Handler
{
public:

    virtual ~Handler() = 0;

    virtual void init(
            Displayer *displayer,
            Config *config,
            StrategyCollection *collection,
            PoolWorker *poolWorker,
            Database *database,
            Cache *cache) = 0;

    virtual void terminate() = 0;

    /**
     * @brief start Start execution thread(s), do the sync in sync method.
     */
    virtual void start() = 0;

    /**
     * @brief stop Stop execution of the processing thread(s).
     */
    virtual void stop() = 0;

    /**
     * @brief sync Process necessary synchronizations with the main thread.
     */
    virtual void sync() = 0;

    /**
     * @brief timestamp Second unit timestamp with the best possible precision.
     */
    virtual o3d::Double timestamp() const = 0;

    /**
     * @brief traderProxy Unique trader proxy instance (read only).
     */
    virtual const TraderProxy* traderProxy() const = 0;

    /**
     * @brief traderProxy Unique trader proxy instance.
     */
    virtual TraderProxy* traderProxy() = 0;

    /**
     * @brief setPaperMode Turn on/off the paper mode.
     */
    virtual void setPaperMode(o3d::Bool active) = 0;

    /**
     * @brief onTick On tick received.
     * @param tick Reference on a valid tick.
     */
    virtual void onTick(const o3d::String &marketId, const Tick &tick) = 0;

    /**
     * @brief onOhlc On ohlc received.
     * @param ohlc Reference on a valid ohlc.
     */
    virtual void onOhlc(const o3d::String &marketId, Ohlc::Type ohlcType, const Ohlc &ohlc) = 0;

    /**
     * @brief onOrderBook On order book update received.
     * @param orderBook Reference on a valid order book change.
     */
    // virtual void onOrderBook(const o3d::String &marketId, const OrderBook &orderBook) = 0;

    /**
     * @brief market Get a market by its id, or null if not exists.
     */
    virtual Market* market(const o3d::String &marketId) = 0;

    /**
     * @brief market Get a market by its id, or null if not exists (read-only).
     */
    virtual const Market* market(const o3d::String &marketId) const = 0;

    /**
     * @brief strategy Get a strategy instance from its associated market-id.
     */
    virtual Strategy* strategy(const o3d::String &marketId) = 0;

    /**
     * @brief strategy Get a strategy instance from its associated market-id (read-only).
     */
    virtual const Strategy* strategy(const o3d::String &marketId) const = 0;

    /**
     * @brief database
     */
    virtual Database* database() = 0;

    /**
     * @brief cache
     */
    virtual Cache *cache() = 0;

    /**
     * @brief log Log a message throught the message logger of the handler.
     * @param timeframe Timeframe of the message or -1 if none.
     * @param marketId Related market id or empty if none.
     * @param channel Mapped name of the channel or empty if general.
     * @param msg Message content.
     */
    virtual void log(o3d::Double timeframe, const o3d::String &marketId, const o3d::String &channel, const o3d::String &msg) = 0;
};

} // namespace siis

#endif // SIIS_HANDLER_H
