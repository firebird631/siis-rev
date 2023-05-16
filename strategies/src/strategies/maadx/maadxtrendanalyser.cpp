/**
 * @brief SiiS MAADX strategy trend analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadxtrendanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

MaAdxTrendAnalyser::MaAdxTrendAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_slow_h_ma("slow_h_ma", timeframe),
    m_slow_m_ma("slow_m_ma", timeframe),
    m_slow_l_ma("slow_l_ma", timeframe),
    m_trend(0)
{

}

MaAdxTrendAnalyser::~MaAdxTrendAnalyser()
{

}

void MaAdxTrendAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "slow_h_ma", m_slow_h_ma);
    configureIndictor(conf, "slow_m_ma", m_slow_m_ma);
    configureIndictor(conf, "slow_l_ma", m_slow_l_ma);

    m_trend = 0;

    StdAnalyser::init(conf);
}

void MaAdxTrendAnalyser::terminate()
{

}

TradeSignal MaAdxTrendAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    m_slow_h_ma.compute(timestamp, price().high());
    m_slow_m_ma.compute(timestamp, price().price());
    m_slow_l_ma.compute(timestamp, price().low());

    o3d::Int32 hc = DataArray::cross(price().close(), m_slow_h_ma.hma());
    o3d::Int32 lc = DataArray::cross(price().close(), m_slow_l_ma.hma());

    if (hc > 0) {
        m_trend = 1;
    } else if (lc < 0) {
        m_trend = -1;
    }

    // no trend between two MAs
//    if (price().last() <= m_slow_h_ma.last() && price().last() >= m_slow_l_ma.last()) {
//        m_trend = 0;
//    }

    return signal;
}
