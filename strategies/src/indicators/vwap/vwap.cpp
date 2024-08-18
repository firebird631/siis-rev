/**
 * @brief SiiS tick VWAP indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-18
 */

#include "siis/indicators/vwap/vwap.h"
#include "siis/utils/common.h"
#include "siis/utils/math.h"

#include <o3d/core/math.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

VWapData::VWapData(o3d::Double vwapTimeframe, o3d::Double timestamp, o3d::Int32 numStdDev)
{
    vwap.zero(1);

    plusStdDev.reserve(3);
    minusStdDev.reserve(3);

    for (o3d::Int32 i = 0; i < numStdDev; ++i) {
        plusStdDev[i].zero(1);
        minusStdDev[i].zero(1);
    }
}

const DataArray& VWapData::stdDevAt(o3d::Int32 stdDev) const
{
    if (stdDev == 0) throw o3d::E_IndexOutOfRange("");

    if (stdDev > 0) {
        o3d::Int32 size = static_cast<o3d::Int32>(minusStdDev.size());

        if (stdDev >= size) throw o3d::E_IndexOutOfRange("");

        return plusStdDev[stdDev];
    }

    o3d::Int32 size = static_cast<o3d::Int32>(minusStdDev.size());

    if (-stdDev >= size) throw o3d::E_IndexOutOfRange("");

    return minusStdDev[-stdDev];
}

VWap::VWap(const o3d::String &name,
           o3d::Double timeframe,
           const o3d::CString &vwapTimeframe,
           o3d::Int32 historySize,
           o3d::Int32 numStdDev,
           o3d::Bool sessionFilter) :
    Indicator(name, timeframe),
    m_vwapTimeframe(0.0),
    m_numStdDev(numStdDev),
    m_sessionOffset(0.0),
    m_sessionDuration(0.0),
    m_historySize(historySize),
    m_sessionFilter(sessionFilter),
    m_pCurrent(nullptr),
    m_openTimestamp(0.0),
    m_pvs(0.0),
    m_volumes(0.0),
    m_volume_dev(0.0),
    m_dev2(0.0),
    m_prev(0.0),
    m_last(0.0)
{
    if (vwapTimeframe.isValid()) {
        m_vwapTimeframe = timeframeFromStr(vwapTimeframe);
    }
}

VWap::VWap(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_vwapTimeframe(0.0),
    m_numStdDev(3),
    m_sessionOffset(0.0),
    m_sessionDuration(0.0),
    m_historySize(0),
    m_sessionFilter(false),
    m_pCurrent(nullptr),
    m_openTimestamp(0.0),
    m_pvs(0.0),
    m_volumes(0.0),
    m_volume_dev(0.0),
    m_dev2(0.0),
    m_prev(0.0),
    m_last(0.0)
{
    if (conf.data().isObject()) {
        m_vwapTimeframe = timeframeFromStr(conf.data().get("vwap-timeframe", "1d").asString().c_str());
        m_historySize = conf.data().get("history", 7).asInt();
        m_numStdDev = conf.data().get("num-std-dev", 3).asInt();
        m_sessionFilter = conf.data().get("session-filter", false).asBool();
    } else if (conf.data().isArray()) {
        m_vwapTimeframe = timeframeFromStr(conf.data().get((Json::ArrayIndex)1, "1d").asString().c_str());
        m_historySize = conf.data().get((Json::ArrayIndex)2, 7).asInt();
        m_numStdDev = conf.data().get((Json::ArrayIndex)3, 3).asInt();
        m_sessionFilter = conf.data().get((Json::ArrayIndex)4, false).asBool();
    }
}

VWap::~VWap()
{
    if (m_pCurrent) {
        o3d::deletePtr(m_pCurrent);
    }

    for (auto vwap : m_vwap) {
        o3d::deletePtr(vwap);
    }
}

void VWap::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_vwapTimeframe = timeframeFromStr(conf.data().get("vwap-timeframe", "1d").asString().c_str());
        m_historySize = conf.data().get("history", 7).asInt();
        m_numStdDev = conf.data().get("num-std-dev", 3).asInt();
        m_sessionFilter = conf.data().get("session-filter", false).asBool();
    } else if (conf.data().isArray()) {
        m_vwapTimeframe = timeframeFromStr(conf.data().get((Json::ArrayIndex)1, "1d").asString().c_str());
        m_historySize = conf.data().get((Json::ArrayIndex)2, 7).asInt();
        m_numStdDev = conf.data().get((Json::ArrayIndex)3, 3).asInt();
        m_sessionFilter = conf.data().get((Json::ArrayIndex)4, false).asBool();
    }
}

