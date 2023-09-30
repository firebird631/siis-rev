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
    EntryExit(),
    m_breakevenDistance(0.0),
    m_breakevenPriceType(PRICE_NONE),
    m_breakevenDistanceType(DIST_NONE)
{

}

void DynamicStopLoss::init(const Market *market, const ContextConfig &conf)
{
    EntryExit::init(market, conf.dynamicStopLoss());

    // needed if configured
    BreakevenConfig breakevenConfig = conf.breakeven();
    if (breakevenConfig.data().isMember("type")) {
        o3d::String type = breakevenConfig.data().get("type", Json::Value()).asCString();
        if (type == "fixed") {
            m_breakevenPriceType = PRICE_FIXED;
        } else if (type == "fixed-pct") {
            m_breakevenPriceType = PRICE_FIXED;
        } else if (type == "fixed-dist") {
            m_breakevenPriceType = PRICE_FIXED;
        } else if (type == "custom") {
            m_breakevenPriceType = PRICE_CUSTOM;
        }
    }

    if (breakevenConfig.data().isMember("distance")) {
        if (breakevenConfig.data().isMember("distance")) {
            o3d::String distance = breakevenConfig.data().get("distance", Json::Value()).asCString();
            if (distance.endsWith("%")) {
                distance.trimRight('%');
                m_breakevenDistance = distance.toDouble() * 0.01;
                m_breakevenDistanceType = DIST_PERCENTIL;
            } else if (distance.endsWith("pip")) {
                distance.trimRight("pip");
                m_breakevenDistance = distance.toDouble() * market->onePipMean();
                m_breakevenDistanceType = DIST_PRICE;
            } else if (distance.endsWith("pips")) {
                distance.trimRight("pips");
                m_breakevenDistance = distance.toDouble() * market->onePipMean();
                m_breakevenDistanceType = DIST_PRICE;
            } else {
                m_breakevenDistance = distance.toDouble();
                m_breakevenDistanceType = DIST_PRICE;
            }
        }
    }
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

void DynamicStopLoss::updateΤrade(o3d::Double timestamp, o3d::Double lastTimestamp, Trade *trade) const
{
    if (!trade) {
        return;
    }

    // at least a breakeven distance if defined else it is 0
    if (m_breakevenDistance > 0.0) {
        if (m_breakevenDistanceType == DIST_PERCENTIL) {
            if (trade->estimateProfitLossRate() <= m_breakevenDistance) {
                return;
            }
        } else if (m_breakevenDistanceType == DIST_PRICE) {
            if (trade->deltaPrice() <= trade->direction() * m_breakevenDistance) {
                return;
            }
        }
    } else {
        if (trade->estimateProfitLossRate() <= 0.0) {
            return;
        }
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

o3d::Bool DynamicStopLoss::checkMinDistance(Trade *trade) const
{
    if (m_distanceType == DIST_PERCENTIL) {
            o3d::Double closeExecPrice = trade->strategy()->market()->closeExecPrice(trade->direction());

        if (distanceFromPercentile(trade, closeExecPrice) > m_distance) {
            return true;
        }
    } else if (m_distanceType == DIST_PRICE) {
        o3d::Double closeExecPrice = trade->strategy()->market()->closeExecPrice(trade->direction());

        if (distanceFromPrice(trade, closeExecPrice) >= m_distance) {
            return true;
        }
    }

    return false;
}
