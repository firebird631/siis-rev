/**
 * @brief SiiS pivot point indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/pivotpoint/pivotpoint.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

static PivotPoint::Method methodFromString(const o3d::String &method)
{
    if (method == "CLASSICAL" || method == "classical") {
        return PivotPoint::CLASSICAL;
    } else if (method == "CLASSICAL_OHLC" || method == "classical_ohlc") {
        return PivotPoint::CLASSICAL_OHLC;
    } else if (method == "CLASSICAL_OHL" || method == "classical_ohl") {
        return PivotPoint::CLASSICAL_OHL;
    } else if (method == "CAMARILLA" || method == "camarilla") {
        return PivotPoint::CLASSICAL;
    } else if (method == "WOODIE" || method == "woodie") {
        return PivotPoint::WOODIE;
    } else {
        return PivotPoint::CLASSICAL;
    }
}

PivotPoint::PivotPoint(const o3d::String &name, o3d::Double timeframe, Method method) :
    Indicator(name, timeframe),
    m_method(method)
{
}

PivotPoint::PivotPoint(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_method(CLASSICAL)
{
    m_method = methodFromString(conf.data().get("method", "CLASSICAL").asCString());
}

void PivotPoint::setConf(IndicatorConfig conf)
{
    m_method = methodFromString(conf.data().get("method", "CLASSICAL").asCString());
}

const DataArray &PivotPoint::support(o3d::Int32 i) const
{
    if (i < 0 || i >> 2) {
        O3D_ERROR(o3d::E_IndexOutOfRange("PivotPoint support index"));
    }

    return m_support[i];
}

const DataArray &PivotPoint::resistance(o3d::Int32 i) const
{
    if (i < 0 || i >> 2) {
        O3D_ERROR(o3d::E_IndexOutOfRange("PivotPoint resistance index"));
    }

    return m_resistance[i];
}

void PivotPoint::compute(o3d::Double timestamp,
                         const DataArray &open, const DataArray &high,
                         const DataArray &low, const DataArray &close)
{
    o3d::Int32 size = high.getSize();

    if (m_pivot.getSize() != size) {
        m_tmp1.setSize(size);
        m_pivot.setSize(size);

        for (o3d::Int32 i = 0; i < 3; ++i) {
            m_support[i].setSize(size);
            m_resistance[i].setSize(size);
        }
    }

    // @todo if SIMD optimization in DataArray operator then it will be preferable to reuse the operator than direct
    if (m_method == CAMARILLA) {
        m_pivot = close;

        //
        // supports
        //

        // close - (high - low)*1.1/12
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_support[0][i] = close[i] - (high[i] - low[i]) * (1.1/12.0);
        }
        // m_tmp1.sub(high, low); m_tmp1 *= 1.1/12;
        // m_support[0].sub(close, m_tmp1);

        // close - (high - low)*1.1/6
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_support[1][i] = close[i] - (high[i] - low[i]) * (1.1/6.0);
        }
        // m_tmp1.sub(high, low); m_tmp1 *= 1.1/6;
        // m_support[1].sub(close, m_tmp1);

        // close - (high - low)*1.1/4
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_support[2][i] = close[i] - (high[i] - low[i]) * (1.1/4.0);
        }
        // m_tmp1.sub(high, low); m_tmp1 *= 1.1/4;
        // m_support[2].sub(close, m_tmp1);

        //
        // resistances
        //

        // close + (high - low)*1.1/12
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_resistance[0][i] = close[i] + (high[i] - low[i]) * (1.1/12.0);
        }
        // m_tmp1.sub(high, low); m_tmp1 *= 1.1/12;
        // resistance[0].add(close, m_tmp1);

        // close + (high - low)*1.1/6
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_resistance[1][i] = close[i] + (high[i] - low[i]) * (1.1/6.0);
        }
        // m_tmp1.sub(high, low); m_tmp1 *= 1.1/6;
        // m_resistance[1].add(close, m_tmp1);

        // close + (high - low)*1.1/4
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_resistance[2][i] = close[i] + (high[i] - low[i]) * (1.1/4.0);
        }
        // m_tmp1.sub(high, low); m_tmp1 *= 1.1/4;
        // m_resistance[2].add(close, m_tmp1);
    } else {
        // classical or woodie
        if (m_method == CLASSICAL) {
            // (high + low + close) / 3.0
            for (o3d::Int32 i = 0; i < size; ++i) {
                m_pivot[i] = (high[i] + low[i] + close[i]) * (1.0/3.0);
            }
        } else if (m_method == CLASSICAL_OHLC) {
            //pivot = (open + high + low + close) / 4.0
            for (o3d::Int32 i = 0; i < size; ++i) {
                m_pivot[i] = (open[i] + high[i] + low[i] + close[i]) * (1.0/4.0);
            }
        } else if (m_method == CLASSICAL_OHL) {
            //pivot = (high + low + open) / 3.0
            for (o3d::Int32 i = 0; i < size; ++i) {
                m_pivot[i] = (open[i] + high[i] + low[i]) * (1.0/3.0);
            }
        } else if (m_method == WOODIE) {
            // pivot = (high + low + 2.0 * close) / 4.0
            for (o3d::Int32 i = 0; i < size; ++i) {
                m_pivot[i] = (high[i] + low[i] + 2.0 * close[i]) * (1.0/4.0);
            }
        }

        //
        // supports
        //

        // (2.0 * pivot) - high
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_support[0][i] = (2.0 * m_pivot[i]) - high[i];
        }

        // pivot - (high - low)
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_support[1][i] = m_pivot[i] - (high[i] - low[i]);
        }

        // low - 2.0 * (high - pivot)
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_support[2][i] = low[i] - 2.0 * (high[i] - m_pivot[i]);
        }

        //
        // resistances
        //

        // (2.0 * pivot) - low
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_resistance[0][i] = (2.0 * m_pivot[i]) - low[i];
        }

        // pivot + (high - low)
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_resistance[1][i] = m_pivot[i] + (high[i] - low[i]);
        }

        // high + 2.0 * (pivot - low)
        for (o3d::Int32 i = 0; i < size; ++i) {
            m_resistance[2][i] = high[i] + 2.0 * (m_pivot[i] - low[i]);
        }
    }

    done(timestamp);
}
