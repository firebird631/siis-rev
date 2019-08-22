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
 */
class SIIS_API Price : public Indicator
{
public:

    // TYPE_TREND
    // CLS_INDEX

    enum Method {
        PRICE_CLOSE = 0,  //!< Close only
        PRICE_HLC = 1,    //!< (H+L+C) / 3
        PRICE_OHLC = 2    //!< (O+H+L+C) / 4
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

    o3d::Double min() const { return m_min; }
    o3d::Double max() const { return m_max; }

    // void compute(o3d::Double timestamp, const OhlcArray &ohlc, o3d::Int32 ofs = 0);
    void compute(const OhlcCircular &ohlc);

private:

    Method m_method;

    DataArray m_open;
    DataArray m_high;
    DataArray m_low;
    DataArray m_close;

    DataArray m_price;
    DataArray m_timestamp;

    o3d::Bool m_consolidated;

    o3d::Double m_min;
    o3d::Double m_max;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_PRICE_H
