/**
 * @brief SiiS strategy local connector implementation for market based on FIFO margin position.
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

o3d::Int32 LocalConnector::_execFifoMarginOrder(Order *order, const Market *market,
                                                o3d::Double openExecPrice, o3d::Double closeExecPrice)
{
    // only perform entry order (not close, reduce)
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

    // list any positions for market order them by FIFO and execute the quantity over them

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

        return Order::RET_ERROR;
    } else {
        // create a new position and increase quantity
        // @todo

        return Order::RET_ERROR;
    }
}
