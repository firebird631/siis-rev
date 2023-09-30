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
    EntryExit(),
    m_maxSpread(0.0)
{

}

void Entry::init(const Market *market, ContextConfig &conf)
{
    EntryConfig entryConfig = conf.entry();

    if (entryConfig.data().isMember("type")) {
        o3d::String type = entryConfig.data().get("type", Json::Value()).asCString();
        if (type == "fixed") {
            m_priceType = PRICE_FIXED;
        } else if (type == "fixed-pct") {
            m_priceType = PRICE_FIXED;
        } else if (type == "fixed-dist") {
            m_priceType = PRICE_FIXED;
        } else if (type == "custom") {
            m_priceType = PRICE_CUSTOM;
        } else if (type == "last") {
            m_priceType = PRICE_LAST;
        } else if (type == "best+1") {
            m_priceType = PRICE_BEST1;
        } else if (type == "best+2") {
            m_priceType = PRICE_BEST2;
        }
    }

    if (entryConfig.data().isMember("timeframe")) {
        o3d::String timeframe = entryConfig.data().get("timeframe", Json::Value()).asCString();
        m_timeframe = timeframeFromStr(timeframe);
    }

    if (entryConfig.data().isMember("distance")) {
        o3d::String distance = entryConfig.data().get("distance", Json::Value()).asCString();
        if (distance.endsWith("%")) {
            distance.trimRight('%');
            m_distance = distance.toDouble() * 0.01;
            m_distanceType = DIST_PERCENTIL;
        } else if (distance.endsWith("pip")) {
            distance.trimRight("pip");
            m_distance = distance.toDouble() * market->onePipMean();
            m_distanceType = DIST_PRICE;
        } else if (distance.endsWith("pips")) {
            distance.trimRight("pips");
            m_distance = distance.toDouble() * market->onePipMean();
            m_distanceType = DIST_PRICE;
        } else {
            m_distance = distance.toDouble();
            m_distanceType = DIST_PRICE;
        }
    }

    if (m_priceType == PRICE_CUSTOM) {
        m_adjustPolicy = ADJ_CUSTOM;
    } else if (m_timeframe > 0.0 && m_distance > 0.0) {
        m_adjustPolicy = ADJ_CLOSE;
    } else if (m_timeframe == 0.0 && m_distance > 0.0) {
        m_adjustPolicy = ADJ_PRICE;
    } else {
        m_adjustPolicy = ADJ_NONE;
    }

    if (entryConfig.data().isMember("max-spread")) {
        Json::Value maxSpread = entryConfig.data().get("max-spread", Json::Value());
        if (maxSpread.isNumeric()) {
            m_maxSpread = maxSpread.asDouble();
        } else if (maxSpread.isString()) {
            o3d::String value = maxSpread.asCString();
            if (value.endsWith("pip")) {
                value.trimRight("pip");
                m_maxSpread = value.toDouble() * market->onePipMean();
            } else if (value.endsWith("pips")) {
                value.trimRight("pips");
                value = value.toDouble() * market->onePipMean();
            } else {
                m_maxSpread = value.toDouble();
            }
        }
    }

    if (entryConfig.data().isMember("timeout")) {
        o3d::String timeout = entryConfig.data().get("timeout", Json::Value()).asCString();
        m_timeout = timeframeFromStr(timeout);
    }
}

void Entry::updateSignal(TradeSignal &signal, const Market *market) const
{
    if (m_priceType == PRICE_LAST) {
        // enter at market
        signal.setOrderType(Order::ORDER_MARKET);
        signal.setEntryPrice(market->openExecPrice(signal.direction()));
    } else if (m_priceType == PRICE_BEST1) {
        // best immediate limit price
        if (signal.direction() > 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            signal.setEntryPrice(market->bid());
        } else if (signal.direction() < 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            signal.setEntryPrice(market->ask());
        }
    } else if (m_priceType == PRICE_BEST2) {
        // second best limit price (widen by spread)
        if (signal.direction() > 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            signal.setEntryPrice(market->bid() - market->spread());
        } else if (signal.direction() < 0) {
            signal.setOrderType(Order::ORDER_LIMIT);
            signal.setEntryPrice(market->ask() + market->spread());
        }
    } else if (m_priceType == PRICE_FIXED) {
        if (m_distanceType == DIST_PERCENTIL) {
            // sub a price percent
            if (signal.direction() > 0) {
                signal.setOrderType(Order::ORDER_LIMIT);
                signal.setEntryPrice(market->bid() * (1.0 - m_distance));
            } else if (signal.direction() < 0) {
                signal.setOrderType(Order::ORDER_LIMIT);
                signal.setEntryPrice(market->ask() * (1.0 + m_distance));
            }
        } else if (m_distanceType == DIST_PRICE) {
            // sub a price delta price
            if (signal.direction() > 0) {
                signal.setOrderType(Order::ORDER_LIMIT);
                signal.setEntryPrice(market->bid() - m_distance);
            } else if (signal.direction() < 0) {
                signal.setOrderType(Order::ORDER_LIMIT);
                signal.setEntryPrice(market->ask() + m_distance);
            }
        }
    }
}

o3d::Bool Entry::checkMaxSpread(const Market *market) const
{
    if (m_maxSpread > 0.0) {
        return market->spread() <= m_maxSpread;
    }

    return true;
}
