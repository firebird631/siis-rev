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
    m_h_ma("ma-high", timeframe),
    m_m_ma("ma-mid", timeframe),
    m_l_ma("ma-low", timeframe)
{

}

MaAdxTrendAnalyser::~MaAdxTrendAnalyser()
{

}

void MaAdxTrendAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "ma-high", m_h_ma);
    configureIndictor(conf, "ma-mid", m_m_ma);
    configureIndictor(conf, "ma-low", m_l_ma);

    StdAnalyser::init(conf);
}

void MaAdxTrendAnalyser::terminate()
{

}

TradeSignal MaAdxTrendAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    m_h_ma.compute(lastTimestamp, price().close());
    m_m_ma.compute(lastTimestamp, price().price());
    m_l_ma.compute(lastTimestamp, price().close());

    return signal;
}
