/**
 * @brief SiiS MAADX range-bar strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#include "maadxrbsessionanalyser.h"

#include "siis/config/strategyconfig.h"
#include "siis/strategy.h"

using namespace siis;

MaAdxRbSessionAnalyser::MaAdxRbSessionAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, rangeSize, depth, history, priceMethod),
    m_vp("volumeprofile", rangeSize),
    m_vpoc_bollinger("vpoc_bollinger", rangeSize, 5, MA_EMA, 1.0, 1.0),
    m_compositeVP("composite_vp", rangeSize, 30),
    m_ma("ma", rangeSize, 8),
    m_vPocBreakout(0),
    m_vPocTrend(0),
    m_vpCross(0),
    m_vPocs(depth)
{

}

MaAdxRbSessionAnalyser::~MaAdxRbSessionAnalyser()
{

}

o3d::String MaAdxRbSessionAnalyser::typeName() const
{
    return "session";
}

void MaAdxRbSessionAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "vp", m_vp);
    configureIndicator(conf, "vpoc_bollinger", m_vpoc_bollinger);
    // configureIndicator(conf, "composite_vp", m_compositeVP);
    configureIndicator(conf, "ma", m_ma);

    m_compositeVP.setLength(conf.data().get("composite-vp-len", 30).asInt());

    m_vp.init(strategy()->market()->precisionPrice(), strategy()->market()->stepPrice());
    m_vp.setSession(strategy()->sessionOffset(), strategy()->sessionDuration());

    m_compositeVP.setVolumeProfile(&m_vp);
    m_vPocs.setSize(m_vp.historySize());

    m_vPocBreakout = 0;
    m_vPocTrend = 0;
    m_vpCross = 0;

    RangeBarAnalyser::init(conf);
}

void MaAdxRbSessionAnalyser::terminate()
{

}

void MaAdxRbSessionAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    // reset
    m_vPocBreakout = 0;
    m_vpCross = 0;

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

        if (m_vPocs.size() > 2) {
            // collect VPOC
            // DataArray vPocs;

            // for (o3d::Int32 i = 0; i < m_vp.vp().size(); ++i) {
            //     vPocs.push(m_vp.previous(i)->pocPrice);
            // }

            DataArray vPocs = m_vPocs.asArray();

            if (m_vpoc_bollinger.active()) {
                // bollinger-bands of VPOC
                m_vpoc_bollinger.compute(timestamp, vPocs);

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

            if (m_ma.active()) {
                m_ma.compute(timestamp, price().close());
                m_vpCross = m_ma.wma().cross(vPocs);

                // need up bar
                if (m_vpCross > 0 && price().close().at(-2) <= price().open().at(-2)) {
                    m_vpCross = 0;
                }

                // need down bar
                if (m_vpCross < 0 && price().close().at(-2) >= price().open().at(-2)) {
                    m_vpCross = 0;
                }
            }

/*            // or second solution pure bar
            if (price().close().last() > vPocs.last() && price().open().last() < vPocs.last() && price().close().at(-2) < vPocs.last()) {
                m_vpCross = 1;
            } else if (price().close().last() < vPocs.last() && price().open().last() > vPocs.last() && price().close().at(-2) < vPocs.last()) {
                m_vpCross = -1;
            }*/
        }
    }
}

void MaAdxRbSessionAnalyser::updateTick(const Tick &tick, o3d::Bool finalize)
{
    m_vp.update(tick, finalize);
}
