/**
 * @brief SiiS MAADX strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadxsiganalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

MaAdxSigAnalyser::MaAdxSigAnalyser(
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
    m_adx("adx", timeframe)
{

}

MaAdxSigAnalyser::~MaAdxSigAnalyser()
{

}

void MaAdxSigAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "slow_h_ma", m_slow_h_ma);
    configureIndictor(conf, "slow_m_ma", m_slow_m_ma);
    configureIndictor(conf, "slow_l_ma", m_slow_l_ma);

    configureIndictor(conf, "adx", m_adx);

    StdAnalyser::init(conf);
}

void MaAdxSigAnalyser::terminate()
{

}

TradeSignal MaAdxSigAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    m_slow_h_ma.compute(lastTimestamp, price().close());
    m_slow_m_ma.compute(lastTimestamp, price().price());
    m_slow_l_ma.compute(lastTimestamp, price().close());

    m_adx.compute(lastTimestamp, price().high(), price().low(), price().close());

    return signal;
}
