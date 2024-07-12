/**
 * @brief SiiS strategy candle generator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#ifndef SIIS_TIMEFRAMEOHLCGEN_H
#define SIIS_TIMEFRAMEOHLCGEN_H

#include "../ohlc.h"
#include "../tick.h"
#include "../utils/common.h"

#include <o3d/core/base.h>

namespace siis {

/**
 * @brief Generator OHLC for a specific fixed timeframe.
 * @author Frederic Scherma
 * @date 2019-03-07
 * The current, non consolidated OHLC is kept as the last ohlc of the out array.
 * It can generate from a tick series or from a lower timeframe (multiple of).
 */
class SIIS_API TimeframeOhlcGen
{
public:

    TimeframeOhlcGen(o3d::Double fromTimeframe, o3d::Double toTimeframe, Ohlc::Type ohlcType=Ohlc::TYPE_MID);
    ~TimeframeOhlcGen();

    o3d::Double fromTimeframe() const { return m_fromTf; }
    o3d::Double toTimeframe() const { return m_toTf; }

    o3d::Double lastTimestamp() const { return m_lastTimestamp; }
    o3d::UInt32 numLastConsumed() const { return m_numLastConsumed; }

    /**
     * @brief genFromTicks Generate as many higher ohlc as possible from the array of ticks given in parameters.
     * @param ticks Input array of tick (1 or more).
     * @param out Circular array of ohlc to complete (new ohlc array pushed back).
     * @return Number of newly generated ohlc.
     */
    o3d::UInt32 genFromTicks(const TickArray &ticks, OhlcCircular &out);

    /**
     * @brief genFromOhlc Generate as many higher ohlc as possible from the array of ohlc given in parameters.
     * @param ohlc Input array of ohlc (1 or more).
     * @param out Circular array of ohlc to complete (new ohlc array pushed back).
     * @return Number of newly generated ohlc.
     */
    o3d::UInt32 genFromOhlc(const OhlcArray &ohlc, OhlcCircular &out);

    o3d::Double baseTime(o3d::Double timestamp) const { return siis::baseTime(timestamp, m_toTf); }

    o3d::Bool valid() const;

    /**
     * @brief updateFromTick Update from one more tick and last ohlc from out.
     * @return True if a new ohlc is append to out.
     */
    o3d::Bool updateFromTickMid(const Tick *tick, OhlcCircular &out);

    /**
     * @brief updateFromOhlc Update from one more sub timeframe ohlc and last ohlc from out.
     * @return True if a new ohlc is append to out.
     */
    o3d::Bool updateFromOhlcMid(const Ohlc *ohlc, OhlcCircular &out);

    o3d::Bool updateFromTickLast(const Tick *tick, OhlcCircular &out);
    o3d::Bool updateFromTickBid(const Tick *tick, OhlcCircular &out);
    o3d::Bool updateFromTickAsk(const Tick *tick, OhlcCircular &out);

    o3d::Bool updateFromOhlcLast(const Ohlc *ohlc, OhlcCircular &out);
    o3d::Bool updateFromOhlcBid(const Ohlc *ohlc, OhlcCircular &out);
    o3d::Bool updateFromOhlcAsk(const Ohlc *ohlc, OhlcCircular &out);

private:

    o3d::Double m_fromTf;
    o3d::Double m_toTf;
    Ohlc::Type m_ohlcType;

    o3d::Double m_lastTimestamp;
    o3d::UInt32 m_numLastConsumed;

    Ohlc *m_curOhlc;
};

} // namespace siis

#endif // SIIS_TIMEFRAMEOHLCGEN_H
