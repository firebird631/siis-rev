/**
 * @brief SiiS strategy standard trades manager.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#ifndef SIIS_STDTRADEMANAGER_H
#define SIIS_STDTRADEMANAGER_H

#include "trademanager.h"

#include <o3d/core/mutex.h>

#include <list>

namespace siis {

class Strategy;

/**
 * @brief Strategy standard implementation of the trades manager.
 * @author Frederic Scherma
 * @date 2019-03-17
 * @todo Uses a std::list of trades, but this could be optimized.
 */
class SIIS_API StdTradeManager : public TradeManager<StdTradeManager>
{
public:

    StdTradeManager(Strategy *strategy);

    ~StdTradeManager();

    void acquire() const;
    void release() const;

    void addTrade(Trade *trade);
    void removeTrade(Trade *trade);

    void process(o3d::Double timestamp);

    o3d::Bool hasTrades() const;
    o3d::Bool hasTradesByDirection(o3d::Int32 dir) const;
    o3d::Bool hasTradeById(o3d::Int32 id) const;

    Trade* getTrade(o3d::Int32 id);
    const Trade* getTrade(o3d::Int32 id) const;

    o3d::Bool hasTradeByTimeframe(o3d::Double timeframe) const;

    Trade* findTrade(o3d::Double timeframe);
    const Trade* findTrade(o3d::Double timeframe) const;

    void closeAll(o3d::Double timestamp, o3d::Double price);
    void closeAllByDirection(o3d::Int32 dir, o3d::Double timestamp, o3d::Double price);

    void onOrderSignal(const OrderSignal &orderSignal);
    void onPositionSignal(const PositionSignal &positionSignal);

    void saveTrades(TradeDb *tradeDb);
    void loadTrades(TradeDb *tradeDb, TraderProxy *traderProxy);

protected:

    o3d::FastMutex m_mutex;
    Strategy *m_strategy;

    std::list<Trade*> m_trades;
};

} // namespace siis

#endif // SIIS_STDTRADEMANAGER_H
