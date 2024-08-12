/**
 * @brief SiiS Ichimoku strategy signal analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#include "ichimokustsiganalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

IchimokuStSigAnalyser::IchimokuStSigAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_ichimoku("ichimoku", timeframe),
    m_span_cross_ofs(-26),
    m_chikou_ofs(-52),
    m_priceCross(0),
    m_priceDir(0),
    m_chikouCross(0),
    m_chikouDir(0),
    m_tenkanDir(0),
    m_cloudDir(0)
{

}

IchimokuStSigAnalyser::~IchimokuStSigAnalyser()
{

}

o3d::String IchimokuStSigAnalyser::typeName() const
{
    return "sig";
}

void IchimokuStSigAnalyser::init(const AnalyserConfig &conf)
{
    configureIndictor(conf, "ichimoku", m_ichimoku);

    m_span_cross_ofs = -m_ichimoku.kijunLen();
    m_chikou_ofs = -m_ichimoku.senkouSpanBLen();

//    m_span_cross_ofs = conf.getAnalyserInt("ichimoku", "span-cross-offset", -26);
//    m_chikou_ofs = conf.getAnalyserInt("ichimoku", "chikou-offset", -52);

    TimeframeBarAnalyser::init(conf);
}

void IchimokuStSigAnalyser::terminate()
{

}

void IchimokuStSigAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    // reset
    m_priceCross = 0;
    m_chikouCross = 0;
    m_priceDir = 0;
    m_chikouDir = 0;
    m_tenkanDir = 0;
    m_cloudDir = 0;

    if (price().consolidated()) {
        m_ichimoku.compute(timestamp, price().high(), price().low(), price().close());

        // prices cross/direction
        o3d::Int32 i = m_span_cross_ofs - 1;
        if (m_ichimoku.ssa().at(i) < m_ichimoku.ssb().at(i)) {
            // cloud A<B
            if (price().close().at(-3) <= m_ichimoku.ssb().at(i-1) && price().close().at(-2) > m_ichimoku.ssb().at(i)) {
                m_priceCross = 1;
            } else if (price().close().at(-3) >= m_ichimoku.ssa().at(i-1) && price().close().at(-2) < m_ichimoku.ssa().at(i)) {
                m_priceCross = -1;
            }

            if (price().close().at(-2) > m_ichimoku.ssb().at(i)) {
                m_priceDir = 1;
            } else if (price().close().at(-2) < m_ichimoku.ssa().at(i)) {
                m_priceDir = -1;
            }
        } else if (m_ichimoku.ssa().at(i) > m_ichimoku.ssb().at(i)) {
            // cloud A>B
            if (price().close().at(-3) <= m_ichimoku.ssa().at(i-1) && price().close().at(-2) > m_ichimoku.ssa().at(i)) {
                m_priceCross = 1;
            } else if (price().close().at(-3) >= m_ichimoku.ssb().at(i-1) && price().close().at(-2) < m_ichimoku.ssb().at(i)) {
                m_priceCross = -1;
            }

            if (price().close().at(-2) > m_ichimoku.ssa().at(i)) {
                m_priceDir = 1;
            } else if (price().close().at(-2) < m_ichimoku.ssb().at(i)) {
                m_priceDir = -1;
            }
        }

        // chikou cross/direction past cloud
        i = m_chikou_ofs - 1;

        if (m_ichimoku.ssa().at(i) < m_ichimoku.ssb().at(i)) {
            // cloud A<B
            if (price().close().at(-3) <= m_ichimoku.ssb().at(i-1) && price().close().at(-2) > m_ichimoku.ssb().at(i)) {
                m_chikouCross = 1;
            } else if (price().close().at(-3) >= m_ichimoku.ssa().at(i-1) && price().close().at(-2) < m_ichimoku.ssa().at(i)) {
                m_chikouCross = -1;
            }

            if (price().close().at(-2) > m_ichimoku.ssb().at(i)) {
                m_chikouDir = 1;
            } else if (price().close().at(-2) < m_ichimoku.ssa().at(i)) {
                m_chikouDir = -1;
            }
        } else if (m_ichimoku.ssa().at(i) > m_ichimoku.ssb().at(i)) {
            // cloud A>B
            if (price().close().at(-3) <= m_ichimoku.ssa().at(i-1) && price().close().at(-2) > m_ichimoku.ssa().at(i)) {
                m_chikouCross = 1;
            } else if (price().close().at(-3) >= m_ichimoku.ssb().at(i-1) && price().close().at(-2) < m_ichimoku.ssb().at(i)) {
                m_chikouCross = -1;
            }

            if (price().close().at(-2) > m_ichimoku.ssa().at(i)) {
                m_chikouDir = 1;
            } else if (price().close().at(-2) < m_ichimoku.ssb().at(i)) {
                m_chikouDir = -1;
            }
        }

        // tenkan direction
        if (m_ichimoku.tenkan().last() > m_ichimoku.kijun().last()) {
            // tenkan above kijun
            m_tenkanDir = 1;
        } else if (m_ichimoku.tenkan().last() < m_ichimoku.kijun().last()) {
            // tenkan below kijun
            m_tenkanDir = -1;
        }

        // future cloud direction
        if (m_ichimoku.ssa().last() > m_ichimoku.ssb().last()) {
            // cloud A>B green
            m_cloudDir = 1;
        } else if (m_ichimoku.ssa().last() < m_ichimoku.ssb().last()) {
            // cloud A<B red
            m_cloudDir = -1;
        }
    }
}

o3d::Double IchimokuStSigAnalyser::stopPrice(o3d::Int32 direction) const
{
    if (direction > 0) {
        return o3d::min(m_ichimoku.ssa().last(), m_ichimoku.ssb().last());
    } else if (direction < 0) {
        return o3d::max(m_ichimoku.ssa().last(), m_ichimoku.ssb().last());
    }

    return 0.0;
}
