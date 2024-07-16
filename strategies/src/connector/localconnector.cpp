/**
 * @brief SiiS strategy local connector implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-06-03
 */

#include "siis/connector/localconnector.h"
#include "siis/connector/traderproxy.h"

#include "siis/handler.h"
#include "siis/strategy.h"
#include "siis/config/config.h"
#include "siis/connector/ordersignal.h"
#include "siis/statistics/statistics.h"

#include <o3d/core/debug.h>
#include <o3d/core/uuid.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;


LocalConnector::LocalConnector(Handler *handler) :
    m_handler(handler),
    m_running(false),
    m_traderProxy(nullptr)
{

}

LocalConnector::~LocalConnector()
{
    stop();
}

void LocalConnector::start()
{
    if (!m_running) {
        m_running = true;
    }
}

void LocalConnector::stop()
{
    if (m_running) {
        m_running = false;
    }
}

void LocalConnector::update()
{
    m_mutex.lock();

    if (!m_virtualOrders.empty()) {
        for (auto it = m_virtualOrders.begin(); it != m_virtualOrders.end(); ++it) {
            // check and execute order...
            Order *order = it->second;
            const Market *market = order->strategy->market();
            o3d::Bool done = false;  // filled or rejected or canceled but processed

            try {
                if (order->orderType == Order::ORDER_LIMIT) {
                    done = _handleLimitOrder(order, market);
                }
                else if (order->orderType == Order::ORDER_STOP) {
                    done = _handleStopOrder(order, market);
                }
                else if (order->orderType == Order::ORDER_STOP_LIMIT) {
                    done = _handleStopLimitOrder(order, market);
                }
                else if (order->orderType == Order::ORDER_TAKE_PROFIT) {
                    done = _handleTakeProfitOrder(order, market);
                }
                else if (order->orderType == Order::ORDER_TAKE_PROFIT_LIMIT) {
                    done = _handleTakeProfitLimitOrder(order, market);
                }
            } catch (o3d::E_BaseException &e) {
            }

            if (done) {
                m_removedOrders.push_back(order);
            }
        }
    }

    // update positions
    if (m_virtualPositions.empty()) {
        for (auto it = m_virtualPositions.begin(); it != m_virtualPositions.end(); ++it) {
            Position *position = it->second;
            const Market *market = position->strategy->market();
            o3d::Bool done = false;

            try {
                // trigger position limit/stop for position only
                if (market->hasPosition()) {
                    _updatePosition(position, market);
                }

                // update profit/loss for stats
                position->updatePnl(market);
            } catch (o3d::E_BaseException &e) {
            }

            if (done) {
                m_removedPositions.push_back(position);
            }
        }
    }

    // update virtual account
    m_virtualAccount.updateBalance();
    m_virtualAccount.updateDrawDown();
    m_virtualAccount.dailyUpdate(handler()->timestamp());

    // cleanup resources
    if (!m_removedOrders.empty()) {
        for (auto it = m_removedOrders.begin(); it != m_removedOrders.end(); ++it) {
            // and remove from pending orders
            auto mit = m_virtualOrders.find((*it)->orderId);
            if (mit != m_virtualOrders.end()) {
                m_virtualOrders.erase(mit);
            }

            // clean non removed orders
            m_traderProxy->freeOrder(*it);
        }

        m_removedOrders.clear();
    }

    if (!m_removedPositions.empty()) {
        for (auto it = m_removedPositions.begin(); it != m_removedPositions.end(); ++it) {
            // and remove from actives positions
            auto mit = m_virtualPositions.find((*it)->positionId);
            if (mit != m_virtualPositions.end()) {
                m_virtualPositions.erase(mit);
            }

            // clean non removed positions
            m_traderProxy->freePosition(*it);
        }

        m_removedPositions.clear();
    }

    m_mutex.unlock();
}

void LocalConnector::connect()
{
    // nothing
}

void LocalConnector::disconnect()
{
    // nothing
}

void LocalConnector::subscribe(Subscription &sub)
{
    // nothing
}

void LocalConnector::unsubscribe(Subscription &sub)
{
    // nothing
}

