/**
 * @brief SiiS MAADX range-bar strategy trend analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadxrbtrendanalyser.h"

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
    m_trend(0)
{

}

MaAdxRbTrendAnalyser::~MaAdxRbTrendAnalyser()
{

}

o3d::String MaAdxRbTrendAnalyser::typeName() const
{
    return "trend";
}

void MaAdxRbTrendAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "slow_h_ma", m_slow_h_ma);
    configureIndictor(conf, "slow_m_ma", m_slow_m_ma);
    configureIndictor(conf, "slow_l_ma", m_slow_l_ma);

    m_trend = 0;

    RangeBarAnalyser::init(conf);
}

void MaAdxRbTrendAnalyser::terminate()
{

}

void MaAdxRbTrendAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_slow_h_ma.compute(timestamp, price().high());
    m_slow_m_ma.compute(timestamp, price().price());
    m_slow_l_ma.compute(timestamp, price().low());

    o3d::Int32 hc = DataArray::cross(price().close(), m_slow_h_ma.hma());
    o3d::Int32 lc = DataArray::cross(price().close(), m_slow_l_ma.hma());

    if (hc > 0) {
        m_trend = 1;
    } else if (lc < 0) {
        m_trend = -1;
    } else if (hc < 0 || lc > 0) {
        // no trend between two MAs
        m_trend = 0;
    }
}
