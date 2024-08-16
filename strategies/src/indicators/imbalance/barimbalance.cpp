/**
 * @brief SiiS bar imbalance detector indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-02
 */

#include "siis/indicators/imbalance/barimbalance.h"
#include "siis/utils/common.h"
#include "siis/utils/math.h"

using namespace siis;
using o3d::Logger;
using o3d::Debug;


BarImbalance::BarImbalance(const o3d::String &name,
                             o3d::Double timeframe,
                             o3d::Int32 depth) :
    Indicator(name, timeframe),
    m_sessionOffset(0.0),
    m_sessionDuration(0.0),
    m_depth(depth)
{

}

BarImbalance::BarImbalance(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_sessionOffset(0.0),
    m_sessionDuration(0.0),
    m_depth(10)
{
    if (conf.data().isObject()) {
        m_depth = conf.data().get("depth", 10).asInt();
    } else if (conf.data().isArray()) {
        m_depth = conf.data().get((Json::ArrayIndex)1, 10).asInt();
    }
}

BarImbalance::~BarImbalance()
{
}

void BarImbalance::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_depth = conf.data().get("depth()", 10).asInt();
    } else if (conf.data().isArray()) {
        m_depth = conf.data().get((Json::ArrayIndex)1, 10).asInt();
    }
}

void BarImbalance::compute(o3d::Double timestamp, const DataArray &timestamps,
                           const DataArray &open, const DataArray &high, const DataArray &low, const DataArray &close,
                           o3d::Int32 numLastBars)
{
    if (numLastBars <= 0) {
        // need number of update and generated bar
        return;
    }

    if (timestamps.getSize() < 4) {
        // need at least 4 samples (3 closed + current)
        return;
    }

    T_Imbalance foundImbalances;

    if (numLastBars > 1) {
        o3d::Int32 baseIdx = timestamps.getSize() - numLastBars - 1;
        o3d::Int32 endIdx = baseIdx + numLastBars - 1;

        if (timestamps.getSize() > m_depth) {
            // limit input size
            baseIdx += timestamps.getSize() - m_depth;
        }

        for (o3d::Int32 i = baseIdx; i < endIdx; ++i) {
            o3d::Double curOpen = open[baseIdx];
            o3d::Double curClose = close[baseIdx];

            // lookup for upper imbalance
            o3d::Double prevHigh = high[baseIdx-1];
            o3d::Double nextLow  = low[baseIdx+1];

            if (nextLow > prevHigh) {
                if (curOpen <= prevHigh && curClose >= nextLow) {
                    // found at timestamp
                    foundImbalances.push_back(Imbalance());

                    Imbalance &imbalance = foundImbalances.back();

                    imbalance.direction = 1;
                    imbalance.lowPrice = prevHigh;
                    imbalance.highPrice = nextLow;
                    imbalance.barTimestamp = timestamps[baseIdx];
                }
            }

            // lookup for downer imbalance
            o3d::Double prevLow= low[baseIdx-1];
            o3d::Double nextHigh= high[baseIdx+1];

            if (nextHigh < prevLow) {
                if (curOpen >= prevLow && curClose <= nextHigh) {
                    // found at timestamp
                    foundImbalances.push_back(Imbalance());

                    Imbalance &imbalance = foundImbalances.back();

                    imbalance.direction = -1;
                    imbalance.lowPrice = prevLow;
                    imbalance.highPrice = nextHigh;
                    imbalance.barTimestamp = timestamps[baseIdx];
                }
            }
        }
    }

    // update previous
    T_Imbalance previousImbalances = updateImbalances(m_imbalances, high, low, numLastBars);

    // keep filtered previous and newly found
    m_imbalances = previousImbalances;
    m_imbalances.insert(m_imbalances.end(), foundImbalances.begin(), foundImbalances.end());

    done(timestamp);
}

o3d::Int32 BarImbalance::lookback() const
{
    return 4;
}

BarImbalance::T_Imbalance BarImbalance::updateImbalances(T_Imbalance &imbalances,
                                    const DataArray &high, const DataArray &low,
                                    o3d::Int32 numLastBars)
{
    T_Imbalance newList;
    o3d::Bool keep;

    for (o3d::Int32 i = high.getSize() - numLastBars; i < high.getSize(); ++i) {
        o3d::Double highPrice = high[i];
        o3d::Double lowPrice = low[i];

        for (Imbalance imbalance : imbalances) {
            keep = true;

            if (highPrice <= imbalance.highPrice && lowPrice >= imbalance.lowPrice) {
                // the bar is inside, it cancels and potentially could create 2 news one
                keep = false;
            } else if (highPrice > imbalance.highPrice && lowPrice < imbalance.lowPrice) {
                // the bar eat the imbalance, it cancels the imbalance
                keep = false;
            } else if (highPrice < imbalance.highPrice && lowPrice < imbalance.lowPrice) {
                // partially inside (low part), reduce the lower part
                imbalance.lowPrice = highPrice;
            } else if (highPrice > imbalance.highPrice && lowPrice > imbalance.lowPrice) {
                // partially inside (high part), reduce the higher part
                imbalance.highPrice = lowPrice;
            }

            if (keep) {
                newList.push_back(imbalance);
            }
        }
    }

    return newList;
}
