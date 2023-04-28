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
    m_fast_h_ma("fast_h_ma", timeframe),
    m_fast_m_ma("fast_m_ma", timeframe),
    m_fast_l_ma("sfast_l_ma", timeframe),
    m_adx("adx", timeframe),
    m_trend(0),
    m_sig(0)
{

}

MaAdxSigAnalyser::~MaAdxSigAnalyser()
{

}

void MaAdxSigAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "fast_h_ma", m_fast_h_ma);
    configureIndictor(conf, "fast_m_ma", m_fast_m_ma);
    configureIndictor(conf, "fast_l_ma", m_fast_l_ma);

    configureIndictor(conf, "adx", m_adx);

    m_trend = 0;

    StdAnalyser::init(conf);
}

void MaAdxSigAnalyser::terminate()
{

}

TradeSignal MaAdxSigAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    if (price().consolidated()) {
        // compute only at close
        m_fast_h_ma.compute(lastTimestamp, price().price());
        m_fast_m_ma.compute(lastTimestamp, price().price());
        m_fast_l_ma.compute(lastTimestamp, price().price());

        m_adx.compute(lastTimestamp, price().high(), price().low(), price().close());

        o3d::Int32 hc = DataArray::cross(price().close(), m_fast_h_ma.hma());
        o3d::Int32 lc = DataArray::cross(price().close(), m_fast_l_ma.hma());

        // @todo need to take either the first cross signal either the last of the candle
        if (hc > 0) {
            m_trend = 1;
            //    o3d::Int32 size = price().close().getSize();
            //    printf("+++ %f %i %f %f - %f %f - %f %f\n", timestamp, size, price().close()[size-2], price().close()[size-1], m_fast_h_ma.hma()[size-2], m_fast_h_ma.hma()[size-1], price().timestamp()[size-2], price().timestamp()[size-1]);
            m_sig = 1;
        } else if (lc < 0) {
            m_trend = -1;
            //    o3d::Int32 size = price().close().getSize();
            //    printf("--- %f %i %f %f - %f %f - %f %f\n", timestamp, size, price().close()[size-2], price().close()[size-1], m_fast_h_ma.hma()[size-2], m_fast_h_ma.hma()[size-1], price().timestamp()[size-2], price().timestamp()[size-1]);
            m_sig = -1;
        } else {
            m_sig = 0;
        }
    }
    return signal;
}
