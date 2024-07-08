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

    // @todo implement offset

    if (m_adjustPolicy == ADJ_PRICE) {
        // price change at anytime
        if (m_distanceType == DIST_PERCENTIL) {
            if (trade->direction() > 0) {
                if (trade->estimateProfitLossRate() > m_distance && trade->stopLossPrice() < trade->entryPrice()) {
                    trade->modifyStopLoss(trade->entryPrice() + trade->strategy()->market()->spread(), Trade::MOD_PREVIOUS);
                }
            } else if (trade->direction() < 0) {
                if (trade->estimateProfitLossRate() > m_distance && trade->stopLossPrice() > trade->entryPrice()) {
                    trade->modifyStopLoss(trade->entryPrice() - trade->strategy()->market()->spread(), Trade::MOD_PREVIOUS);
                }
            }
        } else if (m_distanceType == DIST_PRICE) {
            if (trade->direction() > 0) {
                if (trade->deltaPrice() >= m_distance && trade->stopLossPrice() < trade->entryPrice()) {
                    trade->modifyStopLoss(trade->entryPrice() + trade->strategy()->market()->spread(), Trade::MOD_PREVIOUS);
                }
            } else if (trade->direction() < 0) {
                if (trade->deltaPrice() <= -m_distance && trade->stopLossPrice() > trade->entryPrice()) {
                    trade->modifyStopLoss(trade->entryPrice() - trade->strategy()->market()->spread(), Trade::MOD_PREVIOUS);
                }
            }
        }
    } else if (m_adjustPolicy == ADJ_CLOSE) {
        // price change at a close
        if (m_consolidated) {
            if (m_distanceType == DIST_PERCENTIL) {
                if (trade->direction() > 0) {
                    if (trade->estimateProfitLossRate() > m_distance && trade->stopLossPrice() < trade->entryPrice()) {
                        trade->modifyStopLoss(trade->entryPrice() + trade->strategy()->market()->spread(), Trade::MOD_PREVIOUS);
                    }
                } else if (trade->direction() < 0) {
                    if (trade->estimateProfitLossRate() > m_distance && trade->stopLossPrice() > trade->entryPrice()) {
                        trade->modifyStopLoss(trade->entryPrice() - trade->strategy()->market()->spread(), Trade::MOD_PREVIOUS);
                    }
                }
            } else if (m_distanceType == DIST_PRICE) {
                if (trade->direction() > 0) {
                    if (trade->deltaPrice() >= m_distance && trade->stopLossPrice() < trade->entryPrice()) {
                        trade->modifyStopLoss(trade->entryPrice() + trade->strategy()->market()->spread(), Trade::MOD_PREVIOUS);
                    }
                } else if (trade->direction() < 0) {
                    if (trade->deltaPrice() <= -m_distance && trade->stopLossPrice() > trade->entryPrice()) {
                        trade->modifyStopLoss(trade->entryPrice() - trade->strategy()->market()->spread(), Trade::MOD_PREVIOUS);
                    }
                }
            }
        }
    } else if (m_adjustPolicy == ADJ_CUSTOM) {
        // custom method from strategy
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
