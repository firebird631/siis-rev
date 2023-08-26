/**
 * @brief SiiS strategy local connector implementation for market based on individual position.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-08-23
 */

#include "siis/connector/localconnector.h"
#include "siis/connector/traderproxy.h"

#include "siis/handler.h"
#include "siis/strategy.h"
#include "siis/config/config.h"
#include "siis/connector/ordersignal.h"

#include <o3d/core/debug.h>
#include <o3d/core/uuid.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;


void LocalConnector::_updatePosition(Position *position, const Market *market)
{
    if (position->quantity <= 0.0) {
        return;
    }

    if (!market->hasPosition()) {
        return;
    }

    // only for individual positions with attached limit/stop prices
    const o3d::Double closeExecPrice = market->closeExecPrice(position->direction);
    o3d::Int32 closedBy = 0;   // 0 none, 1 stop, 2 limit

    if (position->stopPrice > 0.0) {
        // execute stop at market
        if (position->direction > 0) {
            if (closeExecPrice <= position->stopPrice) {
                closedBy = 1;
            }
        } else if (position->direction < 0) {
            if (closeExecPrice >= position->stopPrice) {
                closedBy = 1;
            }
        }
    }

    if (position->limitPrice > 0.0) {
        // execute limit on maker
        if (position->direction > 0) {
            if (closeExecPrice >= position->limitPrice) {
                closedBy = 2;
            }
        } else if (position->direction < 0) {
            if (closeExecPrice <= position->limitPrice) {
                closedBy = 2;
            }
        }
    }

    if (closedBy > 0) {
        _closePosition(position, market, closeExecPrice);
    }
}

o3d::Int32 LocalConnector::_execPositionOrder(Order *order, const Market *market,
                                             o3d::Double openExecPrice, o3d::Double closeExecPrice)
{
    Strategy *strategy = order->strategy;
    if (strategy == nullptr) {
        return Order::RET_ERROR;
    }

    if (strategy->tradeType() != Trade::TYPE_POSITION) {
        // only for position trade
        return Order::RET_ERROR;
    }

    if (!order->hedging()) {
        // support only hedging
        return Order::RET_ERROR;
    }

    if (order->positionId.isEmpty()) {
        if (order->closeOnly() || order->reduceOnly()) {
            // not compatible with opening/increasing position
            return Order::RET_INVALID_ARGS;
        }

        // order is not related to a position, meaning create a new one
        return _createPosition(order, market, openExecPrice);
    } else {
        Position *position = nullptr;

        auto it = m_virtualPositions.find(order->positionId);
        if (it != m_virtualPositions.end()) {
            position = it->second;
        }

        // else retrieve the position and reduce or close it
        if (position != nullptr) {
            if (order->orderQuantity > 0 && order->orderQuantity < position->quantity) {
                return _reducePosition(position, market, closeExecPrice);
            } else {
                return _closePosition(position, market, closeExecPrice);
            }
        }

        return Order::RET_ERROR;
    }
}

o3d::Int32 LocalConnector::_createPosition(Order *order, const Market *market, o3d::Double openExecPrice)
{
    Strategy *strategy = order->strategy;
    if (strategy == nullptr) {
        return Order::RET_ERROR;
    }

    // always a new position with the same order as order id for a position
    Position *position = traderProxy()->newPosition(strategy);
    if (position == nullptr) {
        return Order::RET_ERROR;
    }

    // execution price at open
    const o3d::Double execPrice = openExecPrice;
    if (execPrice <= 0.0) {
        return Order::RET_ERROR;
    }

    // the order is normally in opened state

    // position and order id are the same
    position->positionId = order->orderId;
    position->refOrderId = order->refId;
    position->direction = order->direction;
    position->marketId = order->marketId;
    position->created = handler()->timestamp();
    position->updated = handler()->timestamp();

    position->quantity = order->orderQuantity;
    position->stopPrice = order->stopPrice;
    position->limitPrice = order->limitPrice;
    position->avgPrice = execPrice;
    position->execPrice = execPrice;
    position->profitCurrency = market->quote().symbol;

    // local position data
    position->local.entryPrice = execPrice;
    position->local.entryQty = order->orderQuantity;  // 100% entry

    order->positionId = position->positionId;

    m_virtualPositions[position->positionId] = position;

    PositionSignal openPositionSignal(PositionSignal::OPENED);
    openPositionSignal.direction = order->direction;
    openPositionSignal.marketId = order->marketId;
    openPositionSignal.created = handler()->timestamp();
    openPositionSignal.updated = handler()->timestamp();
    openPositionSignal.refOrderId = order->refId;
    openPositionSignal.positionId = position->positionId;
    // openPositionSignal.commission @todo

    // entry traded
    openPositionSignal.avgPrice = execPrice;
    openPositionSignal.execPrice = execPrice;
    openPositionSignal.filled = order->orderQuantity;
    openPositionSignal.cumulativeFilled = order->orderQuantity;

    strategy->onPositionSignal(openPositionSignal);

    // not necessary but could send a position opened signal

    OrderSignal deletedOrderSignal(OrderSignal::DELETED);
    deletedOrderSignal.direction = order->direction;
    deletedOrderSignal.marketId = order->marketId;
    deletedOrderSignal.executed = handler()->timestamp();
    deletedOrderSignal.orderId = order->orderId;
    deletedOrderSignal.refId = order->refId;
    deletedOrderSignal.orderType = order->orderType;
    deletedOrderSignal.flags = order->flags;
    // @todo do we set cumulative, avg and completed here ?

    strategy->onOrderSignal(deletedOrderSignal);

    return Order::RET_OK;
}

