/**
 * @brief SiiS MAADX strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#include "maadxsessionanalyser.h"

#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"

using namespace siis;

MaAdxSessionAnalyser::MaAdxSessionAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_vp("volumeprofile", timeframe),
    m_vpoc_bollinger("vpoc_bollinger", timeframe, 5, MA_EMA, 1.0, 1.0),
    m_compositeVP("composite_vp", timeframe, 60),
    m_vPocBreakout(0),
    m_vPocTrend(0),
    m_vPocs(depth)
{

}

MaAdxSessionAnalyser::~MaAdxSessionAnalyser()
{

}

o3d::String MaAdxSessionAnalyser::typeName() const
{
    return "session";
}

void MaAdxSessionAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "vp", m_vp);
    configureIndicator(conf, "vpoc_bollinger", m_vpoc_bollinger);
    // configureIndicator(conf, "composite_vp", m_compositeVP);

    m_vp.init(strategy()->market()->precisionPrice(), strategy()->market()->stepPrice());
    m_vp.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    m_vPocBreakout = 0;

    m_compositeVP.setVolumeProfile(&m_vp);
    m_vPocs.setSize(m_vp.historySize());

    TimeframeBarAnalyser::init(conf);
}

void MaAdxSessionAnalyser::terminate()
{

}

void MaAdxSessionAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_vPocBreakout = 0;

    o3d::Bool compute = true;

    if (isUpdateAtclose()) {
        compute = price().consolidated();
    }

    if (compute) {
        if (m_vp.vp().size() > 0) {
            // composite
            m_compositeVP.composite();

            m_vPocs.append(m_compositeVP.vp().pocPrice);
        }

        if (m_vPocs.size() > 2 && m_vpoc_bollinger.active()) {
            // bollinger-bands of VPOC
            DataArray vPocs = m_vPocs.asArray();

            m_vpoc_bollinger.compute(timestamp, vPocs);
            // printf("%f %f\n", vPocs.last(), m_vpoc_bollinger.lastUpper());

            if (vPocs.cross(m_vpoc_bollinger.upper()) > 0) {
                m_vPocBreakout = 1;
            } else if (vPocs.cross(m_vpoc_bollinger.lower()) < 0) {
                m_vPocBreakout = -1;
            }

            if (vPocs.last() > m_vpoc_bollinger.upper().last()) {
                m_vPocTrend = 1;
            } else if (vPocs.last() < m_vpoc_bollinger.lower().last()) {
                m_vPocTrend = -1;
            } else {
                m_vPocTrend = 0;
            }
        }
    }
}

void MaAdxSessionAnalyser::updateTick(const Tick &tick, o3d::Bool finalize)
{
    m_vp.update(tick, finalize);
}
