/**
 * @brief SiiS strategy candle generator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#include "siis/utils/ohlcgen.h"

using namespace siis;

OhlcGen::OhlcGen(o3d::Double fromTf, o3d::Double toTf, Ohlc::Type ohlcType) :
    m_fromTf(fromTf),
    m_toTf(toTf),
    m_ohlcType(ohlcType),
    m_lastTimestamp(0),
    m_numLastConsumed(0),
    m_curOhlc(nullptr)
{
    O3D_ASSERT((fromTf == 0.0) || (fromTf > 0 && static_cast<o3d::Int32>(toTf) % static_cast<o3d::Int32>(fromTf) == 0));
}

OhlcGen::~OhlcGen()
{

}

o3d::UInt32 OhlcGen::genFromTicks(const TickArray &ticks, OhlcCircular &out)
{
    o3d::UInt32 n = 0;
    m_numLastConsumed = 0;

    if (m_ohlcType == Ohlc::TYPE_MID) {
        for (o3d::Int32 i = 0; i < ticks.getSize(); ++i) {
            if (updateFromTickMid(ticks.get(i), out)) {
                n += 1;
            }
        }
    } else if (m_ohlcType == Ohlc::TYPE_BID) {
        for (o3d::Int32 i = 0; i < ticks.getSize(); ++i) {
            if (updateFromTickBid(ticks.get(i), out)) {
                n += 1;
            }
        }
    } else if (m_ohlcType == Ohlc::TYPE_OFR) {
        for (o3d::Int32 i = 0; i < ticks.getSize(); ++i) {
            if (updateFromTickOfr(ticks.get(i), out)) {
                n += 1;
            }
        }
    }

    return n;
}

o3d::UInt32 OhlcGen::genFromOhlc(const OhlcArray &ohlc, OhlcCircular &out)
{
    o3d::UInt32 n = 0;
    m_numLastConsumed = 0;

    if (m_ohlcType == Ohlc::TYPE_MID) {
        for (o3d::Int32 i = 0; i < ohlc.getSize(); ++i) {
            if (updateFromOhlcMid(ohlc.get(i), out)) {
                n += 1;
            }
        }
    } else if (m_ohlcType == Ohlc::TYPE_BID) {
        for (o3d::Int32 i = 0; i < ohlc.getSize(); ++i) {
            if (updateFromOhlcBid(ohlc.get(i), out)) {
                n += 1;
            }
        }
    } else if (m_ohlcType == Ohlc::TYPE_OFR) {
        for (o3d::Int32 i = 0; i < ohlc.getSize(); ++i) {
            if (updateFromOhlcOfr(ohlc.get(i), out)) {
                n += 1;
            }
        }
    }

    return n;
}

o3d::Bool OhlcGen::valid() const
{
    if (m_fromTf == 0.0) {
        if (m_toTf <= 0.0) {
            return false;
        }

        return true;
    } else if (m_fromTf > 0.0) {
        return (o3d::Int32(m_toTf) % o3d::Int32(m_fromTf)) == 0;
    } else {
        return false;
    }
}

o3d::Bool OhlcGen::updateFromTickMid(const Tick *tick, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    if (tick->timestamp() <= m_lastTimestamp) {
        // already done (but what if two consecutives ticks have the same exact timestamp ?)
        return false;
    }

    // compute the middle price
    const o3d::Double price = (tick->bid() + tick->ofr()) * 0.5;

    o3d::Double curBaseTime = baseTime(tick->timestamp());

    if (m_curOhlc && !m_curOhlc->consolidated() && (m_curOhlc->timestamp() + m_toTf) <= curBaseTime) {
        // need to close the current ohlc
        m_curOhlc->setConsolidated();
        m_curOhlc = nullptr;
    }

    if (!m_curOhlc) {
        // have a new ohlc, and init a new one as current
        isNew = true;
        m_curOhlc = out.writeElt();

        m_curOhlc->setTimestamp(curBaseTime);
        m_curOhlc->setTimeframe(m_toTf);

        // all OHLC from the current price
        m_curOhlc->setOhlc(price);
    }

    // update volumes
    m_curOhlc->setVolume(m_curOhlc->volume() + tick->volume());

    // bid high/low
    m_curOhlc->setH(o3d::max(m_curOhlc->h(), price));
    m_curOhlc->setL(o3d::min(m_curOhlc->l(), price));

    // potential close
    m_curOhlc->setC(price);

    // keep last timestamp
    m_lastTimestamp = tick->timestamp();

    return isNew;
}

o3d::Bool OhlcGen::updateFromTickBid(const Tick *tick, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    if (tick->timestamp() <= m_lastTimestamp) {
        // already done (but what if two consecutives ticks have the same exact timestamp ?)
        return false;
    }

    // the bid price
    const o3d::Double price = tick->bid();

    o3d::Double curBaseTime = baseTime(tick->timestamp());

    if (m_curOhlc && !m_curOhlc->consolidated() && (m_curOhlc->timestamp() + m_toTf) <= curBaseTime) {
        // need to close the current ohlc
        m_curOhlc->setConsolidated();
        m_curOhlc = nullptr;
    }

    if (!m_curOhlc) {
        // have a new ohlc, and init a new one as current
        isNew = true;
        m_curOhlc = out.writeElt();

        m_curOhlc->setTimestamp(curBaseTime);
        m_curOhlc->setTimeframe(m_toTf);

        // all OHLC from the current price
        m_curOhlc->setOhlc(price, price, price, price);
    }

    // update volumes
    m_curOhlc->setVolume(m_curOhlc->volume() + tick->volume());

    // bid high/low
    m_curOhlc->setH(o3d::max(m_curOhlc->h(), price));
    m_curOhlc->setL(o3d::min(m_curOhlc->l(), price));

    // potential close
    m_curOhlc->setC(price);

    // keep last timestamp
    m_lastTimestamp = tick->timestamp();

    return isNew;
}

o3d::Bool OhlcGen::updateFromTickOfr(const Tick *tick, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    if (tick->timestamp() <= m_lastTimestamp) {
        // already done (but what if two consecutives ticks have the same exact timestamp ?)
        return false;
    }

    // the ofr price
    const o3d::Double price = tick->ofr();

    o3d::Double curBaseTime = baseTime(tick->timestamp());

    if (m_curOhlc && !m_curOhlc->consolidated() && (m_curOhlc->timestamp() + m_toTf) <= curBaseTime) {
        // need to close the current ohlc
        m_curOhlc->setConsolidated();
        m_curOhlc = nullptr;
    }

    if (!m_curOhlc) {
        // have a new ohlc, and init a new one as current
        isNew = true;
        m_curOhlc = out.writeElt();

        m_curOhlc->setTimestamp(curBaseTime);
        m_curOhlc->setTimeframe(m_toTf);

        // all OHLC from the current price
        m_curOhlc->setOhlc(price, price, price, price);
    }

    // update volumes
    m_curOhlc->setVolume(m_curOhlc->volume() + tick->volume());

    // bid high/low
    m_curOhlc->setH(o3d::max(m_curOhlc->h(), price));
    m_curOhlc->setL(o3d::min(m_curOhlc->l(), price));

    // potential close
    m_curOhlc->setC(price);

    // keep last timestamp
    m_lastTimestamp = tick->timestamp();

    return isNew;
}

o3d::Bool OhlcGen::updateFromOhlcMid(const Ohlc *ohlc, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    return isNew;
}

o3d::Bool OhlcGen::updateFromOhlcBid(const Ohlc *ohlc, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    return isNew;
}

o3d::Bool OhlcGen::updateFromOhlcOfr(const Ohlc *ohlc, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    return isNew;
}
