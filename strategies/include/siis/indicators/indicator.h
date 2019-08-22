/**
 * @brief SiiS strategy indicator base model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#ifndef SIIS_INDICATOR_H
#define SIIS_INDICATOR_H

#include "../base.h"
#include "../config/strategyconfig.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief SiiS strategy indicator base model.
 * @author Frederic Scherma
 * @date 2019-03-15
 * normalizeClose = {'EMA', 'DEMA', 'MIDPOINT', 'MIDPRICE', 'SAREXT', 'LINEARREG_INTERCEPT', 'SMA', 'BBANDS', 'TRIMA',
 *   'TEMA', 'KAMA', 'PLUS_DM', 'MINUS_DM', 'T3', 'SAR', 'VAR', 'MA', 'WMA', 'LINEARREG', 'MAMA', 'TSF', 'HT_TRENDLINE', 'STDDEV'}
 * normalize360 = {'HT_DCPHASE', 'HT_PHASOR', 'HT_DCPERIOD'}
 * normalize100 = {'CMO', 'STOCHF', 'MINUS_DI', 'CCI', 'DX', 'TRANGE', 'ROCR100', 'MFI', 'PLUS_DI', 'AROON',
 *   'LINEARREG_ANGLE', 'WILLR', 'ULTOSC', 'MOM', 'ADX', 'LINEARREG_SLOPE', 'MACD', 'MACDEXT', 'STOCH', 'MACDFIX',
 *   'AROONOSC', 'RSI', 'ADXR', 'APO', 'ATR', 'STOCHRSI', 'ADOSC'}
 */
class SIIS_API Indicator
{
public:

    enum Type {
        TYPE_UNDEFINED = 0,
        TYPE_AVERAGE_PRICE = 1,
        TYPE_MOMENTUM = 2,
        TYPE_VOLATILITY = 4,
        TYPE_SUPPORT_RESISTANCE = 8,
        TYPE_TREND = 16,
        TYPE_VOLUME = 32,
        TYPE_MOMENTUM_VOLUME = 2|32,
        TYPE_MOMENTUM_SUPPORT_RESISTANCE_TREND = 2|8|16
    };

    enum Class {
        CLS_UNDEFINED = 0,
        CLS_CUMULATIVE = 1,
        CLS_INDEX = 2,
        CLS_OSCILLATOR = 3,
        CLS_OVERLAY = 4
    };

    enum Norm {
        NORM_CLOSE = 0,
        NORM_360 = 1,
        NORM_100 = 2
    };

    Indicator(const o3d::String &name, o3d::Double timeframe) :
        m_name(name),
        m_timeframe(timeframe),
        m_lastTimestamp(0)
    {
    }

    const o3d::String& name() const { return m_name; }
    o3d::Double timeframe() const { return m_timeframe; }
    o3d::Double lastTimestamp() const { return m_lastTimestamp; }

    // o3d::Int32 lookback() const;
    // void trace(StrategyLogger &logger) const;

protected:

    inline void done(o3d::Double timestamp) { m_lastTimestamp = timestamp; }

private:

    o3d::String m_name;
    o3d::Double m_timeframe;
    o3d::Double m_lastTimestamp;
};

} // namespace siis

#endif // SIIS_INDICATOR_H
