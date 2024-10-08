/**
 * @brief SiiS SuperTrend range-bar strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#include "supertrendrbsiganalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

SuperTrendRbSigAnalyser::SuperTrendRbSigAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 barSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, barSize, depth, history, priceMethod, 1.0),
    m_gain(0.7),  // step 0.01 min 0.0001
    m_kahlman(true),
    m_trendTimestamp(0.0),
    m_hma("hma", barSize),
    m_hma3("hma3", barSize),
    m_superTrend("supertrend", barSize),
    m_trend(0),
    m_kHma("kHma", barSize, 22),
    m_kHma3("kHma3", barSize, 11)
{

}

SuperTrendRbSigAnalyser::~SuperTrendRbSigAnalyser()
{

}

o3d::String SuperTrendRbSigAnalyser::typeName() const
{
    return "sig";
}

void SuperTrendRbSigAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "hma", m_hma);
    // configureIndictor(conf, "hma3", m_hma3);  // might be half len of hma

    m_hma3.setLength(m_hma.len() / 2);
    configureIndicator(conf, "supertrend", m_superTrend);

    // m_kHma.setLength(22);
    // m_kHma3.setLength(11);

    m_trend = 0;

    RangeBarAnalyser::init(conf);
}

void SuperTrendRbSigAnalyser::terminate()
{

}

void SuperTrendRbSigAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    if (price().consolidated()) {
        // compute only at close
        m_hma.compute(timestamp, price().price());   // compute on HL2 price
        m_hma3.compute(timestamp, price().close());  // compute on close price
        m_superTrend.compute(timestamp, price().high(), price().low(), price().close());

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
        }

        if (prevTrend != m_trend) {
            // retain timestamp
            m_trendTimestamp = timestamp;
        }
    }
}

o3d::Int32 SuperTrendRbSigAnalyser::sig() const
{
    return m_superTrend.positionChange();
}

o3d::Double SuperTrendRbSigAnalyser::takeProfit(o3d::Int32 direction, o3d::Double profitScale) const
{
    return 0.0;
}

o3d::Double SuperTrendRbSigAnalyser::stopLoss(o3d::Int32 direction, o3d::Double riskScale, o3d::Double onePipMeans) const
{
    return m_superTrend.last() - direction * riskScale * onePipMeans;
}

o3d::Double SuperTrendRbSigAnalyser::dynamicStopLoss(o3d::Int32 direction,
                                                   o3d::Double curStopLoss,
                                                   o3d::Double distance) const
{
    if (m_superTrend.position() != direction) {
        return 0.0;
    }

    if (direction > 0) {
        return o3d::max(m_superTrend.last() * (1.0 - distance), curStopLoss);
    } else if (direction < 0) {
        return o3d::min(m_superTrend.last() * (1.0 + distance), curStopLoss);
    }

    return 0.0;
}
