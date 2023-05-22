/**
 * @brief SiiS strategy trade initial stop-loss handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-22
 */

#include "siis/trade/stoploss.h"
#include "siis/config/strategyconfig.h"
#include "siis/market.h"
#include "siis/strategy.h"
#include "siis/handler.h"
#include "siis/trade/tradesignal.h"

using namespace siis;

StopLoss::StopLoss() :
    EntryExit()
{

}

void StopLoss::init(const Market *market, ContextConfig &conf)
{
    EntryExit::init(market, conf.stopLoss());
}

void StopLoss::updateSignal(TradeSignal &signal) const
{
    if (m_distanceType == DIST_PERCENTIL) {
        if (signal.direction() > 0) {
            if (signal.stopLoss() <= 0.0 || signal.estimateStopLossRate() > m_distance) {
                signal.setStopLossPrice(signal.price() * (1.0 - m_distance));
                printf("clamp to SL %g\n", m_distance*100);
            }
        } else if (signal.direction() < 0) {
            if (signal.stopLoss() <= 0.0 || signal.estimateStopLossRate() > m_distance) {
                signal.setStopLossPrice(signal.price() * (1.0 + m_distance));
                printf("clamp to SL %g\n", m_distance*100);
            }
        }
    } else if (m_distanceType == DIST_PRICE) {
        if (signal.direction() > 0) {
            if (signal.stopLoss() <= 0.0 || signal.absDistanceFromStop() > m_distance) {
                signal.setStopLossPrice(signal.price() - m_distance);
            }
        } else if (signal.direction() < 0) {
            if (signal.stopLoss() <= 0.0 || signal.absDistanceFromStop() > m_distance) {
                signal.setStopLossPrice(signal.price() + m_distance);
            }
        }
    }
}
