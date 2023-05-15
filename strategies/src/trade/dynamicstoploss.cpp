/**
 * @brief Strategy trade dynnamic stop-loss or in profit handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-04
 */

#include "siis/trade/dynamicstoploss.h"
#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"
#include "siis/market.h"

using namespace siis;

DynamicStopLoss::DynamicStopLoss() :
    EntryExit()
{

}

void DynamicStopLoss::init(const Market *market, ContextConfig &conf)
{
    EntryExit::init(market, conf);
}

o3d::Double distanceFromPercentile(Trade *trade, o3d::Double closeExecPrice)
{
    if (trade->stopLossPrice() > 0.0) {
        return trade->direction() * (closeExecPrice - trade->stopLossPrice()) / trade->stopLossPrice();
    }

    return 0.0;
}

o3d::Double computeStopLossPriceFixedDistancePercentile(Trade *trade, o3d::Double distance, o3d::Double closeExecPrice)
{
    o3d::Double newStopLossPrice = 0.0;

    if (trade->direction() > 0) {
        newStopLossPrice = closeExecPrice * (1.0 - distance);

        if (newStopLossPrice > trade->stopLossPrice() && newStopLossPrice < closeExecPrice) {
            return newStopLossPrice;
        }
    } else if (trade->direction() < 0) {
        newStopLossPrice = closeExecPrice * (1.0 + distance);

        if (newStopLossPrice < trade->stopLossPrice() && newStopLossPrice > closeExecPrice) {
            return newStopLossPrice;
        }
    }

    return 0.0;
}

o3d::Double distanceFromPrice(Trade *trade, o3d::Double closeExecPrice)
{
    if (trade->stopLossPrice() > 0.0) {
        return trade->direction() * (closeExecPrice - trade->stopLossPrice());
    }

    return 0.0;
}

o3d::Double computeStopLossPriceFixedDistancePrice(Trade *trade, o3d::Double distance, o3d::Double closeExecPrice)
{
    o3d::Double newStopLossPrice = 0.0;

    if (trade->direction() > 0) {
        newStopLossPrice = closeExecPrice - distance;

        if (newStopLossPrice > trade->stopLossPrice() && newStopLossPrice < closeExecPrice) {
            return newStopLossPrice;
        }
    } else if (trade->direction() < 0) {
        newStopLossPrice = closeExecPrice + distance;

        if (newStopLossPrice < trade->stopLossPrice() && newStopLossPrice > closeExecPrice) {
            return newStopLossPrice;
        }
    }

    return 0.0;
}

void DynamicStopLoss::updateΤrade(o3d::Double timestamp, o3d::Double lastTimestamp, Trade *trade)
{
    if (!trade) {
        return;
    }

    if (trade->estimateProfitLossRate() <= 0.0) {
        return;
    }

    if (m_adjustPolicy == ADJ_PRICE) {
        // price change at anytime
        if (m_distanceType == DIST_PERCENTIL) {
            o3d::Double closeExecPrice = trade->strategy()->market()->closeExecPrice(trade->direction());

            if (distanceFromPercentile(trade, closeExecPrice) > m_distance) {
                o3d::Double newStopLossPrice = computeStopLossPriceFixedDistancePercentile(
                                                   trade, m_distance, closeExecPrice);

                if (newStopLossPrice > 0.0) {
                    trade->modifyStopLoss(newStopLossPrice, Trade::MOD_PREVIOUS);
                }
            }
        } else if (m_distanceType == DIST_PRICE) {
            o3d::Double closeExecPrice = trade->strategy()->market()->closeExecPrice(trade->direction());

            if (distanceFromPrice(trade, closeExecPrice) >= m_distance) {
                o3d::Double newStopLossPrice = computeStopLossPriceFixedDistancePrice(
                                                   trade, m_distance, closeExecPrice);

                if (newStopLossPrice > 0.0) {
                    trade->modifyStopLoss(newStopLossPrice, Trade::MOD_PREVIOUS);
                }
            }
        }
    } else if (m_adjustPolicy == ADJ_CLOSE) {
        // price change at a close
        if (m_consolidated) {
            if (m_distanceType == DIST_PERCENTIL) {
                o3d::Double closeExecPrice = trade->strategy()->market()->closeExecPrice(trade->direction());

                if (distanceFromPercentile(trade, closeExecPrice) > m_distance) {
                    o3d::Double newStopLossPrice = computeStopLossPriceFixedDistancePercentile(
                                                       trade, m_distance, closeExecPrice);

                    if (newStopLossPrice > 0.0) {
                        trade->modifyStopLoss(newStopLossPrice, Trade::MOD_PREVIOUS);
                    }
                }
            } else if (m_distanceType == DIST_PRICE) {
                o3d::Double closeExecPrice = trade->strategy()->market()->closeExecPrice(trade->direction());

                if (distanceFromPrice(trade, closeExecPrice) >= m_distance) {
                    o3d::Double newStopLossPrice = computeStopLossPriceFixedDistancePrice(
                                                       trade, m_distance, closeExecPrice);

                    if (newStopLossPrice > 0.0) {
                        trade->modifyStopLoss(newStopLossPrice, Trade::MOD_PREVIOUS);
                    }
                }
            }
        }
    } else if (m_adjustPolicy == ADJ_CUSTOM) {
        // custom method from strategy
    }
}
