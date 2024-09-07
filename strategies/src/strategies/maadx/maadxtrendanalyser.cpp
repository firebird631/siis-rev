/**
 * @brief SiiS MAADX strategy trend analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadxtrendanalyser.h"

#include "siis/strategy.h"
#include "siis/config/strategyconfig.h"

using namespace siis;

MaAdxTrendAnalyser::MaAdxTrendAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_slow_h_ma("slow_h_ma", timeframe),
    m_slow_m_ma("slow_m_ma", timeframe),
    m_slow_l_ma("slow_l_ma", timeframe),
    m_vwap("vwap", timeframe, depth),
    m_trend(0),
    m_cross(0),
    m_sig(0),
    m_vwapTrend(0)
{

}

MaAdxTrendAnalyser::~MaAdxTrendAnalyser()
{

}

o3d::String MaAdxTrendAnalyser::typeName() const
{
    return "trend";
}

void MaAdxTrendAnalyser::init(const AnalyserConfig &conf)
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

    TimeframeBarAnalyser::init(conf);
}

void MaAdxTrendAnalyser::terminate()
{

}

void MaAdxTrendAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
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
            if (m_vwap.current() && m_vwap.current()->vwap.getSize() > 3) {
                if (m_vwap.current()->vwap.at(-3) < m_vwap.current()->vwap.at(-2) < m_vwap.current()->vwap.at(-1)) {
                    m_vwapTrend = 1;
                } else if (m_vwap.current()->vwap.at(-3) > m_vwap.current()->vwap.at(-2) > m_vwap.current()->vwap.at(-1)) {
                    m_vwapTrend = -1;
                } else {
                    m_vwapTrend = 0;
                }
            }

//            if (price().close().last() > m_vwap.last()) {
//                m_vwapTrend = 1;
//            } else if (price().close().last() < m_vwap.last()) {
//                m_vwapTrend = -1;
//            } else {
//                m_vwapTrend = 0;
//            }
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

void MaAdxTrendAnalyser::updateTick(const Tick &tick, o3d::Bool finalize)
{
    if (m_vwap.active()) {
        m_vwap.update(tick, finalize);
    }
}
