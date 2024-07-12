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
    EntryExit::init(market, conf.breakeven());
}

void Breakeven::updateΤrade(o3d::Double timestamp, o3d::Double lastTimestamp, Trade *trade) const
{
    if (!trade) {
        return;
    }

    if ((m_adjustPolicy == ADJ_PRICE) || (m_adjustPolicy == ADJ_CLOSE && m_consolidated)) {
        // price change at anytime using ADJ_PRICE policy or only on consolidation using ADJ_CLOSE policy
        if (m_distanceType == DIST_PERCENTIL) {
            if (trade->direction() > 0) {
                if (trade->estimateProfitLossRate() > m_distance && trade->stopLossPrice() < trade->entryPrice()) {
                    modifyTradeStopLossPriceWithOffset(trade);
                }
            } else if (trade->direction() < 0) {
                if (trade->estimateProfitLossRate() > m_distance && trade->stopLossPrice() > trade->entryPrice()) {
                    modifyTradeStopLossPriceWithOffset(trade);
                }
            }
        } else if (m_distanceType == DIST_PRICE) {
            if (trade->direction() > 0) {
                if (trade->deltaPrice() >= m_distance && trade->stopLossPrice() < trade->entryPrice()) {
                    modifyTradeStopLossPriceWithOffset(trade);
                }
            } else if (trade->direction() < 0) {
                if (trade->deltaPrice() <= -m_distance && trade->stopLossPrice() > trade->entryPrice()) {
                    modifyTradeStopLossPriceWithOffset(trade);
                }
            }
        }
    } else if (m_adjustPolicy == ADJ_CUSTOM) {
        // custom method from strategy, nothing to do here
    }
}

o3d::Double Breakeven::breakevenTiggerPrice(o3d::Double price, o3d::Int32 direction) const
{
    if (m_distanceType == DIST_PERCENTIL) {
        return price + (direction * price * m_distance);
    } else if (m_distanceType == DIST_PRICE) {
        return price + direction * m_distance;
    }

    return 0.0;
}

void Breakeven::modifyTradeStopLossPriceWithOffset(Trade *trade) const
{
    if (m_offset == 0.0) {
        // same price in both cases
        trade->modifyStopLoss(trade->entryPrice(), Trade::MOD_PREVIOUS);
    } else {
        if (m_offsetType == DIST_PERCENTIL) {
            if (trade->direction() > 0) {
                trade->modifyStopLoss(trade->entryPrice() * (1.0 + m_offset), Trade::MOD_PREVIOUS);
            } else if (trade->direction() < 0) {
                trade->modifyStopLoss(trade->entryPrice() * (1.0 - m_offset), Trade::MOD_PREVIOUS);
            }
        } else if (m_offsetType == DIST_PRICE) {
            if (trade->direction() > 0) {
                trade->modifyStopLoss(trade->entryPrice() + m_offset, Trade::MOD_PREVIOUS);
            } else if (trade->direction() < 0) {
                trade->modifyStopLoss(trade->entryPrice() - m_offset, Trade::MOD_PREVIOUS);
            }
        }
    }
}
