/**
 * @brief SiiS strategy trade initial take-profit handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#include "siis/trade/takeprofit.h"
#include "siis/config/strategyconfig.h"
#include "siis/market.h"
#include "siis/strategy.h"
#include "siis/handler.h"
#include "siis/trade/tradesignal.h"

using namespace siis;

TakeProfit::TakeProfit() :
    EntryExit()
{

}

void TakeProfit::init(const Market *market, ContextConfig &conf)
{
    EntryExit::init(market, conf.takeProfit());
}

void TakeProfit::updateSignal(TradeSignal &signal) const
{
    if (m_distanceType == DIST_PERCENTIL) {
        if (signal.direction() > 0) {
            if (signal.takeProfit() <= 0.0 || signal.estimateTakeProfitRate() > m_distance) {
                signal.setTakeProfitPrice(signal.price() * (1.0 + m_distance));
            }
        } else if (signal.direction() < 0) {
            if (signal.takeProfit() <= 0.0 || signal.estimateTakeProfitRate() > m_distance) {
                signal.setTakeProfitPrice(signal.price() * (1.0 - m_distance));
            }
        }
    } else if (m_distanceType == DIST_PRICE) {
        if (signal.direction() > 0) {
            if (signal.takeProfit() <= 0.0 || signal.absDistanceFromTakeProfit() > m_distance) {
                signal.setTakeProfitPrice(signal.price() + m_distance);
            }
        } else if (signal.direction() < 0) {
            if (signal.takeProfit() <= 0.0 || signal.absDistanceFromTakeProfit() > m_distance) {
                signal.setTakeProfitPrice(signal.price() - m_distance);
            }
        }
    }
}
