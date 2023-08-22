/**
 * @brief SiiS strategy trade initial entry handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-22
 */

#include "siis/trade/entry.h"
#include "siis/config/strategyconfig.h"
#include "siis/market.h"
#include "siis/strategy.h"
#include "siis/handler.h"
#include "siis/trade/tradesignal.h"

using namespace siis;

Entry::Entry() :
    EntryExit()
{

}

void Entry::init(const Market *market, ContextConfig &conf)
{
    if (conf.data().isMember("type")) {
        o3d::String type = conf.data().get("type", Json::Value()).asCString();
        if (type == "fixed-pct") {
            m_distanceType = DIST_PERCENTIL;
        } else if (type == "fixed-dist") {
            m_distanceType = DIST_PRICE;
        } else if (type == "custom") {
            m_distanceType = DIST_CUSTOM;
        } else if (type == "last") {
            m_distanceType = DIST_LAST;
        } else if (type == "best+1") {
            m_distanceType = DIST_BEST1;
        } else if (type == "best+2") {
            m_distanceType = DIST_BEST2;
        }
    }

    if (conf.data().isMember("timeframe")) {
        o3d::String timeframe = conf.data().get("timeframe", Json::Value()).asCString();
        m_timeframe = timeframeFromStr(timeframe);
    }

    if (conf.data().isMember("distance")) {
        o3d::String distance = conf.data().get("distance", Json::Value()).asCString();
        if (distance.endsWith("%")) {
            distance.trimRight('%');
            m_distance = distance.toDouble() * 0.01;
        } else if (distance.endsWith("pip")) {
            distance.trimRight("pip");
            m_distance = distance.toDouble() * market->onePipMean();
        } else if (distance.endsWith("pips")) {
            distance.trimRight("pips");
            m_distance = distance.toDouble() * market->onePipMean();
        } else {
            m_distance = distance.toDouble();
        }
    }

    if (m_distanceType == DIST_CUSTOM) {
        m_adjustPolicy = ADJ_CUSTOM;
    } else if (m_timeframe > 0.0 && m_distance > 0.0) {
        m_adjustPolicy = ADJ_CLOSE;
    } else if (m_timeframe == 0.0 && m_distance > 0.0) {
        m_adjustPolicy = ADJ_PRICE;
    } else {
        m_adjustPolicy = ADJ_NONE;
    }
}

void Entry::updateSignal(TradeSignal &signal, const Market *market) const
{
    if (m_distanceType == DIST_LAST) {
        // enter at market
        signal.setOrderType(Order::ORDER_MARKET);
        signal.setEntryPrice(market->openExecPrice(signal.direction()));
    } else if (m_distanceType == DIST_BEST1) {
        // best immediate limit price
        if (signal.direction() > 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            signal.setEntryPrice(market->bid());
        } else if (signal.direction() < 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            signal.setEntryPrice(market->ask());
        }
    } else if (m_distanceType == DIST_BEST2) {
        // second best limit price (widen by spread)
        if (signal.direction() > 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            signal.setEntryPrice(market->bid() - market->spread());
        } else if (signal.direction() < 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            signal.setEntryPrice(market->ask() + market->spread());
        }
    } else if (m_distanceType == DIST_PERCENTIL) {
        // @todo price percent delta price
        if (signal.direction() > 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            // signal.setEntryPrice(price);
        } else if (signal.direction() < 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            // signal.setEntryPrice(price);
        }
    } else if (m_distanceType == DIST_PRICE) {
        // @todo price delta price
        if (signal.direction() > 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            // signal.setEntryPrice(price);
        } else if (signal.direction() < 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            // signal.setEntryPrice(price);
        }
    }
}
