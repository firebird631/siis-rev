/**
 * @brief SiiS MAADX range-bar strategy trend analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadxrbtrendanalyser.h"

#include "siis/strategy.h"
#include "siis/config/strategyconfig.h"

using namespace siis;

MaAdxRbTrendAnalyser::MaAdxRbTrendAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, rangeSize, depth, history, priceMethod),
    m_slow_h_ma("slow_h_ma", rangeSize),
    m_slow_m_ma("slow_m_ma", rangeSize),
    m_slow_l_ma("slow_l_ma", rangeSize),
    m_vwap("vwap", rangeSize, depth),
    m_trend(0),
    m_cross(0),
    m_sig(0),
    m_vwapTrend(0)
{

}

MaAdxRbTrendAnalyser::~MaAdxRbTrendAnalyser()
{

}

o3d::String MaAdxRbTrendAnalyser::typeName() const
{
    return "trend";
}

void MaAdxRbTrendAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "slow_h_ma", m_slow_h_ma);
    configureIndicator(conf, "slow_m_ma", m_slow_m_ma);
    configureIndicator(conf, "slow_l_ma", m_slow_l_ma);

    configureIndicator(conf, "vwap", m_vwap);

    m_vwap.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    m_trend = 0;
    m_cross = 0;
    m_sig = 0;
    m_vwapTrend = 0;

    RangeBarAnalyser::init(conf);
}

void MaAdxRbTrendAnalyser::terminate()
{

}

void MaAdxRbTrendAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    o3d::Bool compute = true;

    if (isUpdateAtclose()) {
        compute = price().consolidated();
    }

    o3d::Int32 hc = 0;
    o3d::Int32 lc = 0;

    // reset sig
    m_sig = 0;

    if (compute) {
        m_slow_h_ma.compute(timestamp, price().high());
        // m_slow_m_ma.compute(timestamp, price().price());
        m_slow_l_ma.compute(timestamp, price().low());

        hc = DataArray::cross(price().close(), m_slow_h_ma.hma());
        lc = DataArray::cross(price().close(), m_slow_l_ma.hma());

        if (m_vwap.active()) {
            if (price().close().last() > m_vwap.last()) {
                m_vwapTrend = 1;
            } else if (price().close().last() < m_vwap.last()) {
                m_vwapTrend = -1;
            } else {
                m_vwapTrend = 0;
            }
        }

        // use the previous cross value (previous close)
        if (price().open().last() > m_slow_h_ma.last() && m_cross > 0) {
            m_sig = 1;
        } else if (price().open().last() < m_slow_l_ma.last() && m_cross < 0) {
            m_sig = -1;
        }

        // reset cross
        m_cross = 0;

        // compute here, take care, cross is not reset until next close
        // but sig will only trigger once as wish
        if (hc > 0) {
            m_trend = 1;
            m_cross = 1;
        } else if (lc < 0) {
            m_trend = -1;
            m_cross = -1;
        } else if (lc > 0 || hc < 0) {
            // no trend between two MAs
            m_trend = 0;
        }
    }
}

void MaAdxRbTrendAnalyser::updateTick(const Tick &tick, o3d::Bool finalize)
{
    if (m_vwap.active()) {
        m_vwap.update(tick, finalize);
    }
}
