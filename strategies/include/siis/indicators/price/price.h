/**
 * @brief SiiS strategy price indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#ifndef SIIS_PRICE_H
#define SIIS_PRICE_H

#include "../indicator.h"
#include "../../ohlc.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS strategy price indicator.
 * @author Frederic Scherma
 * @date 2019-03-15
 * @todo minimalist compute version (only update the current bar)
 */
class SIIS_API Price : public Indicator
{
public:

    // TYPE_TREND
    // CLS_INDEX

    enum Method {
        PRICE_CLOSE = 0,  //!< Close only
        PRICE_HLC = 1,    //!< (H+L+C) / 3
        PRICE_OHLC = 2,   //!< (O+H+L+C) / 4
        PRICE_HL = 3      //!< (H+L) / 2
    };

    Price(const o3d::String &name, o3d::Double timeframe, Method method=PRICE_CLOSE);
    Price(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    const DataArray& price() const { return m_price; }

    /**
     * @brief consolidated Is the last computed ohlc consolidated/ended.
     */
    o3d::Bool consolidated() const { return m_consolidated; }
    o3d::Bool ended() const { return m_consolidated; }

    const DataArray& open() const { return m_open; }
    const DataArray& high() const { return m_high; }
    const DataArray& low() const { return m_low; }
    const DataArray& close() const { return m_close; }

    const DataArray& timestamp() const { return m_timestamp; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    // void compute(o3d::Double timestamp, const OhlcArray &ohlc, o3d::Int32 ofs = 0);
    void compute(const OhlcCircular &ohlc);

    /**
     * @brief computeMinimalist Similar as compute but only update the last bar if num bars is 1
     * @param ohlc
     * If num bars is greater than 1 it then call the default compute method.
     * @note This is an optimized compute version without trade-off.
     */
    void computeMinimalist(const OhlcCircular &ohlc, const Ohlc *current, o3d::Int32 numBars);

    /**
     * @brief cross Cross the previous and last updated price with a given price
     * The prices used are not the prices of the 2 lasts OHLCs but 2 last from the 2 last update.
     * @warning Do not confuse with a cross with bar-2 and bar-1 close.
     */
    inline o3d::Int32 cross(o3d::Double price) const {
        if (m_prev > price && m_last < price) {
            return -1;
        }

        if (m_prev < price && m_last > price) {
            return 1;
        }

        return 0;
    }

private:

    Method m_method;

    DataArray m_open;
    DataArray m_high;
    DataArray m_low;
    DataArray m_close;

    DataArray m_price;
    DataArray m_timestamp;

    o3d::Bool m_consolidated;
    o3d::Double m_lastClosedTimestamp;

    o3d::Double m_prev;
    o3d::Double m_last;

    void computeLast(const Ohlc *current);
};

} // namespace siis

#endif // SIIS_PRICE_H
