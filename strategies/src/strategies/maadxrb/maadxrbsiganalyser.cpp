/**
 * @brief SiiS MAADX range-bar strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "maadxrbsiganalyser.h"

#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"

using namespace siis;

MaAdxRbSigAnalyser::MaAdxRbSigAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, rangeSize, depth, history, priceMethod),
    m_fast_h_ma("fast_h_ma", rangeSize),
    m_fast_m_ma("fast_m_ma", rangeSize),
    m_fast_l_ma("sfast_l_ma", rangeSize),
    m_adx("adx", rangeSize),
    m_cvd("cvd", rangeSize, depth, "1d"),
    m_cvd_ma("cvd_ma", rangeSize),
    m_trend(0),
    m_sig(0),
    m_cvdTrend(0),
    m_cvdCross(0),
    m_confirmation(0)
{

}

MaAdxRbSigAnalyser::~MaAdxRbSigAnalyser()
{

}

o3d::String MaAdxRbSigAnalyser::typeName() const
{
    return "sig";
}

void MaAdxRbSigAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "fast_h_ma", m_fast_h_ma);
    configureIndicator(conf, "fast_m_ma", m_fast_m_ma);
    configureIndicator(conf, "fast_l_ma", m_fast_l_ma);

    configureIndicator(conf, "adx", m_adx);

    configureIndicator(conf, "cvd", m_cvd);
    configureIndicator(conf, "cvd_ma", m_cvd_ma);

    m_cvd.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    m_confirmation = 0;
    m_trend = 0;
    m_sig = 0;
    m_cvdTrend = 0;
    m_cvdCross = 0;

    RangeBarAnalyser::init(conf);
}

void MaAdxRbSigAnalyser::terminate()
{

}

void MaAdxRbSigAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_confirmation = 0;

    if (price().consolidated()) {
        if (price().close().last() > price().close().prev()) {
            m_confirmation = 1;
        } else if (price().close().last() < price().close().prev()) {
            m_confirmation = -1;
        }
    }

    o3d::Bool compute = true;

    if (isUpdateAtclose()) {
        compute = price().consolidated();
    }

    o3d::Int32 hc = 0;
    o3d::Int32 lc = 0;

    // reset
    m_cvdCross = 0;
    m_sig = 0;

    if (compute) {
        m_fast_h_ma.compute(timestamp, price().high());
        m_fast_m_ma.compute(timestamp, price().price());
        m_fast_l_ma.compute(timestamp, price().low());

        m_adx.compute(timestamp, price().high(), price().low(), price().close());

        hc = DataArray::cross(price().close(), m_fast_h_ma.hma());
        lc = DataArray::cross(price().close(), m_fast_l_ma.hma());

        if (m_cvd.active() && m_cvd_ma.active()) {
            m_cvd_ma.compute(timestamp, m_cvd.cvd().asArray());

            o3d::Int32 cvdTrend = 0;
            if (m_cvd.last() > m_cvd_ma.last()) {
                cvdTrend = 1;
            } else if (m_cvd.last() < m_cvd_ma.last()) {
                cvdTrend = -1;
            }

            if (m_cvdTrend != cvdTrend && cvdTrend != 0) {
                m_cvdCross = cvdTrend;
            }

            m_cvdTrend = cvdTrend;
        }  

        if (hc > 0) {
            m_trend = 1;
            m_sig = 1;
        } else if (lc < 0) {
            m_trend = -1;
            m_sig = -1;
        } else if (lc > 0 || hc < 0) {
            // no trend between two MAs
            m_trend = 0;
        }
    }
}

void MaAdxRbSigAnalyser::updateTick(const Tick &tick, o3d::Bool finalize)
{
    if (m_cvd.active()) {
        m_cvd.update(tick, finalize);
    }
}

o3d::Double MaAdxRbSigAnalyser::takeProfit(o3d::Double profitScale) const
{
    if (m_trend > 0) {
        return price().close().last() + profitScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
    } else if (m_trend < 0) {
        return price().close().last() - profitScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
    }

    return 0.0;
}

o3d::Double MaAdxRbSigAnalyser::stopLoss(o3d::Double lossScale, o3d::Double riskReward) const
{
    if (m_trend > 0) {
        return price().close().last() - riskReward * lossScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
    } else if (m_trend < 0) {
        return price().close().last() + riskReward * lossScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
    }

    return 0.0;
}
