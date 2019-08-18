/**
 * @brief SiiS moving average convergence divergence indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_MACD_H
#define SIIS_MACD_H

#include "../indicator.h"
#include "../../dataarray.h"

namespace siis {

/**
 * @brief SiiS moving average convergence divergence indicator.
 * @author Frederic Scherma
 * @date 2019-03-19
 * @ref https://fr.wikipedia.org/wiki/MACD
 */
class SIIS_API Macd : public Indicator
{
public:

    // TYPE_TREND
    // CLS_OVERLAY

    Macd(const o3d::String &name, o3d::Double timeframe, o3d::Int32 fastLen=26, o3d::Int32 slowLen=12, o3d::Int32 signalLen=9);
    Macd(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    void setConf(IndicatorConfig conf);

    o3d::Int32 fastLen() const { return m_fastLen; }
    o3d::Int32 slowLen() const { return m_slowLen; }
    o3d::Int32 signalLen() const { return m_signalLen; }

    const DataArray& macd() const { return m_macd; }
    const DataArray& signal() const { return m_signal; }
    const DataArray& hist() const { return m_hist; }

    o3d::Double last() const { return m_last; }
    o3d::Double prev() const { return m_prev; }

    /**
     * @brief compute Compute a SMA.
     * @param price A close or any other array of price.
     */
    void compute(o3d::Double timestamp, const DataArray &price);

    /**
     * @brief lookback Min number of necessary samples.
     */
    o3d::Int32 lookback() const;

private:

    o3d::Int32 m_fastLen;
    o3d::Int32 m_slowLen;
    o3d::Int32 m_signalLen;

    DataArray m_macd;
    DataArray m_signal;
    DataArray m_hist;

    o3d::Double m_prev;
    o3d::Double m_last;
};

} // namespace siis

#endif // SIIS_MACD_H
