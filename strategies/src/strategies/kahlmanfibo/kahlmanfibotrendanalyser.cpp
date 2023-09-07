/**
 * @brief SiiS strategy KahlmanFibo strategy higher trend analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-04
 */

#include "kahlmanfibotrendanalyser.h"

#include "siis/config/strategyconfig.h"

#include <o3d/core/math.h>
#include <cmath>

using namespace siis;

KahlmanFiboTrendAnalyser::KahlmanFiboTrendAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_gain(0.7),  // step 0.01 min 0.0001
    m_kahlman(true),
    m_trendTimestamp(0.0),
    m_hma("hma", timeframe),
    m_hma3("hma3", timeframe),
    m_trend(0),
    m_kHma(22),
    m_kHma3(11)
{

}

KahlmanFiboTrendAnalyser::~KahlmanFiboTrendAnalyser()
{

}

void KahlmanFiboTrendAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "hma", m_hma);
    // configureIndictor(conf, "hma3", m_hma3);  // might be half len of hma

    m_hma3.setLength(m_hma.len() / 2);

    m_trend = 0;

    StdAnalyser::init(conf);
}

void KahlmanFiboTrendAnalyser::terminate()
{

}

TradeSignal KahlmanFiboTrendAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    if (1) {  // price().consolidated()) {
        // compute only at close
        m_hma.compute(timestamp, price().price());   // compute on HL2 price
        m_hma3.compute(timestamp, price().close());  // compute on close price

        o3d::Int32 prevTrend = m_trend;

        if (m_kahlman) {
            // it reduces the lag and then improve matching with the donchian+fibo based signal
            m_kHma.compute(timestamp, m_hma.hma());
            m_kHma3.compute(timestamp, m_hma3.hma3());

            m_trend = m_kHma3.last() > m_kHma.last() ? 1 : -1;

            /*o3d::Int32 cr = DataArray::cross(m_kHma3, m_kHma);

            // changes of trend
            if (cr > 0) {
                m_trend = 1;
                m_trendTimestamp = timestamp;
            } else if (cr < 0) {
                m_trend = -1;
                m_trendTimestamp = timestamp;
            }*/
        } else {
            m_trend = m_hma3.last() > m_hma.last() ? 1 : -1;

            /*o3d::Int32 cr = DataArray::cross(m_hma3.hma3(), m_hma.hma());

            // changes of trend
            if (cr > 0) {
                m_trend = 1;
                m_trendTimestamp = timestamp;
            } else if (cr < 0) {
                m_trend = -1;
                m_trendTimestamp = timestamp;
            }*/

            // maybee donchian based indicator can m_trend = 0 if range is detected
        }

        if (prevTrend != m_trend) {
            // retain timestamp
            m_trendTimestamp = timestamp;
        }
    }

    return signal;
}

void KahlmanFiboTrendAnalyser::setUseKahlman(o3d::Bool use)
{
    m_kahlman = use;
}

KahlmanFiboTrendAnalyser::KahlmanFilter::KahlmanFilter(o3d::Int32 len, o3d::Double gain) :
    m_len(len),
    m_gain(gain),
    m_g2Sqrt(0.0),
    m_prev(0.0),
    m_last(0.0),
    m_lastTimestamp(0.0)
{
    m_gain = gain;
    m_g2Sqrt = o3d::Math::sqrt(gain*2.0);

    resize(len);
}

void KahlmanFiboTrendAnalyser::KahlmanFilter::resize(o3d::Int32 len)
{
    if (len != m_kf.getSize()) {
        m_len = len;

        m_kf.setSize(len);
        m_dk.setSize(len);
        m_smooth.setSize(len);
        m_velo.setSize(len);
    }
}

void KahlmanFiboTrendAnalyser::KahlmanFilter::compute(o3d::Double timestamp, const DataArray &price)
{
    m_prev = m_last;

    if (m_len != price.getSize()) {
        resize(price.getSize());
    }

    for (o3d::Int32 i = 0; i < m_len; ++i) {
        m_dk[i] = price[i] - (i == 0 || std::isnan(m_kf[i-1]) ? price[i] : m_kf[i-1]);
        m_smooth[i] = (i == 0 || std::isnan(m_kf[i-1]) ? price[i] : m_kf[i-1]) + m_dk[i] * m_g2Sqrt;
        m_velo[i] = (i == 0 || std::isnan(m_velo[i-1]) ? 0.0 : m_velo[i-1]) + (m_gain * m_dk[i]);
        m_kf[i] = m_smooth[i] + m_velo[i];
    }

    m_last = m_kf.last();
    m_lastTimestamp = timestamp;
}