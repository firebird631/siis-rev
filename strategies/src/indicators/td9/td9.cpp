/**
 * @brief SiiS tom demark 9 indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/indicators/td9/td9.h"
#include "siis/utils/common.h"

using namespace siis;
using o3d::Logger;
using o3d::Debug;

Td9::Td9(const o3d::String &name, o3d::Double timeframe) :
    Indicator (name, timeframe),
    m_highLow(0.0)
{
}

Td9::Td9(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator (name, timeframe),
    m_highLow(0.0)
{

}

void Td9::setConf(IndicatorConfig conf)
{

}

void Td9::compute(o3d::Double timestamp,
                 const DataArray &timestamps,
                 const DataArray &high,
                 const DataArray &low,
                 const DataArray &close)
{
    // @todo
    // delta of 0 mean overwrite the last
    o3d::Int32 delta = o3d::min(o3d::Int32((timestamp - lastTimestamp()) / timeframe()), timestamps.getSize());

    // base index (minus one if update the current candle)
    o3d::Int32 base = delta + (timestamp == lastTimestamp() ? 1 : 0);
    o3d::Int32 size = timestamps.getSize();

    for (o3d::Int32 b = size-base; b < size; ++b) {
        // reset latest
        m_c.reset();
        m_cd.reset();
        m_aggCd.reset();

        // continue with the next non processed candle or the current non closed
        td9(b, high, low, close);

        if (timestamps[b] > lastTimestamp()) {
            // validate last candle
            m_prevC = m_c;

            if (m_cd.c > 0) {
                // validate a count-down only if valid at this candle
                m_prevCd = m_cd;
            }

            if (m_aggCd.c > 0) {
                // similar of aggressive count-down
                m_prevAggCd = m_aggCd;
            }
        }
    }

    done(timestamp);
}

o3d::Int32 Td9::lookback() const
{
    return 4;
}

void Td9::td9(o3d::Int32 b, const DataArray &high, const DataArray &low, const DataArray &close)
{
    // True low/True high – is the lowest and the highest point for a setup,
    // BUT including gaps before bar 1 and the days after the 9-th bar, qualifying for a setup bar.
    // @todo and test CD/ACD

    if ((close[b] <= close[b-4]) && (close[b-1] >= close[b-5])) {
        // buy-setup
        m_c.d = 1;  // momentum flip, buy setup
        m_c.c = 1;  // new buy setup
        m_highLow = high[b];
    }
    else if ((close[b] >= close[b-4]) && (close[b-1] <= close[b-5])) {
        // sell-setup
        m_c.d = -1;  // momentum flip, sell setup
        m_c.c = 1;   // new sell setup
        m_highLow = low[b];
    }
    else if ((close[b] < close[b-4]) && (m_prevC.d > 0)) {
        // buy setup continuation
        m_c.d = 1;

        if (m_prevC.c < 9) {
            m_c.c = m_prevC.c + 1;
            m_highLow = o3d::max(high[b], m_highLow);
        } else {
            // buy-setup begin in buy-setup
            m_c.c = 1;
        }
    }
    else if ((close[b] > close[b-4]) and (m_prevC.d < 0)) {
        // sell setup continuation
        m_c.d = -1;

        if (m_prevC.c < 9) {
            m_c.c = m_prevC.c + 1;
            m_highLow = o3d::min(low[b], m_highLow);
        } else {
            // sell-setup begin in sell-setup
            m_c.c = 1;
        }
    }

    // 8 or 9 perfect
    if (m_c.c >= 8) {
        if (m_c.d > 0) {
            // lower low at count 8 over 6 and 9 over 7
            if (low[b] < low[b-2]) {
                m_c.p = true;
            } else {
                m_c.p = false;  // can be loose
            }
        } else if (m_c.d < 0) {
            // higher high at count 8 over 6 and 9 over 7
            if (high[b] > high[b-2]) {
                m_c.p = true;
            } else {
                m_c.p = false;  // can be loose
            }
        }
    }

    // combo (buy-setup in buy-setup / sell-setup in sell-setup)
    // @todo

    //
    // count-down
    //

    // retain the price if the 8 count-down
    if (m_cd.c == 8) {
        m_cd.eight = close[b];
    }

    // similar for aggressive count-down
    if (m_aggCd.c == 8) {
        m_aggCd.eight = close[b];
    }

    //
    // buy-setup countdown
    //

    if (m_prevC.d > 0) {
        // classical countdown, start on a nine
        if (m_c.c == 9 && m_prevCd.c == 0) {
            // start
            m_cd.c = 1;
            m_cd.d = 1;
        }

        // aggressive countdown
        if (m_c.c == 9 && m_prevCd.c == 0) {
            // start
            m_aggCd.c = 1;
            m_aggCd.d = 1;
        }
    }

    // buy-setup countdown
    if (m_cd.d > 0) {
        if ((close[b] < low[b-2]) && (m_prevCd.c >= 1) && (m_prevCd.c < 13)) {
            // continue
            m_cd.c = m_prevCd.c + 1;
            m_cd.d = 1;

            if ((m_cd.c == 13) && (low[b] <= m_cd.eight)) {
                m_cd.q = true;
            }
        }
    }

    // buy-setup countdown aggressive
    if (m_aggCd.d == 1) {
        if ((low[b] < low[b-2]) && (m_prevAggCd.c >= 1) && (m_prevAggCd.c < 13)) {
            // continue
            m_aggCd.c = m_prevAggCd.c + 1;
            m_aggCd.d = -1;

            // qualifier
            if ((m_aggCd.c == 13) && (low[b] <= m_aggCd.eight)) {
                m_aggCd.q = true;
            }
        }
    }

    //
    // sell-setup countdown
    //

    if (m_prevC.d < 0) {
        // classical countdown, start on a nine
        if (m_c.c == 9 && m_prevCd.c == 0) {
            // start
            m_cd.c = 1;
            m_cd.d = -1;

            // aggressive countdown
            if (m_c.c == 9 && m_prevCd.c == 0) {
                // start
                m_aggCd.c = 1;
                m_aggCd.d = -1;
            }
        }
    }

    // sell-setup countdown
    if (m_cd.d < 0) {
        if ((close[b] >= high[b-2]) && (m_prevCd.c >= 1) &&(m_prevCd.c < 13)) {
            // continue
            m_cd.c = m_prevCd.c + 1;
            m_cd.d = -1;

            // qualifier
            if ((m_cd.c == 13) && (high[b] >= m_cd.eight)) {
                m_cd.q = true;
            }
        }
    }

    // sell-setup countdown aggressive
    if (m_aggCd.d < 0) {
        if ((high[b] >= high[b-2]) && (m_prevAggCd.c >= 1) && (m_prevAggCd.c < 13)) {
            // continue
            m_aggCd.c = m_prevAggCd.c + 1;
            m_aggCd.d = 1;

            // qualifier
            if ((m_aggCd.c == 13) && (high[b] >= m_aggCd.eight)) {
                m_aggCd.q = true;
            }
        }
    }

    //
    // TDST is the lowest low of a buy setup, or the highest high of a sell setup
    //

    if (m_c.c == 9) {
        // set TDST if a setup accomplished
        m_c.tdst = m_highLow;

        if (m_cd.d > 0 && m_c.d != m_cd.d) {
            // count-down cancelation when buy-setup appears during a sell count-down or a sell-setup appear during a buy count-down.
            m_cd.reset();
        }

        if (m_aggCd.d > 0 && m_c.d != m_aggCd.d) {
            // same for aggressive count-down
            m_aggCd.reset();
        }
    } else {
        // copy from previous
        m_c.tdst = m_prevC.tdst;
    }

    // count-down cancelation on TDST
    if (((m_prevC.d > 0) && (close[b] > m_c.tdst)) || ((m_prevC.d < 0) && (close[b] < m_c.tdst))) {
        m_cd.reset();
        m_aggCd.reset();

        // TDST canceled too
        m_c.tdst = 0.0;
    }
}