o3d::Int32 LocalConnector::_reducePosition(Position *position, const Market *market, o3d::Double closeExecPrice)
{
    if (position == nullptr) {
        return Order::RET_ERROR;
    }

    Strategy *strategy = position->strategy;
    if (strategy == nullptr) {
        return Order::RET_ERROR;
    }

    // execution price at open
    const o3d::Double execPrice = closeExecPrice;
    if (execPrice <= 0.0) {
        return Order::RET_ERROR;
    }

    // @todo

    DBG("", o3d::String("LocalConnector::_reducePosition !!TODO!! {0} at={1} pl={2}{3}")
                .arg(position->positionId).arg(execPrice).arg(position->local.profitLoss)
                .arg(o3d::String(position->profitCurrency)));

    return Order::RET_OK;
}

o3d::Int32 LocalConnector::_closePosition(Position *position, const Market *market, o3d::Double closeExecPrice)
{
    if (position == nullptr) {
        return Order::RET_ERROR;
    }

    Strategy *strategy = position->strategy;
    if (strategy == nullptr) {
        return Order::RET_ERROR;
    }

    // execution price at open
    const o3d::Double execPrice = closeExecPrice;
    if (execPrice <= 0.0) {
        return Order::RET_ERROR;
    }

    // @todo avg exit price according to local members in case _reducePosition usage

    // local position data
    position->local.exitPrice = execPrice;
    position->local.exitQty = position->local.entryQty;  // 100% exit
    position->updated = handler()->timestamp();

    // @todo last update and deleted signal
    PositionSignal deletedPositionSignal(PositionSignal::DELETED);

    deletedPositionSignal.direction = position->direction;
    deletedPositionSignal.marketId = position->marketId;
    deletedPositionSignal.created = position->created;
    deletedPositionSignal.updated = handler()->timestamp();
    deletedPositionSignal.refOrderId = position->refOrderId;
    deletedPositionSignal.positionId = position->positionId;
    // deletedPositionSignal.commission @todo

    // exit traded @todo correct qty
    deletedPositionSignal.avgPrice = execPrice;
    deletedPositionSignal.execPrice = execPrice;
    deletedPositionSignal.filled = position->quantity;
    deletedPositionSignal.cumulativeFilled = position->local.exitQty;

    strategy->onPositionSignal(deletedPositionSignal);

    position->updatePnl(market);
    position->quantity = 0;

//    DBG("", o3d::String("LocalConnector::_closePosition {0} at={1} pl={2}{3}")
//                .arg(position->positionId).arg(execPrice).arg(position->local.profitLoss)
//                .arg(o3d::String(position->profitCurrency)));

//    o3d::String msg = o3d::String("LocalConnector::_closePosition {0} at={1} pl={2}{3}")
//                .arg(position->positionId).arg(execPrice).arg(position->local.profitLoss)
//                .arg(o3d::String(position->profitCurrency));
//    handler()->log(0, market->marketId(), "local", msg);

    return Order::RET_OK;
}
