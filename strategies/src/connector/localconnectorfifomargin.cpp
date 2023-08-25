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

void LocalConnector::_execFifoMarginOrder(Order *order, const Market *market,
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
}
