/**
 * @brief SiiS strategy trade breakeven handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-04
 */

#include "siis/trade/breakeven.h"
#include "siis/config/strategyconfig.h"
#include "siis/market.h"
#include "siis/strategy.h"
#include "siis/handler.h"

using namespace siis;

Breakeven::Breakeven() :
    EntryExit()
{

}

void Breakeven::init(const Market *market, ContextConfig &conf)
{
    EntryExit::init(market, conf);
}

void Breakeven::update(o3d::Double timestamp, o3d::Double lastTimestamp, Trade *trade)
{
    if (!trade) {
        return;
    }

    if (m_adjustPolicy == ADJ_PRICE) {
        // price change at anytime
        if (m_distanceType == DIST_PERCENTIL) {
            if (trade->estimateProfitLossRate() > m_distance && trade->stopLossPrice() < trade->entryPrice()) {
                trade->modifyStopLoss(trade->entryPrice(), Trade::MOD_PREVIOUS);
            }
        } else if (m_distanceType == DIST_PRICE && trade->stopLossPrice() < trade->entryPrice()) {
            if (trade->deltaPrice() >= m_distance) {
                trade->modifyStopLoss(trade->entryPrice(), Trade::MOD_PREVIOUS);
            }
        }
    } else if (m_adjustPolicy == ADJ_CLOSE) {
        // price change at a close
        if (consolidated(timestamp, lastTimestamp)) {
            if (m_distanceType == DIST_PERCENTIL) {
                if (trade->estimateProfitLossRate() > m_distance && trade->stopLossPrice() < trade->entryPrice()) {
                    trade->modifyStopLoss(trade->entryPrice(), Trade::MOD_PREVIOUS);
                }
            } else if (m_distanceType == DIST_PRICE && trade->stopLossPrice() < trade->entryPrice()) {
                if (trade->deltaPrice() >= m_distance) {
                    trade->modifyStopLoss(trade->entryPrice(), Trade::MOD_PREVIOUS);
                }
            }
        }
    } else if (m_adjustPolicy == ADJ_CUSTOM) {
        // custom method from strategy
    }
}
