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

    /**
     * @brief acquire Acquire a lock on the manager and all its trades.
     */
    void acquire() const { T::acquire(); }

    /**
     * @brief release Release a previous lock on the manager and all its trades.
     */
    void release() const { T::release(); }

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

    o3d::Bool hasTrade(o3d::Int32 id) const { return T::hasTrade(id); }

    Trade* getTrade(o3d::Int32 id) { return T::getTrade(id); }
    const Trade* getTrade(o3d::Int32 id) const { return T::getTrade(id); }

    o3d::Bool hasTrade(o3d::Double timeframe) const { return T::hasTrade(timeframe); }

    Trade* findTrade(o3d::Double timeframe) { return T::findTrade(timeframe); }
    const Trade* findTrade(o3d::Double timeframe) const { return T::findTrade(timeframe); }

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
