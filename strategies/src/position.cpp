/**
 * @brief SiiS strategy position signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/position.h"
#include "siis/market.h"
#include "siis/connector/traderproxy.h"
#include "siis/strategy.h"

using namespace siis;

void Position::updatePnl(const Market *market)
{
    if (!market || market->bid() <= 0 || market->ask() <= 0) {
        return;
    }

    if (avgPrice == PRICE_UNDEFINED) {
        return;
    }

    o3d::Double positionCost = market->effectiveCost(quantity, avgPrice);
    o3d::Double lastPrice = avgPrice;

    if (direction > 0) {
        lastPrice = market->bid();
    } else if (direction < 0) {
        lastPrice = market->ask();
    }

    o3d::Double rawProfitLoss = market->computePnl(quantity, direction, avgPrice, lastPrice);

    // without fees neither commissions
    local.rawProfitLoss = rawProfitLoss;
    local.rawProfitLossRate = positionCost > 0.0 ? (local.rawProfitLoss / positionCost) : 0.0;

    // use maker fee and commission
    local.profitLoss = rawProfitLoss - (positionCost * market->makerFee().rate) - market->makerFee().commission;
    local.profitLossRate = positionCost > 0.0 ? (local.profitLoss / positionCost) : 0.0;

    // use taker fee and commission
    local.profitLossMarket = rawProfitLoss - (positionCost * market->takerFee().rate) - market->takerFee().commission;
    local.profitLossMarketRate = positionCost > 0.0 ? (local.profitLossMarket / positionCost) : 0.0;

    profitLoss = local.profitLoss;
}
