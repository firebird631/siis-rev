/**
 * @brief Strategy trade dynnamic stop-loss or in profit handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-04
 */

#include "siis/trade/dynamicstoploss.h"
#include "siis/config/strategyconfig.h"
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

void DynamicStopLoss::update(o3d::Double timestamp, o3d::Double lastTimestamp, Trade *trade)
{
    if (!trade) {
        return;
    }

//        if (m_stopLossPrice > 0.0 && estimateProfitLossRate() > 0.01) {
//            o3d::Double closeExecPrice = m_strategy->market()->closeExecPrice(m_direction);
//            m_stopLossPrice = m_entryPrice + m_direction * 0.01 * closeExecPrice;
//        }

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
