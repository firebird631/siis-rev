/**
 * @brief SiiS tick Cumulative Volume Delta indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-18
 */

#include "siis/indicators/cumulativevolumedelta/cvd.h"
#include "siis/utils/common.h"
#include "siis/utils/math.h"

using namespace siis;
using o3d::Logger;
using o3d::Debug;

CumulativeVolumeDelta::CumulativeVolumeDelta(const o3d::String &name,
           o3d::Double timeframe,
           o3d::Int32 depth,
           const o3d::CString &cvdTimeframe,
           o3d::Bool sessionFilter) :
    Indicator(name, timeframe),
    m_cvdTimeframe(0.0),
    m_sessionOffset(0.0),
    m_sessionDuration(0.0),
    m_depth(depth),
    m_sessionFilter(sessionFilter),
    m_openTimestamp(0.0),
    m_prevTickPrice(0.0),
    m_cvd(depth),
    m_prev(0.0),
    m_last(0.0)
{
    O3D_ASSERT(depth > 0);

    if (cvdTimeframe.isValid()) {
        m_cvdTimeframe = timeframeFromStr(cvdTimeframe);
    }

    // begin the first value
    m_cvd.append(0.0);
}

CumulativeVolumeDelta::CumulativeVolumeDelta(const o3d::String &name, o3d::Double timeframe,
                                             o3d::Int32 depth, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_cvdTimeframe(0.0),
    m_sessionOffset(0.0),
    m_sessionDuration(0.0),
    m_depth(depth),
    m_sessionFilter(false),
    m_openTimestamp(0.0),
    m_prevTickPrice(0.0),
    m_cvd(depth),
    m_prev(0.0),
    m_last(0.0)
{
    O3D_ASSERT(depth > 0);

    if (conf.data().isObject()) {
        m_cvdTimeframe = timeframeFromStr(conf.data().get("cvd-timeframe", "1d").asString().c_str());
        m_sessionFilter = conf.data().get("session-filter", false).asBool();
    } else if (conf.data().isArray()) {
        m_cvdTimeframe = timeframeFromStr(conf.data().get((Json::ArrayIndex)1, "1d").asString().c_str());
        m_sessionFilter = conf.data().get((Json::ArrayIndex)3, false).asBool();
    }

    // begin the first value
    m_cvd.append(0.0);
}

CumulativeVolumeDelta::~CumulativeVolumeDelta()
{

}

void CumulativeVolumeDelta::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_cvdTimeframe = timeframeFromStr(conf.data().get("cvd-timeframe", "1d").asString().c_str());
         m_sessionFilter = conf.data().get("session-filter", false).asBool();
    } else if (conf.data().isArray()) {
        m_cvdTimeframe = timeframeFromStr(conf.data().get((Json::ArrayIndex)1, "1d").asString().c_str());
        m_sessionFilter = conf.data().get((Json::ArrayIndex)3, false).asBool();
    }
}

void CumulativeVolumeDelta::setSession(o3d::Double sessionOffset, o3d::Double sessionDuration)
{
    m_sessionOffset = sessionOffset;

    if (sessionDuration > 0.0) {
        m_sessionDuration = sessionDuration;
    } else {
        m_sessionDuration = m_cvdTimeframe;
    }
}

void CumulativeVolumeDelta::update(const Tick &tick, o3d::Bool finalize)
{
    m_prev = m_last;

    if (finalize) {
        this->finalize();
    }

    // reset accumulator at each new session and for the initial state
    if (tick.timestamp() > m_openTimestamp + m_cvdTimeframe) {
        // printf("%s %f\n", timestampToStr(tick.timestamp()).toAscii().getData(), m_cvd.back());
        m_cvd.back() = 0.0;

        // new session beginning timestamp
        m_openTimestamp = baseTime(tick.timestamp(), m_cvdTimeframe);

        // session offset and duration only apply to a daily CVD
        if (m_cvdTimeframe == TF_DAY && m_sessionFilter) {
            m_openTimestamp += m_sessionOffset;
        }
    }

    if (m_sessionFilter && m_cvdTimeframe == TF_DAY && (m_sessionOffset > 0 || m_sessionDuration > 0)) {
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

    o3d::Double deltaVolume = 0;

    // -1  for bid, 1 for ask, or 0 if no info
    if (tick.buyOrSell() < 0) {
        deltaVolume = -tick.volume();
    } else if (tick.buyOrSell() > 0) {
        deltaVolume = tick.volume();
    } else if (m_prevTickPrice != 0.0) {
        // detect direction from last tick price, but need at least a previous sample
        if (tick.last() > m_prevTickPrice) {
            deltaVolume = tick.volume();
        } else if (tick.last() < m_prevTickPrice) {
            deltaVolume = -tick.volume();
        }
    }

    m_prevTickPrice = tick.last();

    m_last = m_cvd.back() += deltaVolume;

    // retain the last tick timestamp
    done(tick.timestamp());
}

void CumulativeVolumeDelta::finalize()
{
    m_cvd.append(m_cvd.back());
}
