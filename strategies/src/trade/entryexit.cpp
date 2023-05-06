/**
 * @brief SiiS strategy trade entry/exit handler base model.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-06
 */

#include "siis/trade/entryexit.h"
#include "siis/config/strategyconfig.h"
#include "siis/market.h"
#include "siis/strategy.h"
#include "siis/handler.h"

using namespace siis;

EntryExit::EntryExit() :
    m_timeframe(0.0),
    m_distance(0.0),
    m_distanceType(DIST_NONE),
    m_adjustPolicy(ADJ_NONE),
    m_consolidated(false),
    m_lastClosedTimestamp(0.0)
{

}

void EntryExit::init(const Market *market, ContextConfig &conf)
{
    if (conf.data().isMember("type")) {
        o3d::String type = conf.data().get("type", Json::Value()).asCString();
        if (type == "fixed-pct") {
            m_distanceType = DIST_PERCENTIL;
        } else if (type == "custom") {
            m_distanceType = DIST_CUSTOM;
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

o3d::Bool EntryExit::consolidated(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    if (m_timeframe <= 0.0) {
        return false;
    }

    m_consolidated = false;

    if (baseTime(timestamp, m_timeframe) > m_lastClosedTimestamp) {
        m_consolidated = true;
        m_lastClosedTimestamp = timestamp;
    }

    return m_consolidated;
}