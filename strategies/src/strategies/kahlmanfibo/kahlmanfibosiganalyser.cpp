/**
 * @brief SiiS strategy KahlmanFibo strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#include "kahlmanfibosiganalyser.h"

#include "siis/config/strategyconfig.h"

#include <o3d/core/math.h>
#include <cmath>

using namespace siis;

KahlmanFiboSigAnalyser::KahlmanFiboSigAnalyser(
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
    m_sigTimestamp(0.0),
    m_donchian("donchian", timeframe),
    m_hma("hma", timeframe),
    m_hma3("hma3", timeframe),
    m_trend(0),
    m_sig(0),
    m_dfTrend(0),
    m_confirmation(0),
    m_kHma(22),
    m_kHma3(11)
{

}

KahlmanFiboSigAnalyser::~KahlmanFiboSigAnalyser()
{

}

void KahlmanFiboSigAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "hma", m_hma);
    configureIndictor(conf, "hma3", m_hma3);  // might be half len of hma
    configureIndictor(conf, "donchian", m_donchian);

    m_confirmation = 0;
    m_trend = 0;
    m_sig = 0;

    StdAnalyser::init(conf);
}

void KahlmanFiboSigAnalyser::terminate()
{

}

TradeSignal KahlmanFiboSigAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    m_confirmation = 0;

    if (price().consolidated()) {
        if (price().close().last() > price().close().prev()) {
            m_confirmation = 1;
        } else if (price().close().last() < price().close().prev()) {
            m_confirmation = -1;
        }
    }

    if (1) {  // price().consolidated()) {
        // compute only at close
        m_hma.compute(timestamp, price().price());   // compute on HL2 price
        m_hma3.compute(timestamp, price().close());  // compute on close price
        m_donchian.compute(timestamp, price().high(), price().low());

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

        // donchian fibo testing
        donchianFibo(timestamp);
    }

    return signal;
}

void KahlmanFiboSigAnalyser::setUseKahlman(o3d::Bool use)
{
    m_kahlman = use;
}

o3d::Double KahlmanFiboSigAnalyser::takeProfit(o3d::Double profitScale) const
{
//    if (m_trend > 0) {
//        return price().close().last() + profitScale * (m_donchian.lastUpper() - m_donchian.lastLower());
//    } else if (m_trend < 0) {
//        return price().close().last() - profitScale * (m_donchian.lastUpper() - m_donchian.lastLower());
//    }

    if (m_trend > 0) {
        return m_donchian.lastUpper();
    } else if (m_trend < 0) {
        return m_donchian.lastLower();
    }

    return 0.0;
}

o3d::Double KahlmanFiboSigAnalyser::stopLoss(o3d::Double lossScale, o3d::Double riskReward) const
{
//    if (m_trend > 0) {
//        return price().close().last() - riskReward * lossScale * (m_donchian.lastUpper() - m_donchian.lastLower());
//    } else if (m_trend < 0) {
//        return price().close().last() + riskReward * lossScale * (m_donchian.lastUpper() - m_donchian.lastLower());
//    }

    if (m_trend > 0) {
        return m_donchian.lastLower() - o3d::abs(m_hma.last() - m_hma3.last());
    } else if (m_trend < 0) {
        return m_donchian.lastUpper() + o3d::abs(m_hma.last() - m_hma3.last());
    }

    return 0.0;
}

void KahlmanFiboSigAnalyser::donchianFibo(o3d::Double timestamp)
{
    const size_t LAST = 1;
    const size_t PREV = 0;

    // pullback levels computed for previous and current candle for testing cross)
    o3d::Double dist[2] = {0.0, 0.0};
    o3d::Double med[2] = {0.0, 0.0};

    o3d::Double hiFib[2] = {0.0, 0.0};   // highest fibo
    // o3d::Double centerHiFib[2] = {0.0, 0.0};   // center high fibo
    // o3d::Double centerLoFib[2] = {0.0, 0.0};   // center low fibo
    o3d::Double loFib[2] = {0.0, 0.0};   // lowest fibo

    // distance of the channel
    dist[PREV] = m_donchian.prevUpper() - m_donchian.prevLower();
    dist[LAST] = m_donchian.lastUpper() - m_donchian.lastLower();

    // median of the channel
    med[PREV] = m_donchian.prevUpper() + m_donchian.prevLower() * 0.5;
    med[LAST] = m_donchian.lastUpper() + m_donchian.lastLower() * 0.5;

    // highest fib
    hiFib[PREV] = m_donchian.prevUpper() - dist[PREV] * 0.236;
    hiFib[LAST] = m_donchian.lastUpper() - dist[LAST] * 0.236;

    // center high fib
    // centerHiFib[PREV] = m_donchian.prevUpper() - dist[PREV] * 0.382;
    // centerHiFib[LAST] = m_donchian.lastUpper() - dist[LAST] * 0.382;

    // center low fib
    // centerLoFib[PREV] = m_donchian.prevUpper() - dist[PREV] * 0.618;
    // centerLoFib[LAST] = m_donchian.lastUpper() - dist[LAST] * 0.618;

    // lowest fib
    loFib[PREV] = m_donchian.prevUpper() - dist[PREV] * 0.764;
    loFib[LAST] = m_donchian.lastUpper() - dist[LAST] * 0.764;

    // close cross over hi fib level : market enters uptrend
    o3d::Bool evUpIn = false;
    if (price().close().prev() < hiFib[PREV] && price().close().last() > hiFib[LAST]) {
        evUpIn = true;
    }

    // close cross under hi fib level : market leaves uptrend
    o3d::Int32 evUpOut = false;
    if (price().close().prev() > hiFib[PREV] && price().close().last() < hiFib[LAST]) {
        evUpOut = true;
    }

    // market enters down-trend
    o3d::Bool evDnIn = false;
    if (price().close().prev() > loFib[PREV] && price().close().last() < loFib[LAST]) {
        evDnIn = true;
    }

    // market leaves down-trend
    o3d::Bool evDnOut = false;
    if (price().close().prev() < loFib[PREV] && price().close().last() > loFib[LAST]) {
        evDnOut = true;
    }

    o3d::Int32 upTrend = false;
    o3d::Int32 dnTrend = false;

    // mean up trend
    if (evUpIn && !evUpOut) {
        upTrend = true;
        m_dfTrend = 1;
    }

    // mean down trend
    if (evDnIn && !evDnOut) {
        dnTrend = true;
        m_dfTrend = -1;
    }

    if (!upTrend && !dnTrend) {
        // range
        m_dfTrend = 0;
    }

    // evUpOut -> red cross -> sell
    // evDnOut -> green cross -> buy
    if (evUpOut) {
        m_sig = -1;
        m_sigTimestamp = timestamp;
        // printf("donchian fibo short\n");
    } else if (evDnOut) {
        m_sig = 1;
        m_sigTimestamp = timestamp;
        // printf("donchian fibo long\n");
    } else {
        m_sig = 0;
    }
}

KahlmanFiboSigAnalyser::KahlmanFilter::KahlmanFilter(o3d::Int32 len, o3d::Double gain) :
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

void KahlmanFiboSigAnalyser::KahlmanFilter::resize(o3d::Int32 len)
{
    if (len != m_kf.getSize()) {
        m_len = len;

        m_kf.setSize(len);
        m_dk.setSize(len);
        m_smooth.setSize(len);
        m_velo.setSize(len);
    }
}

void KahlmanFiboSigAnalyser::KahlmanFilter::compute(o3d::Double timestamp, const DataArray &price)
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
