/**
 * @brief SiiS strategy trades manager interface.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#ifndef SIIS_TRADEMANAGER_H
#define SIIS_TRADEMANAGER_H

#include "trade.h"

namespace siis {

class Strategy;
class TradeDb;
class TradeReport;
class Cache;

/**
 * @brief Strategy trades manager template interface.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
template <class T>
class SIIS_API_TEMPLATE TradeManager
{
public:

    //
    // global
    //

    Strategy* strategy() { return T::strategy(); }
    const Strategy* strategy() const { return T::strategy(); }

    /**
     * @brief acquire Acquire a lock on the manager and all its trades.
     */
    void acquire() const { T::acquire(); }

    /**
     * @brief release Release a previous lock on the manager and all its trades.
     */
    void release() const { T::release(); }

    /**
     * @brief terminate Properly cleanup resources.
     */
    void terminate() { T::terminate(); }

    //
    // management
    //

    /**
     * @brief addTrade Add a new trade and set its unique id.
     */
    void addTrade(Trade *trade) { T::addTrade(trade); }

    /**
     * @brief removetrade Remove (delete) a trade from this manager, must be previously closed correctly.
     */
    void removeTrade(Trade *trade) { T::removeTrade(trade); }

    /**
     * @brief process Process the management of the active trade. It can update the trade state, statistics,
     * process a managed taker stop-loss or take-profit (closing the trade).
     * @param timestamp Current timestamp.
     */
    void process(o3d::Double timestamp) { T::process(timestamp); }

    o3d::Int32 numTrades() const { return T::numTrades(); }
    o3d::Int32 numActiveTrades() const { return T::numActiveTrades(); }

    o3d::Bool hasTrades() const { return T::hasTrades(); }

    o3d::Bool hasTradesByDirection(o3d::Int32 dir) const { return T::hasTradesByDirection(dir); }

    o3d::Bool hasTradeById(o3d::Int32 id) const { return T::hasTradeById(id); }

    Trade* getTrade(o3d::Int32 id) { return T::getTrade(id); }
    const Trade* getTrade(o3d::Int32 id) const { return T::getTrade(id); }

    o3d::Bool hasTradeByTimeframe(o3d::Double timeframe) const { return T::hasTradeByTimeframe(timeframe); }

    Trade* findTrade(o3d::Double timeframe) { return T::findTrade(timeframe); }
    const Trade* findTrade(o3d::Double timeframe) const { return T::findTrade(timeframe); }

    void computePerformance(o3d::Double &performance,
                            o3d::Double &drawDownRate,
                            o3d::Double &drawDown,
                            o3d::Int32 &pending,
                            o3d::Int32 &actives) const {
        return T::computePerformance(performance, drawDownRate, drawDown, pending, actives);
    }

    /**
     * @brief closeAll Close all trades at market.
     * @param timestamp Last timestamp
     * @param price Last market price.
     */
    o3d::Int32 closeAll(o3d::Double timestamp, o3d::Double price) { return T::closeAll(timestamp, price); }

    /**
     * @brief closeAllByDirection Similar as closeAll by only trade from a specific direction.
     * @param dir
     * @param timestamp
     * @param price
     */
    o3d::Int32 closeAllByDirection(o3d::Int32 dir, o3d::Double timestamp, o3d::Double price) {
        return T::closeAllByDirection(timestamp, price);
    }

    //
    // signals
    //

    /**
     * @brief onOrderSignal Try to update a trade after an order creation/execution/modification/cancelation/deletion.
     */
    void onOrderSignal(const OrderSignal &orderSignal) { return T::onOrderSignal(orderSignal); }

    /**
     * @brief onPositionSignal Try to update a trade after a position creation/update/modification/deletion.
     */
    void onPositionSignal(const PositionSignal &positionSignal) { return T::onPositionSignal(positionSignal); }

    //
    // persistency
    //

    /**
     * @brief saveTrades Save actives trade into the DB to restore them at next startup.
     */
    void saveTrades(TradeDb *tradeDb) { T::saveTrades(tradeDb); }

    /**
     * @brief loadTrades Reload previously saved actives trade from the DB and reinstanciate them as possible
     * from existing order/position once they are fetched from the trader proxy (primary connector).
     */
    void loadTrades(TradeDb *tradeDb, TraderProxy *traderProxy) { T::loadTrades(tradeDb, traderProxy); }

    //
    // statistics/reporting
    //

    void notifyTrade(Cache *cache, Trade *trade) { T::notifyTrade(cache, trade); }

    void updateReport(Trade *trade) { T::updateReport(trade); }

    const TradeReport& report() const { T::report(); }
};

} // namespace siis

#endif // SIIS_TRADEMANAGER_H
