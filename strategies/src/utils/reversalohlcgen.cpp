/**
 * @brief SiiS strategy range-bar generator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-12
 */

#include "siis/utils/reversalohlcgen.h"
#include "siis/utils/math.h"

using namespace siis;

ReversalOhlcGen::ReversalOhlcGen(o3d::Int32 barSize, o3d::Int32 reversalSize, o3d::Double tickScale,
                                 Ohlc::Type ohlcType):
    m_barSize(barSize),
    m_reversalSize(reversalSize),
    m_tickScale(tickScale),
    m_ohlcType(ohlcType),
    m_lastTimestamp(0),
    m_numLastConsumed(0),
    m_tickSize(1.0),
    m_pricePrecision(1),
    m_curOhlc(nullptr),
    m_reversing(0)
{
    O3D_ASSERT(barSize > 0 && tickScale > 0 && reversalSize > 0);
}

ReversalOhlcGen::~ReversalOhlcGen()
{

}

void ReversalOhlcGen::init(o3d::Int32 pricePrecision, o3d::Double tickSize)
{
    if (pricePrecision == 0) {
        pricePrecision = 8;
    }

    if (tickSize == 0) {
        tickSize = 0.00000001;
    }

    m_pricePrecision = pricePrecision;
    m_tickSize = tickSize * m_tickScale;  // pre-mult
}

o3d::UInt32 ReversalOhlcGen::genFromTicks(const TickArray &ticks, OhlcCircular &out)
{
    o3d::UInt32 n = 0;
    m_numLastConsumed = 0;

    if (m_ohlcType == Ohlc::TYPE_LAST) {
        for (o3d::Int32 i = 0; i < ticks.getSize(); ++i) {
            if (updateFromTickLast(ticks.get(i), out)) {
                n += 1;
            }
        }
    } else if (m_ohlcType == Ohlc::TYPE_MID) {
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
    } else if (m_ohlcType == Ohlc::TYPE_ASK) {
        for (o3d::Int32 i = 0; i < ticks.getSize(); ++i) {
            if (updateFromTickAsk(ticks.get(i), out)) {
                n += 1;
            }
        }
    }

    return n;
}

o3d::Bool ReversalOhlcGen::valid() const
{
    if (m_barSize <= 0) {
        return false;
    }

    if (m_reversalSize <= 0) {
        return false;
    }

    if (m_tickScale <= 0.0) {
        return false;
    }

    if (m_tickSize <= 0.0) {
        return false;
    }

    return true;
}

o3d::Bool ReversalOhlcGen::updateFromTickLast(const Tick *tick, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    if (tick->timestamp() <= m_lastTimestamp) {
        // already done (but what if two consecutives ticks have the same exact timestamp ?)
        return false;
    }

    // compute the middle price
    const o3d::Double price = tick->last();

    if (m_curOhlc) {
        // close at reversal size
        if (m_reversing > 0) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->low()) / m_tickSize);
            if (size > m_reversalSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        } else if (m_reversing < 0) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->high() - price) / m_tickSize);
            if (size > m_reversalSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        }

        // lookup for reversal size
        if (price > m_curOhlc->high()) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->low()) / m_tickSize);
            if (size >= m_barSize) {
                m_reversing = -1;
            }
        } else if (price < m_curOhlc->low()) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->high() - price) / m_tickSize);
            if (size >= m_barSize) {
                m_reversing = 1;
            }
        }

        // is the price extend the size of the range-bar outside its allowed range
        if (price > m_curOhlc->high()) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->open()) / m_tickSize);
            if (size > m_barSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        } else if (price < m_curOhlc->low()) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->open() - price) / m_tickSize);
            if (size > m_barSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        }
    }

    if (!m_curOhlc) {
        // have a new ohlc, and init a new one as current
        isNew = true;
        m_curOhlc = out.writeElt();

        m_curOhlc->setTimestamp(tick->timestamp());
        m_curOhlc->setDuration(0.0);

        // all OHLC from the current price
        m_curOhlc->setOhlc(price);
        m_curOhlc->setVolume(0.0);
    }

    // update volumes
    m_curOhlc->setVolume(m_curOhlc->volume() + tick->volume());

    // bid high/low
    m_curOhlc->setH(o3d::max(m_curOhlc->h(), price));
    m_curOhlc->setL(o3d::min(m_curOhlc->l(), price));

    // potential close
    m_curOhlc->setC(price);

    // bar duration
    m_curOhlc->setDuration(tick->timestamp() - m_curOhlc->timestamp());

    // keep last timestamp
    m_lastTimestamp = tick->timestamp();

    return isNew;
}

o3d::Bool ReversalOhlcGen::updateFromTickMid(const Tick *tick, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    if (tick->timestamp() <= m_lastTimestamp) {
        // already done (but what if two consecutives ticks have the same exact timestamp ?)
        return false;
    }

    // compute the middle price
    const o3d::Double price = tick->price();

    if (m_curOhlc) {
        // close at reversal size
        if (m_reversing > 0) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->low()) / m_tickSize);
            if (size > m_reversalSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        } else if (m_reversing < 0) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->high() - price) / m_tickSize);
            if (size > m_reversalSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        }

        // lookup for reversal size
        if (price > m_curOhlc->high()) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->low()) / m_tickSize);
            if (size >= m_barSize) {
                m_reversing = -1;
            }
        } else if (price < m_curOhlc->low()) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->high() - price) / m_tickSize);
            if (size >= m_barSize) {
                m_reversing = 1;
            }
        }

        // is the price extend the size of the range-bar outside its allowed range
        if (price > m_curOhlc->high()) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->open()) / m_tickSize);
            if (size > m_barSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        } else if (price < m_curOhlc->low()) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->open() - price) / m_tickSize);
            if (size > m_barSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        }
    }

    if (!m_curOhlc) {
        // have a new ohlc, and init a new one as current
        isNew = true;
        m_curOhlc = out.writeElt();

        m_curOhlc->setTimestamp(tick->timestamp());
        m_curOhlc->setDuration(0.0);

        // all OHLC from the current price
        m_curOhlc->setOhlc(price);
        m_curOhlc->setVolume(0.0);
    }

    // update volumes
    m_curOhlc->setVolume(m_curOhlc->volume() + tick->volume());

    // bid high/low
    m_curOhlc->setH(o3d::max(m_curOhlc->h(), price));
    m_curOhlc->setL(o3d::min(m_curOhlc->l(), price));

    // potential close
    m_curOhlc->setC(price);

    // bar duration
    m_curOhlc->setDuration(tick->timestamp() - m_curOhlc->timestamp());

    // keep last timestamp
    m_lastTimestamp = tick->timestamp();

    return isNew;
}

