/**
 * @brief SiiS MAADX strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "pullbackbbanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

PullbackBBAnalyser::PullbackBBAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_bollinger("bollinger", timeframe),
    m_trend(0),
    m_sig(0),
    m_confirmation(0)
{

}

PullbackBBAnalyser::~PullbackBBAnalyser()
{

}

void PullbackBBAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "bollinger", m_bollinger);

    m_trend = 0;

    StdAnalyser::init(conf);
}

void PullbackBBAnalyser::terminate()
{

}

TradeSignal PullbackBBAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
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
        m_bollinger.compute(timestamp, price().close());

        o3d::Int32 hc = DataArray::cross(price().close(), m_bollinger.middle());
        o3d::Int32 lc = DataArray::cross(price().close(), m_bollinger.middle());

        if (hc > 0) {
            m_trend = 1;
            m_sig = 1;
        } else if (lc < 0) {
            m_trend = -1;
            m_sig = -1;
        } else {
            m_sig = 0;
        }
    }

    return signal;
}

o3d::Double PullbackBBAnalyser::takeProfit(o3d::Double profitScale) const
{
//    if (m_trend > 0) {
//        return price().close().last() + profitScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
//    } else if (m_trend < 0) {
//        return price().close().last() - profitScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
//    }

    return 0.0;
}

o3d::Double PullbackBBAnalyser::stopLoss(o3d::Double lossScale, o3d::Double riskReward) const
{
//    if (m_trend > 0) {
//        return price().close().last() - riskReward * lossScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
//    } else if (m_trend < 0) {
//        return price().close().last() + riskReward * lossScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
//    }

    return 0.0;
}