void VWap::setSession(o3d::Double sessionOffset, o3d::Double sessionDuration)
{
    m_sessionOffset = sessionOffset;

    if (sessionDuration > 0.0) {
        m_sessionDuration = sessionDuration;
    } else {
        m_sessionDuration = m_vwapTimeframe;
    }
}

const VWapData* VWap::previous(o3d::Int32 n) const
{
    o3d::Int32 size = static_cast<o3d::Int32>(m_vwap.size());

    if (n >= size) throw o3d::E_IndexOutOfRange("");

    if (n < 0) {
        n = size + n;
        if (n < 0) throw o3d::E_IndexOutOfRange("");
    }

    return m_vwap[n];
}

o3d::Double VWap::vwapAt(o3d::Int32 n) const
{
    if (n > -1) {
        return m_pCurrent != nullptr ? m_pCurrent->vwap.last() : 0.0;
    } else {
        return previous(n)->vwap.last();
    }
}

o3d::Double VWap::stdDevAt(o3d::Int32 n, o3d::Int32 stdDevNum) const
{
    if (n > -1) {
        return m_pCurrent != nullptr ? m_pCurrent->stdDevAt(stdDevNum).last() : 0.0;
    } else {
        return previous(n)->stdDevAt(stdDevNum).last();
    }
}

void VWap::update(const Tick &tick, o3d::Bool finalize)
{
    m_prev = m_last;

    if (tick.timestamp() >= m_openTimestamp + m_vwapTimeframe) {
        // new VWAP
        this->finalize();
    } else if (finalize) {
        // or finalize the current (intra) bar (not the current VWAP timeframe)
        m_pCurrent->vwap.push(m_pCurrent->vwap.last());

        for (o3d::Int32 i = 0; i < m_numStdDev; ++i) {
            m_pCurrent->minusStdDev[i].push(m_pCurrent->minusStdDev[i].last());
            m_pCurrent->plusStdDev[i].push(m_pCurrent->plusStdDev[i].last());
        }
    }

    if (m_sessionFilter && m_vwapTimeframe == TF_DAY && (m_sessionOffset > 0 || m_sessionDuration > 0)) {
        o3d::Double basetime = baseTime(TF_DAY, tick.timestamp());

        if (tick.timestamp() < basetime + m_sessionOffset) {
            // ignored, out of session
            return;
        }

        if (tick.timestamp() >= basetime + m_sessionOffset + m_sessionDuration) {
            // ignored, out of session
            return;
        }
    }

    if (m_pCurrent == nullptr) {
        // new session beginning timestamp
        m_openTimestamp = baseTime(tick.timestamp(), m_vwapTimeframe);

        o3d::Double vwapTimeframe = 0.0;

        // session offset and duration only apply to a daily VWAP
        if (m_vwapTimeframe == TF_DAY && m_sessionFilter) {
            m_openTimestamp += m_sessionOffset;

            // timeframe depends on the session duration
            vwapTimeframe = m_sessionDuration > 0.0 ? m_sessionDuration : m_vwapTimeframe;
        } else {
            // or is initially defined
            vwapTimeframe = m_vwapTimeframe;
        }

        m_pCurrent = new VWapData(vwapTimeframe, m_openTimestamp, m_numStdDev);
    }

    // cumulative
    o3d::Double vwap = 0.0;

    m_pvs += tick.last() * tick.volume();  // price * volume
    m_volumes += tick.volume();

    m_pCurrent->vwap[m_pCurrent->vwap.getSize()-1] = vwap = m_pvs / m_volumes;

    m_last = m_pCurrent->vwap.last();

    // std dev
    m_volume_dev += (tick.last() * tick.last()) * tick.volume();  // price^2 * volume
    m_dev2 = o3d::max(m_volume_dev / m_volumes - vwap * vwap, 0.0);

    o3d::Double stdDev = o3d::Math::sqrt(m_dev2);

    for (o3d::Int32 i = 0; i < m_numStdDev; ++i) {
        m_pCurrent->minusStdDev[i].set(-1, vwap - (i+1) * stdDev);
        m_pCurrent->plusStdDev[i].set(-1, vwap + (i+1) * stdDev);
    }

    // retain the last tick timestamp
    done(tick.timestamp());
}

void VWap::finalize()
{
    if (m_pCurrent == nullptr) {
        return;
    }

    if (m_historySize > 0) {
        m_vwap.push_back(m_pCurrent);

        if (static_cast<o3d::Int32>(m_vwap.size()) > m_historySize) {
            o3d::deletePtr(m_vwap.front());
            m_vwap.pop_front();
        }
    }

    // reset accumulators
    m_pvs = 0.0;
    m_volumes = 0.0;
    m_volume_dev = 0.0;
    m_dev2 = 0.0;

    // force to create a new one
    m_pCurrent = nullptr;
}
