/**
 * @brief SiiS strategy local connector implementation for market based on indivisible margin position.
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

o3d::Int32 LocalConnector::_execIndMarginOrder(Order *order, const Market *market,
                                               o3d::Double openExecPrice, o3d::Double closeExecPrice)
{
    Strategy *strategy = order->strategy;
    if (strategy == nullptr) {
        return Order::RET_ERROR;
    }

    if (strategy->tradeType() != Trade::TYPE_IND_MARGIN) {
        // only for indivisble margin trade
        return Order::RET_ERROR;
    }

    if (order->hedging() && !market->hedging()) {
        // do not support hedging
        return Order::RET_ERROR;
    }

    // either no position exists, must open one increase qty
    // either a position exists, must increase (same direction) or reduce (opposite direction) it fully or partially
    // when a position exists a reversal is possible if quantity in opposite direction is greater than current quantity

    // lookup for an existing position
    Position *position = nullptr;

    // or market id for margin, try to retrieve if a position exists else new one
    auto pit = m_virtualPositions.find(market->marketId());
    if (pit != m_virtualPositions.end()) {
        position = pit->second;
    }

    if (position) {
        o3d::Int32 res = Order::RET_ERROR;

        // keep it but order would be deleted just after
        if (order->positionId.isEmpty()) {
            order->positionId = position->positionId;
        }

        // compare direction to know if increase or decrease (eventually close) position
        if (position->direction == order->direction) {
            // increase qty because on same direction
            res = _indMarginIncreasePosition(order, position, market, openExecPrice);

        } else if (order->orderQuantity <= position->quantity) {
            // close or reduce position
            res = _indMarginClosePosition(order, position, market, closeExecPrice);

        } else if (order->orderQuantity > position->quantity) {
            // position reversal
            res = _indMarginReversePosition(order, position, market, closeExecPrice);
        }

        if (position->quantity <= 0.0) {
            // delete the position
            PositionSignal deletedPositionSignal(PositionSignal::DELETED);

            deletedPositionSignal.direction = position->direction;
            deletedPositionSignal.marketId = position->marketId;
            deletedPositionSignal.created = position->created;
            deletedPositionSignal.updated = position->updated;
            deletedPositionSignal.refOrderId = order->refId;
            deletedPositionSignal.quantity = 0;
            deletedPositionSignal.positionId = position->positionId;

            strategy->onPositionSignal(deletedPositionSignal);

            m_traderProxy->freePosition(position);
            m_virtualPositions.erase(pit);
        }

        return res;

    } else {
        if (order->closeOnly() || order->reduceOnly()) {
            // not compatible with opening/increasing position
            return Order::RET_INVALID_ARGS;
        }

        // create a new position and increase quantity
        return _indMarginOpenPosition(order, market, openExecPrice);
    }

    return Order::RET_ERROR;
}

o3d::Int32 LocalConnector::_indMarginOpenPosition(Order* order, const Market *market, o3d::Double openExecPrice)
{
    Strategy *strategy = order->strategy;
    if (strategy == nullptr) {
        return Order::RET_ERROR;
    }

    if (order == nullptr) {
        return Order::RET_ERROR;
    }

    const o3d::Double execPrice = openExecPrice;
    const o3d::Double executed = handler()->timestamp();

    OrderSignal openOrderSignal(OrderSignal::OPENED);
    openOrderSignal.direction = order->direction;
    openOrderSignal.marketId = order->marketId;
    openOrderSignal.created = executed;
    openOrderSignal.orderId = order->orderId;
    openOrderSignal.refId = order->refId;
    openOrderSignal.orderType = order->orderType;
    openOrderSignal.flags = order->flags;

    // or market id for margin @todo support for hedging
    openOrderSignal.positionId = market->marketId();

    // order is accepted
    strategy->onOrderSignal(openOrderSignal);

    order->executed = executed;
    order->execPrice = execPrice;
    order->avgPrice = execPrice;
    order->filled = order->orderQuantity;
    order->cumulativeFilled = order->orderQuantity;

    OrderSignal tradedOrderSignal(OrderSignal::TRADED);
    tradedOrderSignal.direction = order->direction;
    tradedOrderSignal.marketId = order->marketId;
    tradedOrderSignal.executed = executed;
    tradedOrderSignal.orderId = order->orderId;
    tradedOrderSignal.refId = order->refId;
    tradedOrderSignal.orderType = order->orderType;
    tradedOrderSignal.flags = order->flags;

    tradedOrderSignal.avgPrice = execPrice;
    tradedOrderSignal.execPrice = execPrice;
    tradedOrderSignal.filled = order->orderQuantity;
    tradedOrderSignal.cumulativeFilled = order->orderQuantity;
    tradedOrderSignal.completed = true;

    // compute quote transacted @todo
    // tradedOrderSignal.quoteTransacted =

    // commission fees and its currency @todo
    // tradedOrderSignal.cumulativeCommissionAmount = tradedOrderSignal.FEE_UNDEFINED;
    // tradedOrderSignal.commissionAmount = tradedOrderSignal.FEE_UNDEFINED;

    strategy->onOrderSignal(tradedOrderSignal);

    // create a virtual position
    Position *position = nullptr;

    // or market id for margin, try to retrieve if a position exists else new one
    auto pit = m_virtualPositions.find(market->marketId());
    if (pit != m_virtualPositions.end()) {
        // retrive existing position but it means it is empty in that branch
        position = pit->second;
    } else {
        position = traderProxy()->newPosition(strategy);

        position->positionId = market->marketId();  // same as market id (only if no hedging)
        position->refOrderId = order->refId;
        position->direction = order->direction;
        position->marketId = order->marketId;
        position->created = executed;

        // register the position
        m_virtualPositions[position->positionId] = position;
    }

    // keep it but order would be deleted just after
    order->positionId = position->positionId;

    position->quantity = order->orderQuantity;  // fully-filled
    position->updated = executed;

    // local position data
    position->local.entryPrice = execPrice;
    position->local.entryQty = order->orderQuantity;  // 100% entry

    PositionSignal openPositionSignal(PositionSignal::OPENED);
    openPositionSignal.direction = order->direction;
    openPositionSignal.marketId = order->marketId;
    openPositionSignal.created = executed;
    openPositionSignal.updated = executed;
    openPositionSignal.refOrderId = order->refId;
    openPositionSignal.positionId = position->positionId;
    // openPositionSignal.commission @todo

    // entry fully traded
    openPositionSignal.avgPrice = execPrice;
    openPositionSignal.execPrice = execPrice;
    openPositionSignal.quantity = position->quantity;  // new position qty
    openPositionSignal.filled = order->orderQuantity;
    openPositionSignal.cumulativeFilled = order->orderQuantity;

    strategy->onPositionSignal(openPositionSignal);

    OrderSignal deletedOrderSignal(OrderSignal::DELETED);
    deletedOrderSignal.direction = order->direction;
    deletedOrderSignal.marketId = order->marketId;
    deletedOrderSignal.executed = executed;
    deletedOrderSignal.orderId = order->orderId;
    deletedOrderSignal.refId = order->refId;
    deletedOrderSignal.orderType = order->orderType;
    deletedOrderSignal.flags = order->flags;
    // @todo do we set cumulative, avg and completed here ?

    strategy->onOrderSignal(deletedOrderSignal);

    return Order::RET_OK;
}

o3d::Int32 LocalConnector::_indMarginIncreasePosition(Order *order, Position *position, const Market *market,
                                                      o3d::Double openExecPrice)
{
    Strategy *strategy = order->strategy;
    if (strategy == nullptr) {
        return Order::RET_ERROR;
    }

    if (order == nullptr) {
        return Order::RET_ERROR;
    }

    if (position == nullptr) {
        return Order::RET_ERROR;
    }

    // execution price at open
    const o3d::Double execPrice = openExecPrice;
    if (execPrice <= 0.0) {
        return Order::RET_ERROR;
    }

    const o3d::Double executed = handler()->timestamp();

    OrderSignal openOrderSignal(OrderSignal::OPENED);
    openOrderSignal.direction = order->direction;
    openOrderSignal.marketId = order->marketId;
    openOrderSignal.created = executed;
    openOrderSignal.orderId = order->orderId;
    openOrderSignal.refId = order->refId;
    openOrderSignal.orderType = order->orderType;
    openOrderSignal.flags = order->flags;

    // market id for margin @todo for hedging
    openOrderSignal.positionId = market->marketId();

    // order is accepted
    strategy->onOrderSignal(openOrderSignal);

    order->executed = executed;
    order->execPrice = execPrice;
    order->avgPrice = execPrice;
    order->filled = order->orderQuantity;
    order->cumulativeFilled = order->orderQuantity;

    OrderSignal tradedOrderSignal(OrderSignal::TRADED);
    tradedOrderSignal.direction = order->direction;
    tradedOrderSignal.marketId = order->marketId;
    tradedOrderSignal.executed = executed;
    tradedOrderSignal.orderId = order->orderId;
    tradedOrderSignal.refId = order->refId;
    tradedOrderSignal.orderType = order->orderType;
    tradedOrderSignal.flags = order->flags;

    tradedOrderSignal.avgPrice = execPrice;
    tradedOrderSignal.execPrice = execPrice;
    tradedOrderSignal.filled = order->orderQuantity;
    tradedOrderSignal.cumulativeFilled = order->orderQuantity;
    tradedOrderSignal.completed = true;

    // @todo compute quote transacted
    // tradedOrderSignal.quoteTransacted =

    // commission fees and its currency @todo
    // tradedOrderSignal.cumulativeCommissionAmount = tradedOrderSignal.FEE_UNDEFINED;
    // tradedOrderSignal.commissionAmount = tradedOrderSignal.FEE_UNDEFINED;

    strategy->onOrderSignal(tradedOrderSignal);

    PositionSignal updatedPositionSignal(PositionSignal::UPDATED);

    position->updated = executed;

    position->avgPrice = (position->avgPrice * position->quantity + execPrice * order->orderQuantity) / (
                             position->quantity + order->orderQuantity);
    position->quantity = market->adjustQty(position->quantity + order->orderQuantity);

    // exit size and qty does not change
    position->local.entryPrice = position->avgPrice;
    position->local.entryQty = position->quantity;

    updatedPositionSignal.direction = order->direction;
    updatedPositionSignal.marketId = order->marketId;
    updatedPositionSignal.created = position->created;
    updatedPositionSignal.updated = executed;
    updatedPositionSignal.refOrderId = order->refId;
    updatedPositionSignal.positionId = position->positionId;
    // openPositionSignal.commission @todo

    // increase traded
    updatedPositionSignal.avgPrice = execPrice;
    updatedPositionSignal.execPrice = execPrice;
    updatedPositionSignal.filled = order->orderQuantity;
    updatedPositionSignal.quantity = position->quantity;  // new position qty
    updatedPositionSignal.cumulativeFilled = order->orderQuantity;

    strategy->onPositionSignal(updatedPositionSignal);

    OrderSignal deletedOrderSignal(OrderSignal::DELETED);
    deletedOrderSignal.direction = order->direction;
    deletedOrderSignal.marketId = order->marketId;
    deletedOrderSignal.executed = executed;
    deletedOrderSignal.orderId = order->orderId;
    deletedOrderSignal.refId = order->refId;
    deletedOrderSignal.orderType = order->orderType;
    deletedOrderSignal.flags = order->flags;
    // @todo do we set cumulative, avg and completed here ?

    strategy->onOrderSignal(deletedOrderSignal);

    return Order::RET_OK;
}

o3d::Int32 LocalConnector::_indMarginClosePosition(Order *order, Position *position, const Market *market,
                                                   o3d::Double closeExecPrice)
{
    Strategy *strategy = order->strategy;
    if (strategy == nullptr) {
        return Order::RET_ERROR;
    }

    if (order == nullptr) {
        return Order::RET_ERROR;
    }

    if (position == nullptr) {
        return Order::RET_ERROR;
    }

    // execution price at close
    const o3d::Double execPrice = closeExecPrice;
    if (execPrice <= 0.0) {
        return Order::RET_ERROR;
    }

    const o3d::Double executed = handler()->timestamp();

    OrderSignal openOrderSignal(OrderSignal::OPENED);
    openOrderSignal.direction = order->direction;
    openOrderSignal.marketId = order->marketId;
    openOrderSignal.created = executed;
    openOrderSignal.orderId = order->orderId;
    openOrderSignal.refId = order->refId;
    openOrderSignal.orderType = order->orderType;
    openOrderSignal.flags = order->flags;

    // market id for margin @todo for hedging
    openOrderSignal.positionId = market->marketId();

    // order is accepted
    strategy->onOrderSignal(openOrderSignal);

    order->executed = executed;
    order->execPrice = execPrice;
    order->avgPrice = execPrice;
    order->filled = order->orderQuantity;
    order->cumulativeFilled = order->orderQuantity;

    OrderSignal tradedOrderSignal(OrderSignal::TRADED);
    tradedOrderSignal.direction = order->direction;
    tradedOrderSignal.marketId = order->marketId;
    tradedOrderSignal.executed = executed;
    tradedOrderSignal.orderId = order->orderId;
    tradedOrderSignal.refId = order->refId;
    tradedOrderSignal.orderType = order->orderType;
    tradedOrderSignal.flags = order->flags;

    tradedOrderSignal.avgPrice = execPrice;
    tradedOrderSignal.execPrice = execPrice;
    tradedOrderSignal.filled = order->orderQuantity;
    tradedOrderSignal.cumulativeFilled = order->orderQuantity;
    tradedOrderSignal.completed = true;

    // @todo compute quote transacted
    // tradedOrderSignal.quoteTransacted =

    // commission fees and its currency @todo
    tradedOrderSignal.cumulativeCommissionAmount = tradedOrderSignal.FEE_UNDEFINED;
    tradedOrderSignal.commissionAmount = tradedOrderSignal.FEE_UNDEFINED;

    strategy->onOrderSignal(tradedOrderSignal);

    PositionSignal updatedPositionSignal(PositionSignal::UPDATED);

    position->updated = executed;

    position->local.exitPrice = (position->local.exitPrice * position->local.exitQty +
                                 order->orderQuantity * execPrice) / (position->local.exitQty + order->orderQuantity);
    position->local.exitQty = market->adjustQty(position->local.exitQty + order->orderQuantity);

    // @todo check qty, compute avgPrice, execPrice, entryPrice, entryQty, exitPrice, exitQty
    if (position->quantity > order->orderQuantity) {
        position->quantity = market->adjustQty(position->quantity - order->orderQuantity);
    } else {
        position->quantity = 0.0;  // closed mean 0
    }

    // entry price does not change
    position->local.entryQty = position->quantity;

    updatedPositionSignal.direction = order->direction;
    updatedPositionSignal.marketId = order->marketId;
    updatedPositionSignal.created = position->created;
    updatedPositionSignal.updated = executed;
    updatedPositionSignal.refOrderId = order->refId;
    updatedPositionSignal.positionId = position->positionId;
    // openPositionSignal.commission @todo

    // exit traded
    updatedPositionSignal.avgPrice = execPrice;
    updatedPositionSignal.execPrice = execPrice;
    updatedPositionSignal.quantity = position->quantity;
    updatedPositionSignal.filled = order->orderQuantity;
    updatedPositionSignal.cumulativeFilled = order->orderQuantity;

    strategy->onPositionSignal(updatedPositionSignal);

    OrderSignal deletedOrderSignal(OrderSignal::DELETED);
    deletedOrderSignal.direction = order->direction;
    deletedOrderSignal.marketId = order->marketId;
    deletedOrderSignal.executed = executed;
    deletedOrderSignal.orderId = order->orderId;
    deletedOrderSignal.refId = order->refId;
    deletedOrderSignal.orderType = order->orderType;
    deletedOrderSignal.flags = order->flags;
    // @todo do we set cumulative, avg and completed here ?

    strategy->onOrderSignal(deletedOrderSignal);

    return Order::RET_OK;
}

o3d::Int32 LocalConnector::_indMarginReversePosition(Order *order, Position *position, const Market *market,
                                                     o3d::Double closeExecPrice)
{
    if (order->closeOnly() || order->reduceOnly()) {
        // cannot reverse the position then simply close it
        return _indMarginClosePosition(order, position, market, closeExecPrice);
    }

    Strategy *strategy = order->strategy;
    if (strategy == nullptr) {
        return Order::RET_ERROR;
    }

    if (order == nullptr) {
        return Order::RET_ERROR;
    }

    if (position == nullptr) {
        return Order::RET_ERROR;
    }

    // execution price at close
    const o3d::Double execPrice = closeExecPrice;
    if (execPrice <= 0.0) {
        return Order::RET_ERROR;
    }

    o3d::Double executed = handler()->timestamp();

    OrderSignal openOrderSignal(OrderSignal::OPENED);
    openOrderSignal.direction = order->direction;
    openOrderSignal.marketId = order->marketId;
    openOrderSignal.created = executed;
    openOrderSignal.orderId = order->orderId;
    openOrderSignal.refId = order->refId;
    openOrderSignal.orderType = order->orderType;
    openOrderSignal.flags = order->flags;

    // market id for margin @todo for hedging
    openOrderSignal.positionId = market->marketId();

    // order is accepted
    strategy->onOrderSignal(openOrderSignal);

    order->executed = executed;
    order->execPrice = execPrice;
    order->avgPrice = execPrice;
    order->filled = order->orderQuantity;
    order->cumulativeFilled = order->orderQuantity;

    OrderSignal tradedOrderSignal(OrderSignal::TRADED);
    tradedOrderSignal.direction = order->direction;
    tradedOrderSignal.marketId = order->marketId;
    tradedOrderSignal.executed = executed;
    tradedOrderSignal.orderId = order->orderId;
    tradedOrderSignal.refId = order->refId;
    tradedOrderSignal.orderType = order->orderType;
    tradedOrderSignal.flags = order->flags;

    tradedOrderSignal.avgPrice = execPrice;
    tradedOrderSignal.execPrice = execPrice;
    tradedOrderSignal.filled = order->orderQuantity;
    tradedOrderSignal.cumulativeFilled = order->orderQuantity;
    tradedOrderSignal.completed = true;

    // @todo compute quote transacted
    // tradedOrderSignal.quoteTransacted =

    // commission fees and its currency @todo
    // tradedOrderSignal.cumulativeCommissionAmount = tradedOrderSignal.FEE_UNDEFINED;
    // tradedOrderSignal.commissionAmount = tradedOrderSignal.FEE_UNDEFINED;

    strategy->onOrderSignal(tradedOrderSignal);

    PositionSignal updatedPositionSignal(PositionSignal::UPDATED);

    position->updated = executed;

    // reset previous exit
    position->local.exitPrice = 0;
    position->local.exitQty = 0;

    // reversed position qty
    o3d::Double oppositeQty = market->adjustQty(o3d::abs(position->quantity - order->orderQuantity));

    position->quantity = oppositeQty;
    position->avgPrice = execPrice;

    // new entry price
    position->local.entryPrice = execPrice;
    position->local.entryQty = oppositeQty;

    updatedPositionSignal.direction = order->direction;
    updatedPositionSignal.marketId = order->marketId;
    updatedPositionSignal.created = position->created;
    updatedPositionSignal.updated = executed;
    updatedPositionSignal.refOrderId = order->refId;
    updatedPositionSignal.positionId = position->positionId;
    // openPositionSignal.commission @todo

    // increase traded
    updatedPositionSignal.avgPrice = execPrice;
    updatedPositionSignal.execPrice = execPrice;
    updatedPositionSignal.filled = order->orderQuantity;
    updatedPositionSignal.quantity = position->quantity;  // new position qty
    updatedPositionSignal.cumulativeFilled = order->orderQuantity;

    strategy->onPositionSignal(updatedPositionSignal);

    OrderSignal deletedOrderSignal(OrderSignal::DELETED);
    deletedOrderSignal.direction = order->direction;
    deletedOrderSignal.marketId = order->marketId;
    deletedOrderSignal.executed = executed;
    deletedOrderSignal.orderId = order->orderId;
    deletedOrderSignal.refId = order->refId;
    deletedOrderSignal.orderType = order->orderType;
    deletedOrderSignal.flags = order->flags;
    // @todo do we set cumulative, avg and completed here ?

    strategy->onOrderSignal(deletedOrderSignal);

    return Order::RET_OK;
}
