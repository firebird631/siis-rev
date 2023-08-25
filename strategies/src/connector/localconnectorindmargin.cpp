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

void LocalConnector::execIndMarginOrder(Order *order, const Market *market,
                                        o3d::Double openExecPrice, o3d::Double closeExecPrice)
{
    // only perform entry order (not close, reduce)
    Strategy *strategy = order->strategy;
    if (strategy == nullptr) {
        return;
    }

    if (strategy->tradeType() != Trade::TYPE_IND_MARGIN) {
        // only for indivisble margin trade
        return;
    }

    if (order->hedging() && !market->hedging()) {
        // do not support hedging
        return;
    }

    // either no position exists, must open one increase qty
    // either a position exists, must increase (same direction) or reduce (opposite direction) it fully or partially
    // when a position exists a reversal is possible if quantity in opposite direction is greater than current quantity

    // lookup for an existing position
    Position *position = nullptr;

    // @todo
    if (position) {
        // compare direction to know if increase or decrease (eventually close) position
        if (position->direction == order->direction) {
            // increase qty because of same direction
        } else if (order->orderQuantity == position->quantity) {
            // close position
        } else if (order->orderQuantity < position->quantity) {
            // reduce position but not close it
        } else if (order->orderQuantity > position->quantity) {
            // position reversal
        }
    } else {
        // create a new position and increase quantity
        // @todo
    }
/*
    // always a new position with the same order as order id for a position
    Position *position = traderProxy()->newPosition(strategy);

    // execution price at open
    o3d::Double execPrice = openExecPrice;

    position->positionId = order->orderId;
    position->refOrderId = order->refId;
    position->direction = order->direction;
    position->marketId = order->marketId;
    position->created = handler()->timestamp();
    position->updated = handler()->timestamp();

    position->stopPrice = order->stopPrice;
    position->limitPrice = order->limitPrice;

    m_virtualPositions[position->positionId] = position;

    // need a position opened signal
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

    OrderSignal deletedOrderSignal(OrderSignal::DELETED);
    deletedOrderSignal.direction = order->direction;
    deletedOrderSignal.marketId = order->marketId;
    deletedOrderSignal.executed = handler()->timestamp();
    deletedOrderSignal.orderId = order->orderId;
    deletedOrderSignal.refId = order->refId;
    deletedOrderSignal.orderType = order->orderType;
    deletedOrderSignal.flags = order->flags;
    // @todo do we set cumulative, avg and completed here ?

    strategy->onOrderSignal(deletedOrderSignal);*/
}
