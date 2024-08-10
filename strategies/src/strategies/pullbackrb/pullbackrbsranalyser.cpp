/**
 * @brief SiiS Pullback range-bar strategy support/resistance analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#include "pullbackrbsranalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

PullbackRbSRAnalyser::PullbackRbSRAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    RangeBarAnalyser(strategy, name, rangeSize, depth, history, priceMethod),
    m_pivotpoint("pivotpoint", rangeSize),
    m_breakoutDirection(0),
    m_breakoutPrice(0.0),
    m_srLevel(0)
{

}

PullbackRbSRAnalyser::~PullbackRbSRAnalyser()
{

}

o3d::String PullbackRbSRAnalyser::typeName() const
{
    return "sr";
}

void PullbackRbSRAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "pivotpoint", m_pivotpoint);

    RangeBarAnalyser::init(conf);

    m_breakoutDirection = 0;
    m_breakoutPrice = 0.0;
    m_srLevel = 0;
}

void PullbackRbSRAnalyser::terminate()
{

}

void PullbackRbSRAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    if (price().consolidated()) {
        // compute at close
        m_pivotpoint.compute(timestamp, price().open(), price().high(), price().low(), price().close());
    }

    // check for support and resistance break
    m_breakoutPrice = 0.0;
    m_breakoutDirection = 0;
    m_srLevel = 0;

    // break in downward direction
    for (o3d::Int32 i = 2; i >= 0; --i) {
        o3d::Int32 rc = price().cross(m_pivotpoint.resistance(i).last());

        if (rc < 0 && (m_breakoutPrice <= 0.0 || m_pivotpoint.resistance(i).last() < m_breakoutPrice)) {
            m_breakoutPrice = m_pivotpoint.resistance(i).last();
            m_breakoutDirection = -1;
            m_srLevel = i+1;
        }
    }

    for (o3d::Int32 i = 0; i < 3; ++i) {
        o3d::Int32 sc = price().cross(m_pivotpoint.support(i).last());

        if (sc < 0 && (m_breakoutPrice <= 0.0 || m_pivotpoint.support(i).last() < m_breakoutPrice)) {
            m_breakoutPrice = m_pivotpoint.support(i).last();
            m_breakoutDirection = -1;
            m_srLevel = -i-1;
        }
    }

    // break in upward direction
    for (o3d::Int32 i = 2; i >= 0; --i) {
        o3d::Int32 sc = price().cross(m_pivotpoint.support(i).last());

        if (sc > 0 && (m_breakoutPrice <= 0.0 || m_pivotpoint.support(i).last() > m_breakoutPrice)) {
            m_breakoutPrice = m_pivotpoint.support(i).last();
            m_breakoutDirection = 1;
            m_srLevel = -i-1;
        }
    }

    for (o3d::Int32 i = 0; i < 3; ++i) {
        o3d::Int32 rc = price().cross(m_pivotpoint.resistance(i).last());

        if (rc > 0 && (m_breakoutPrice <= 0.0 || m_pivotpoint.resistance(i).last() > m_breakoutPrice)) {
            m_breakoutPrice = m_pivotpoint.resistance(i).last();
            m_breakoutDirection = 1;
            m_srLevel = i+1;
        }
    }

//    if (m_breakoutPrice > 0) {
//        printf("%s %f dir=%i sr=%i\n", timestampToStr(timestamp).toAscii().getData(), m_breakoutPrice, m_breakoutDirection, m_srLevel);
//    }
}
