/**
 * @brief SiiS strategy reversal-bar generator.
 * @copyright Copyright (C) 2014 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-12
 */

#ifndef SIIS_REVERSALOHLCGEN_H
#define SIIS_REVERSALOHLCGEN_H

#include "../ohlc.h"
#include "../tick.h"
#include "../utils/common.h"

#include <o3d/core/base.h>

namespace siis {

class Analyser;

/**
 * @brief Generator OHLC for a specific size, reversal and scale of reversal-bar
 * @author Frederic Scherma
 * @date 2024-07-12
 * The current, non consolidated OHLC is kept as the last ohlc of the out array.
 * It can generate from a tick series or from a lower timeframe (multiple of).
 */
class SIIS_API ReversalOhlcGen
{
public:

    ReversalOhlcGen(o3d::Int32 barSize, o3d::Int32 reversalSize, o3d::Double tickScale=1.0,
                    Ohlc::Type ohlcType=Ohlc::TYPE_MID);
    ~ReversalOhlcGen();

    o3d::Double lastTimestamp() const { return m_lastTimestamp; }
    o3d::UInt32 numLastConsumed() const { return m_numLastConsumed; }

    o3d::Int32 barSize() const { return m_barSize; }
    o3d::Int32 reversalSize() const { return m_reversalSize; }
    o3d::Double tickScale() const { return m_tickScale; }

    /**
     * @brief init Initialize from instrument price limit.
     * @param pricePrecision 1 or greater decimal place else default to 8
     * @param tickSize Tick size (price limit step) positive value or default to 0.00000001
     */
    void init(o3d::Int32 pricePrecision, o3d::Double tickSize);

    /**
     * @brief genFromTicks Generate as many higher ohlc as possible from the array of ticks given in parameters.
     * @param ticks Input array of tick (1 or more).
     * @param out Circular array of ohlc to complete (new ohlc array pushed back).
     * @return Number of newly generated ohlc.
     */
    o3d::UInt32 genFromTicks(const TickArray &ticks, OhlcCircular &out);

    /**
     * Similar as genFromTicks but take an extra analyser parameters to call updateTick.
     */
    o3d::UInt32 genFromTicks(const TickArray &ticks, OhlcCircular &out, Analyser &analyser);

    o3d::Bool valid() const;

    /**
     * @brief current Return the current working bar. Sometime it could be a nullptr.
     */
    const Ohlc* current() const { return m_curOhlc; }

    /**
     * @brief updateFromTick Update from one more tick and last ohlc from out.
     * @return True if a new ohlc is append to out.
     */
    o3d::Bool updateFromTickMid(const Tick *tick, OhlcCircular &out);

    o3d::Bool updateFromTickLast(const Tick *tick, OhlcCircular &out);
    o3d::Bool updateFromTickBid(const Tick *tick, OhlcCircular &out);
    o3d::Bool updateFromTickAsk(const Tick *tick, OhlcCircular &out);

private:

    o3d::Int32 m_barSize;
    o3d::Int32 m_reversalSize;
    o3d::Double m_tickScale;

    Ohlc::Type m_ohlcType;

    o3d::Double m_lastTimestamp;
    o3d::UInt32 m_numLastConsumed;

    o3d::Double m_tickSize;        //!< Instrument tick size
    o3d::Int32 m_pricePrecision;   //!< Instrument price precision

    Ohlc *m_curOhlc;

    o3d::Int32 m_reversing;

    /**
     * @brief adjustPrice Adjust the price according to the precision.
     */
    o3d::Double adjustPrice(o3d::Double price) const;
};

} // namespace siis

#endif // SIIS_REVERSALOHLCGEN_H
