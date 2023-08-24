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


void LocalConnector::updatePosition(Position *position, const Market *market)
{
    if (position->quantity <= 0.0) {
        return;
    }

    if (!market->hasPosition()) {
        return;
    }

    // only for individual positions with attached limit/stop prices
    o3d::Double closeExecPrice = market->closeExecPrice(position->direction);
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
        // position signal
        o3d::Double execQty = position->quantity;
        o3d::Double execPrice = closeExecPrice;

        // @todo fill and send position signal

        // position->avgPrice = execPrice;  // fully executed
        // position->execPrice = execPrice;
        // position->local.exitPrice = closeExecPrice;
        // position->quantity = 0;
    }
}

void LocalConnector::execPositionOrder(Order *order, const Market *market,
                                       o3d::Double openExePrice, o3d::Double closeExePrice)
{
    // only perform entry order (not close, reduce)
    Strategy *strategy = order->strategy;
    if (strategy == nullptr) {
        return;
    }

    if (strategy->tradeType() != Trade::TYPE_POSITION) {
        // only for position trade
        return;
    }

    if (!order->hedging()) {
        // support only hedging
        return;
    }

    // always a new position with the same order as order id for a position
    Position *position = traderProxy()->newPosition(strategy);

    // execution price at open
    o3d::Double execPrice = openExePrice;

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

    strategy->onOrderSignal(deletedOrderSignal);
}