void LocalConnector::unsubscribeAll()
{
    // nothing
}

void LocalConnector::setTraderProxy(TraderProxy *traderProxy)
{
    m_traderProxy = traderProxy;
}

TraderProxy *LocalConnector::traderProxy()
{
    return m_traderProxy;
}

const TraderProxy *LocalConnector::traderProxy() const
{
    return m_traderProxy;
}

Handler *LocalConnector::handler()
{
    return m_handler;
}

const Handler *LocalConnector::handler() const
{
    return m_handler;
}

void LocalConnector::init(Config *config)
{
    if (config) {
        m_virtualAccount.balance = config->getInitialBalance();
        m_virtualAccount.initialBalance = config->getInitialBalance();
        m_virtualAccount.currency = config->getInitialCurrency();

        // setup precision
        if (m_virtualAccount.currency == "USD" || m_virtualAccount.currency == "USDT") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "USD" || m_virtualAccount.currency == "USDT" || m_virtualAccount.currency == "ZUSD") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "EUR" || m_virtualAccount.currency == "ZEUR") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "CAD" || m_virtualAccount.currency == "ZCAD") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "JPY" || m_virtualAccount.currency == "ZJPY") {
            m_virtualAccount.precision = 2;
        } else if (m_virtualAccount.currency == "BTC" || m_virtualAccount.currency == "XBT") {
            m_virtualAccount.precision = 8;
        } else if (m_virtualAccount.currency == "ETH") {
            m_virtualAccount.precision = 8;
        } else if (m_virtualAccount.currency == "BNB") {
            m_virtualAccount.precision = 8;
        } else if (m_virtualAccount.currency == "XRP") {
            m_virtualAccount.precision = 4;
        } else {
            m_virtualAccount.precision = 2;
        }
    }
}

void LocalConnector::terminate()
{
    for (auto it = m_virtualOrders.begin(); it != m_virtualOrders.end(); ++it) {
        // clean non closed orders
        m_traderProxy->freeOrder(it->second);
    }

    m_virtualOrders.clear();

    for (auto it = m_virtualPositions.begin(); it != m_virtualPositions.end(); ++it) {
        // clean non closed positions
        m_traderProxy->freePosition(it->second);
    }

    m_virtualPositions.clear();

    if (!m_removedOrders.empty()) {
        WARN("memory", o3d::String("{0} virtual orders are not removed").arg(
                           static_cast<o3d::Int32>(m_removedOrders.size())));

        for (auto it = m_removedOrders.begin(); it != m_removedOrders.end(); ++it) {
            // clean non removed orders
            m_traderProxy->freeOrder(*it);
        }

        m_removedOrders.clear();
    }

    if (!m_removedPositions.empty()) {
        WARN("memory", o3d::String("{0} virtual positions are not removed").arg(
                           static_cast<o3d::Int32>(m_removedPositions.size())));

        for (auto it = m_removedPositions.begin(); it != m_removedPositions.end(); ++it) {
            // clean non removed positions
            m_traderProxy->freePosition(*it);
        }

        m_removedPositions.clear();
    }
}

void LocalConnector::fetchAnyOrders()
{
    // nothing
}

void LocalConnector::fetchOrder(const o3d::CString &marketId)
{
    // nothing
}

void LocalConnector::fetchAnyPositions()
{
    // nothing
}

void LocalConnector::fetchPositions(const o3d::CString &marketId)
{
    // nothing
}