o3d::Bool ReversalOhlcGen::updateFromTickBid(const Tick *tick, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    if (tick->timestamp() <= m_lastTimestamp) {
        // already done (but what if two consecutives ticks have the same exact timestamp ?)
        return false;
    }

    // compute the middle price
    const o3d::Double price = tick->bid();

    if (m_curOhlc) {
        // close at reversal size
        if (m_reversing > 0) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->low()) / m_tickSize);
            if (size > m_reversalSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        } else if (m_reversing < 0) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->high() - price) / m_tickSize);
            if (size > m_reversalSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        }

        // lookup for reversal size
        if (price > m_curOhlc->high()) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->low()) / m_tickSize);
            if (size >= m_barSize) {
                m_reversing = -1;
            }
        } else if (price < m_curOhlc->low()) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->high() - price) / m_tickSize);
            if (size >= m_barSize) {
                m_reversing = 1;
            }
        }

        // is the price extend the size of the range-bar outside its allowed range
        if (price > m_curOhlc->high()) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->open()) / m_tickSize);
            if (size > m_barSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        } else if (price < m_curOhlc->low()) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->open() - price) / m_tickSize);
            if (size > m_barSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        }
    }

    if (!m_curOhlc) {
        // have a new ohlc, and init a new one as current
        isNew = true;
        m_curOhlc = out.writeElt();

        m_curOhlc->setTimestamp(tick->timestamp());
        m_curOhlc->setDuration(0.0);

        // all OHLC from the current price
        m_curOhlc->setOhlc(price);
        m_curOhlc->setVolume(0.0);
    }

    // update volumes
    m_curOhlc->setVolume(m_curOhlc->volume() + tick->volume());

    // bid high/low
    m_curOhlc->setH(o3d::max(m_curOhlc->h(), price));
    m_curOhlc->setL(o3d::min(m_curOhlc->l(), price));

    // potential close
    m_curOhlc->setC(price);

    // bar duration
    m_curOhlc->setDuration(tick->timestamp() - m_curOhlc->timestamp());

    // keep last timestamp
    m_lastTimestamp = tick->timestamp();

    return isNew;
}

o3d::Bool ReversalOhlcGen::updateFromTickAsk(const Tick *tick, OhlcCircular &out)
{
    o3d::Bool isNew = false;

    if (tick->timestamp() <= m_lastTimestamp) {
        // already done (but what if two consecutives ticks have the same exact timestamp ?)
        return false;
    }

    // compute the middle price
    const o3d::Double price = tick->ask();

    if (m_curOhlc) {
        // close at reversal size
        if (m_reversing > 0) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->low()) / m_tickSize);
            if (size > m_reversalSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        } else if (m_reversing < 0) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->high() - price) / m_tickSize);
            if (size > m_reversalSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        }

        // lookup for reversal size
        if (price > m_curOhlc->high()) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->low()) / m_tickSize);
            if (size >= m_barSize) {
                m_reversing = -1;
            }
        } else if (price < m_curOhlc->low()) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->high() - price) / m_tickSize);
            if (size >= m_barSize) {
                m_reversing = 1;
            }
        }

        // is the price extend the size of the range-bar outside its allowed range
        if (price > m_curOhlc->high()) {
            o3d::Int32 size = static_cast<o3d::Int32>((price - m_curOhlc->open()) / m_tickSize);
            if (size > m_barSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        } else if (price < m_curOhlc->low()) {
            o3d::Int32 size = static_cast<o3d::Int32>((m_curOhlc->open() - price) / m_tickSize);
            if (size > m_barSize) {
                m_curOhlc->setConsolidated();
                m_curOhlc = nullptr;
            }
        }
    }

    if (!m_curOhlc) {
        // have a new ohlc, and init a new one as current
        isNew = true;
        m_curOhlc = out.writeElt();

        m_curOhlc->setTimestamp(tick->timestamp());
        m_curOhlc->setDuration(0.0);

        // all OHLC from the current price
        m_curOhlc->setOhlc(price);
        m_curOhlc->setVolume(0.0);
    }

    // update volumes
    m_curOhlc->setVolume(m_curOhlc->volume() + tick->volume());

    // bid high/low
    m_curOhlc->setH(o3d::max(m_curOhlc->h(), price));
    m_curOhlc->setL(o3d::min(m_curOhlc->l(), price));

    // potential close
    m_curOhlc->setC(price);

    // bar duration
    m_curOhlc->setDuration(tick->timestamp() - m_curOhlc->timestamp());

    // keep last timestamp
    m_lastTimestamp = tick->timestamp();

    return isNew;
}

o3d::Double ReversalOhlcGen::adjustPrice(o3d::Double price) const
{
    if (price == 0.0) {
        return 0.0;
    }

    return truncate(::round(price / m_tickSize) * m_tickSize, m_pricePrecision);
}
