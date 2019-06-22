/**
 * @brief SiiS strategy standard trades manager.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#include "siis/trade/stdtrademanager.h"
#include "siis/database/tradedb.h"
#include "siis/strategy.h"

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

void StdTradeManager::addTrade(Trade *trade)
{
    if (trade) {
        m_trades.push_back(trade);
    }
}

void StdTradeManager::removeTrade(Trade *trade)
{
    if (trade) {
        m_trades.remove(trade);
    }
}

void StdTradeManager::process(o3d::Double timestamp)
{
    std::list<Trade*> m_deleted;

    m_mutex.lock();

    for (Trade *trade : m_trades) {
        // process dynamique update of the trade, like managed stop-loss, take-profit in taker when defined,
        // statistics update, and cleanup remaining trade

        // @todo more or less as in python trademanager
    }

    for (Trade *trade : m_deleted) {
        // @todo as in python trademanager
        m_trades.remove(trade);
    }

    m_mutex.unlock();
}

o3d::Bool StdTradeManager::hasTrade(o3d::Int32 id) const
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

o3d::Bool StdTradeManager::hasTrade(o3d::Double timeframe) const
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
