/**
 * @brief SiiS strategy standard trades manager.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#include "siis/trade/stdtrademanager.h"
#include "siis/database/tradedb.h"
#include "siis/strategy.h"
#include "siis/connector/traderproxy.h"
#include "siis/handler.h"

using namespace siis;

StdTradeManager::StdTradeManager(Strategy *strategy) :
    m_strategy(strategy)
{

}

StdTradeManager::~StdTradeManager()
{

}

void StdTradeManager::acquire() const
{
    m_mutex.lock();
}

void StdTradeManager::release() const
{
    m_mutex.unlock();
}

void StdTradeManager::terminate()
{
    m_mutex.lock();

    for (Trade *trade : m_trades) {
        // free trade
        strategy()->handler()->traderProxy()->freeTrade(trade);
    }

    m_mutex.unlock();
}

void StdTradeManager::addTrade(Trade *trade)
{
    if (trade) {
        m_trades.push_back(trade);
    }
}

void StdTradeManager::removeTrade(Trade *trade)
{
    if (trade) {
        m_mutex.lock();
        m_trades.remove(trade);
        m_mutex.unlock();

        // free trade
        strategy()->handler()->traderProxy()->freeTrade(trade);
    }
}

void StdTradeManager::process(o3d::Double timestamp)
{
    std::list<Trade*> removed_trades;

    m_mutex.lock();

    for (Trade *trade : m_trades) {
        m_mutex.unlock();

        // process exit conditions of the trade (breakeven, dynamic stop/tp, market close condition)
        trade->process(timestamp);

        // statistics update
        trade->updateStats(m_strategy->market()->last(), timestamp);

        // purge closed or canceled trades
        if (trade->canDelete()) {
            removed_trades.push_back(trade);
        }

        m_mutex.lock();
    }

    for (Trade *trade : removed_trades) {
        o3d::String msg = o3d::String("#{0} {1} exit at p={2} pl={3}% {4}").arg(trade->id())
                          .arg(trade->direction() > 0 ? "long" : "short")
                          .arg(trade->strategy()->market()->formatPrice(trade->exitPrice()))
                          .arg(trade->profitLossRate()*100.0, 2)
                          .arg(trade->profitLossRate() > 0 ? "WIN" : "LOSS");
        m_strategy->log(trade->tf(), "trade-exit", msg);

        m_trades.remove(trade);

        // for statistics
        strategy()->addClosedTrade(trade);

        // free trade
        strategy()->handler()->traderProxy()->freeTrade(trade);
    }

    m_mutex.unlock();
}

o3d::Bool StdTradeManager::hasTrades() const
{
    o3d::Bool result = false;
    m_mutex.lock();

    result = !m_trades.empty();

    m_mutex.unlock();
    return result;
}

o3d::Bool StdTradeManager::hasTradesByDirection(o3d::Int32 dir) const
{
    o3d::Bool result = false;
    m_mutex.lock();

    for (Trade *trade : m_trades) {
        if (trade->direction() == dir) {
            result = true;
            break;
        }
    }

    m_mutex.unlock();
    return result;
}

o3d::Bool StdTradeManager::hasTradeById(o3d::Int32 id) const
{
    o3d::Bool result = false;
    m_mutex.lock();

    for (Trade *trade : m_trades) {
        if (trade->id() == id) {
            result = true;
            break;
        }
    }

    m_mutex.unlock();
    return result;
}

o3d::Int32 StdTradeManager::numTrades() const
{
    m_mutex.lock();
    o3d::Int32 n = static_cast<o3d::Int32>(m_trades.size());
    m_mutex.unlock();

    return n;
}

o3d::Int32 StdTradeManager::numActiveTrades() const
{
    m_mutex.lock();
    o3d::Int32 n = static_cast<o3d::Int32>(m_trades.size());

    for (Trade *trade : m_trades) {
        if (trade->isClosed() || trade->isClosing()) {
            --n;
        }
    }
    m_mutex.unlock();

    return n;
}

Trade *StdTradeManager::getTrade(o3d::Int32 id)
{
    Trade *result = nullptr;
    m_mutex.lock();

    for (Trade *trade : m_trades) {
        if (trade->id() == id) {
            result = trade;
            break;
        }
    }

    m_mutex.unlock();
    return result;
}

const Trade *StdTradeManager::getTrade(o3d::Int32 id) const
{
    const Trade *result = nullptr;
    m_mutex.lock();

    for (const Trade *trade : m_trades) {
        if (trade->id() == id) {
            result = trade;
            break;
        }
    }

    m_mutex.unlock();
    return result;
}

o3d::Bool StdTradeManager::hasTradeByTimeframe(o3d::Double timeframe) const
{
    o3d::Bool found = false;
    m_mutex.lock();

    for (const Trade *trade : m_trades) {
        if (trade->timeframe() == timeframe) {
            found = true;
            break;
        }
    }

    m_mutex.unlock();
    return found;
}

Trade *StdTradeManager::findTrade(o3d::Double timeframe)
{
    Trade *result = nullptr;
    m_mutex.lock();

    for (Trade *trade : m_trades) {
        if (trade->timeframe() == timeframe) {
            result = trade;
            break;
        }
    }

    m_mutex.unlock();
    return result;
}

const Trade *StdTradeManager::findTrade(o3d::Double timeframe) const
{
    const Trade *result = nullptr;
    m_mutex.lock();

    for (const Trade *trade : m_trades) {
        if (trade->timeframe() == timeframe) {
            result = trade;
            break;
        }
    }

    m_mutex.unlock();
    return result;
}

void StdTradeManager::computePerformance(
        o3d::Double &performance,
        o3d::Double &drawDown,
        o3d::Int32 &pending,
        o3d::Int32 &actives) const
{
    o3d::Bool drawDownByPercentage = true;

    performance = 0.0;
    drawDown = 0.0;
    pending = 0;
    actives = 0;

    m_mutex.lock();

    for (Trade *trade : m_trades) {
        if (trade->isActive()) {
            ++actives;

            drawDown += trade->stats().unrealizedProfitLoss;
            performance += trade->estimateProfitLossRate();
        } else {
            ++pending;
        }
    }

    if (drawDownByPercentage) {
        if (performance < 0.0) {
            drawDown = -performance;
        }
    } else {
        if (drawDown < 0.0) {
            o3d::Double freeMargin = strategy()->handler()->traderProxy()->freeMargin();
            if (freeMargin > 0.0) {
                drawDown = -drawDown / freeMargin;
            }
        } else {
            drawDown = 0.0;
        }
    }

    m_mutex.unlock();
}

o3d::Int32 StdTradeManager::closeAll()
{
    o3d::Int32 n = 0;

    m_mutex.lock();

    for (Trade *trade : m_trades) {
        if (trade->isActive()) {
            // found : apply
            m_mutex.lock();
            trade->close(TradeStats::REASON_CLOSE_MARKET);

            ++n;

            m_mutex.unlock();
        } else if (!trade->isActive()) {
            m_mutex.lock();
            trade->cancelOpen();

            ++n;

            m_mutex.unlock();
        }
    }

    m_mutex.unlock();

    return n;
}

o3d::Int32 StdTradeManager::closeAllByDirection(o3d::Int32 dir)
{
    o3d::Int32 n = 0;

    m_mutex.lock();

    for (Trade *trade : m_trades) {
        if (trade->direction() != dir) {
            continue;
        }

        if (trade->isActive()) {
            // found : close
            m_mutex.unlock();
            trade->close(TradeStats::REASON_CLOSE_MARKET);

            ++n;

            m_mutex.lock();
        } else if (!trade->isActive()) {
            // found : cancel
            m_mutex.unlock();
            trade->cancelOpen();

            ++n;

            m_mutex.lock();
        }
    }

    m_mutex.unlock();

    return n;
}

void StdTradeManager::onOrderSignal(const OrderSignal &orderSignal)
{
    m_mutex.lock();

    for (Trade *trade : m_trades) {
        if (trade->isTargetOrder(orderSignal.orderId, orderSignal.refId)) {
            // found : apply
            m_mutex.unlock();
            trade->orderSignal(orderSignal);
            return;
        }
    }

    m_mutex.unlock();
}

void StdTradeManager::onPositionSignal(const PositionSignal &positionSignal)
{
    m_mutex.lock();

    for (Trade *trade : m_trades) {
        if (trade->isTargetPosition(positionSignal.positionId, positionSignal.orderRefId)) {
            // found : apply
            m_mutex.unlock();
            trade->positionSignal(positionSignal);
            return;
        }
    }

    m_mutex.unlock();
}

void StdTradeManager::saveTrades(TradeDb *tradeDb)
{
    m_mutex.lock();

    for (const Trade *trade : m_trades) {
        if (trade->isActive()) {
            tradeDb->storeTrade(m_strategy->brokerId(), m_strategy->market()->marketId(), m_strategy->identifier(), *trade);
        }
    }

    m_mutex.unlock();
}

void StdTradeManager::loadTrades(TradeDb *tradeDb, TraderProxy *traderProxy)
{
    m_mutex.lock();

    // @todo

    m_mutex.unlock();
}