o3d::Int32 LocalConnector::createOrder(Order *order)
{
    if (m_traderProxy) {
        if (!order) {
            return Order::RET_INVALID_ARGS;
        }

        // either execute a market order or add it for later execution (limit, stop...)
        if (order->orderType == Order::ORDER_MARKET) {
            // direct execution and return
            Strategy *strategy = order->strategy;
            const Market* market = strategy->market();

            const o3d::Double openExecPrice = market->openExecPrice(order->direction);
            const o3d::Double closeExecPrice = market->closeExecPrice(order->direction);

            o3d::Int32 res = Order::RET_UNDEFINED;

            // set a unique order identifier
            order->orderId = o3d::Uuid::uuid5("siis").toCString();
            order->created = handler()->timestamp();

            m_mutex.lock();

            if (strategy->tradeType() == Trade::TYPE_SPOT) {
                res = _execAssetOrder(order, market, openExecPrice, closeExecPrice);
            } else if (strategy->tradeType() == Trade::TYPE_MARGIN) {
                res = _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
            } else if (strategy->tradeType() == Trade::TYPE_IND_MARGIN) {
                res = _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
            } else if (strategy->tradeType() == Trade::TYPE_POSITION) {
                res = _execPositionOrder(order, market, openExecPrice, closeExecPrice);
            } else {
                res = Order::RET_INVALID_ARGS;
            }

            // finally free the order because it is fully executed
            m_traderProxy->freeOrder(order);

            m_mutex.unlock();

            return res;

        } else {
            // check and insert for later execution
            order->orderId = o3d::Uuid::uuid5("siis").toCString();

            // direct order signal to strategy
            Strategy *strategy = order->strategy;

            OrderSignal openOrderSignal(OrderSignal::OPENED);
            openOrderSignal.direction = order->direction;
            openOrderSignal.marketId = order->marketId;
            openOrderSignal.created = handler()->timestamp();
            openOrderSignal.orderId = order->orderId;
            openOrderSignal.refId = order->refId;

            openOrderSignal.orderType = order->orderType;
            openOrderSignal.orderQuantity = order->orderQuantity;
            openOrderSignal.orderPrice = order->orderPrice;
            openOrderSignal.orderStopPrice = order->orderStopPrice;
            openOrderSignal.flags = order->flags;

            m_mutex.lock();
            m_virtualOrders[order->orderId] = order;
            m_mutex.unlock();

            strategy->onOrderSignal(openOrderSignal);

            return Order::RET_OK;
        }
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Int32 LocalConnector::cancelOrder(const o3d::CString &orderId)
{
    if (m_traderProxy) {
        if (orderId.isValid()) {
            o3d::FastMutexLocker _(m_mutex);

            auto it = m_virtualOrders.find(orderId);
            if (it != m_virtualOrders.end()) {
                Order *order = it->second;
                Strategy *strategy = order->strategy;

                OrderSignal cancelOrderSignal(OrderSignal::CANCELED);
                cancelOrderSignal.executed = handler()->timestamp();
                cancelOrderSignal.orderId = orderId;
                cancelOrderSignal.refId = order->refId;
                cancelOrderSignal.orderType = order->orderType;

                strategy->onOrderSignal(cancelOrderSignal);

                // finally free the order because it is fully executed
                m_virtualOrders.erase(it);
                m_traderProxy->freeOrder(order);

                return Order::RET_OK;
            }
        }

        return Order::RET_INVALID_ARGS;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Int32 LocalConnector::closePosition(const o3d::CString &positionId,
                                         o3d::Int32 direction,
                                         o3d::Double quantity,
                                         o3d::Bool taker,
                                         o3d::Double limitPrice)
{
    // @note only taker and no limit price supported for now
    // @note only for unique position
    if (m_traderProxy) {
        if (positionId.isValid()) {
            o3d::FastMutexLocker _(m_mutex);

            auto it = m_virtualPositions.find(positionId);
            if (it != m_virtualPositions.end()) {
                Position *position = it->second;
                Strategy *strategy = position->strategy;  // @warn not sure to have it on live
                const Market *market = strategy->market();

                o3d::Double closeExecPrice = market->closeExecPrice(position->direction);
                o3d::Int32 res = Order::RET_ERROR;

                if (market->hasPosition()) {
                    if (quantity > 0 && quantity < position->quantity) {
                        res = _reducePosition(position, market, closeExecPrice);
                    } else {
                        res = _closePosition(position, market, closeExecPrice);
                    }
                }

                if (res == Order::RET_OK) {
                    // finally free the position because it is fully executed
                    m_virtualPositions.erase(it);
                    m_traderProxy->freePosition(position);
                }

                return res;
            }
        }

        return Order::RET_INVALID_ARGS;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

void LocalConnector::fetchAccount()
{
    if (m_traderProxy) {
        // @todo could set initial account margin from configuration
    }
}

void LocalConnector::fetchAnyAssets()
{
    if (m_traderProxy) {
        // @todo could set initial assets quantities from configuration
    }
}

void LocalConnector::fetchAssets(const o3d::CString &assetId)
{
    if (m_traderProxy) {
        // @todo could set initial asset quantity from configuration
    }
}

void LocalConnector::finalAccountStats(AccountStatistics &accountStats) const
{
    accountStats.initialEquity = m_virtualAccount.initialBalance;
    accountStats.finalEquity = m_virtualAccount.balance;
    accountStats.maxDrawDown = m_virtualAccount.maxDrawDown;
    accountStats.maxDrawDownRate = m_virtualAccount.maxDrawDownRate;
    accountStats.profitLoss = m_virtualAccount.profitLoss;
    // @todo currency and precision for formatting

    // copy samples
    for (auto const &sample : m_virtualAccount.samples) {
        accountStats.samples.push_back(sample);
    }
}

o3d::Bool LocalConnector::_handleLimitOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_LIMIT) {
        return false;
    }

    if ((order->direction > 0 && market->ask() <= order->orderPrice) ||
        (order->direction < 0 && market->bid() >= order->orderPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        if (order->direction > 0) {
            openExecPrice = market->ask();
            closeExecPrice = market->bid();
        } else if (order->direction < 0) {
            openExecPrice = market->bid();
            closeExecPrice = market->ask();
        }

        if (market->hasPosition() && order->margin()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition() && order->margin()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->indivisiblePosition() && order->margin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot() && !order->margin()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Bool LocalConnector::_handleStopOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_STOP) {
        return false;
    }

    if ((order->direction > 0 && market->bid() >= order->stopPrice) ||
        (order->direction < 0 && market->ask() <= order->stopPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        if (order->direction > 0) {
            openExecPrice = market->ask();
            closeExecPrice = market->bid();
        } else if (order->direction < 0) {
            openExecPrice = market->bid();
            closeExecPrice = market->ask();
        }

        if (market->hasPosition() && order->margin()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition() && order->margin()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->indivisiblePosition() && order->margin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot() && !order->margin()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Bool LocalConnector::_handleStopLimitOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_STOP_LIMIT) {
        return false;
    }

    if ((order->direction > 0 && market->bid() >= order->stopPrice) ||
        (order->direction < 0 && market->ask() <= order->stopPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        // because of a limit price take the best
        if (order->direction > 0) {
            openExecPrice = o3d::min(order->orderPrice, market->ask());
            closeExecPrice = o3d::min(order->orderPrice, market->bid());
        } else if (order->direction < 0) {
            openExecPrice = o3d::max(order->orderPrice, market->bid());
            closeExecPrice = o3d::max(order->orderPrice, market->ask());
        }

        if (market->hasPosition() && order->margin()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition() && order->margin()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->indivisiblePosition() && order->margin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot() && !order->margin()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Bool LocalConnector::_handleTakeProfitOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_TAKE_PROFIT) {
        return false;
    }

    // act as a stop but in opposite direction
    if ((order->direction > 0 && market->bid() <= order->stopPrice) ||
        (order->direction < 0 && market->ask() >= order->stopPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        if (order->direction > 0) {
            openExecPrice = market->ask();
            closeExecPrice = market->bid();
        } else if (order->direction < 0) {
            openExecPrice = market->bid();
            closeExecPrice = market->ask();
        }

        if (market->hasPosition() && order->margin()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition() && order->margin()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->indivisiblePosition() && order->margin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot() && !order->margin()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Bool LocalConnector::_handleTakeProfitLimitOrder(Order *order, const Market *market)
{
    if (order == nullptr || order->orderType != Order::ORDER_TAKE_PROFIT_LIMIT) {
        return false;
    }

    // act as a stop but in opposite direction and the as a limit order
    if ((order->direction > 0 && market->bid() <= order->stopPrice) ||
        (order->direction < 0 && market->ask() >= order->stopPrice)) {

        o3d::Double openExecPrice = 0.0;
        o3d::Double closeExecPrice = 0.0;

        // because of a limit price take the best
        if (order->direction > 0) {
            openExecPrice = o3d::min(order->orderPrice, market->ask());
            closeExecPrice = o3d::min(order->orderPrice, market->bid());
        } else if (order->direction < 0) {
            openExecPrice = o3d::max(order->orderPrice, market->bid());
            closeExecPrice = o3d::max(order->orderPrice, market->ask());
        }

        if (market->hasPosition() && order->margin()) {
            _execPositionOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->fifoPosition() && order->margin()) {
            _execFifoMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->indivisiblePosition() && order->margin()) {
            _execIndMarginOrder(order, market, openExecPrice, closeExecPrice);
        } else if (market->hasSpot() && !order->margin()) {
            _execAssetOrder(order, market, openExecPrice, closeExecPrice);
        }

        return true;
    }

    return false;
}

o3d::Int32 LocalConnector::modifyPosition(const o3d::CString &positionId,
        o3d::Double stopPrice,
        o3d::Double limitPrice)
{
    if (m_traderProxy) {
        // direct execution and return
        m_mutex.lock();

        Position *position = nullptr;

        auto mit = m_virtualPositions.find(positionId);
        if (mit != m_virtualPositions.end()) {
            position = mit->second;
        }

        if (position == nullptr) {
            // invalid position id
            return Order::RET_ERROR;
        }

        Strategy *strategy = position->strategy;
        const Market* market = strategy->market();

        if (!market->hasPosition()) {
            // only supported for position type
            return Order::RET_ERROR;
        }

        if (limitPrice >= 0.0) {
            position->limitPrice = limitPrice;
        }

        if (stopPrice >= 0.0) {
            position->stopPrice = stopPrice;
        }

        return Order::RET_OK;
    } else {
        return Order::RET_UNREACHABLE_SERVICE;
    }
}

o3d::Double LocalConnector::VirtualAccountData::updateBalance()
{
    return 0.0;
}

void LocalConnector::VirtualAccountData::updateDrawDown()
{
    if (balance > 0 && profitLoss > 0) {
        o3d::Double ddRate = (profitLoss + assetProfitLoss) / balance;
        if (ddRate < 0.0) {
            drawDownRate = -ddRate;
            maxDrawDownRate = o3d::max(maxDrawDownRate, drawDownRate);
        }

        o3d::Double dd = profitLoss + assetProfitLoss;
        if (dd < 0.0) {
            drawDown = -dd;
            maxDrawDown = o3d::max(maxDrawDown, drawDown);
        }
    } else {
        drawDownRate = 0.0;
        drawDown = 0.0;
    }
}

void LocalConnector::VirtualAccountData::dailyUpdate(o3d::Double timestamp)
{
    o3d::Double currentBt = baseTime(timestamp, TF_DAY);

    if (samples.empty()) {
        // initial sample
        AccountSample sample;
        sample.timestamp = currentBt;
        sample.drawDown = drawDown;
        sample.drawDownRate = drawDownRate;
        sample.equity = balance;
        sample.profitLoss = profitLoss;

        samples.push_back(sample);
    }

    if (lastUpdateTimestamp > 0.0) {
        // add one sample per day
        o3d::Double previousBt = baseTime(lastUpdateTimestamp, TF_DAY);

        o3d::Int32 elapsedDays = o3d::min(999, static_cast<o3d::Int32>((currentBt - previousBt) / TF_DAY));
        if (elapsedDays > 0) {
            const AccountSample &prevSample = samples.back();

            while (elapsedDays-- > 0) {
                previousBt += TF_DAY;

                AccountSample sample;
                sample.timestamp = previousBt;
                sample.drawDown = prevSample.drawDown;
                sample.drawDownRate = prevSample.drawDownRate;
                sample.equity = prevSample.equity;
                sample.profitLoss = prevSample.profitLoss;

                samples.push_back(sample);
            }
        }

        // update current day
        AccountSample &currSample = samples.back();
        currSample.drawDown = drawDown;
        currSample.drawDownRate = drawDownRate;
        currSample.equity = balance;
        currSample.profitLoss = profitLoss;
    }

    lastUpdateTimestamp = timestamp;
}
