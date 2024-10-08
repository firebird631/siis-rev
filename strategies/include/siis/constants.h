﻿/**
 * @brief SiiS strategy global constants.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#ifndef SIIS_CONSTS_H
#define SIIS_CONSTS_H

#include "base.h"
#include <o3d/core/types.h>

namespace siis {

constexpr o3d::Double TF_TICK = 0.0;
constexpr o3d::Double TF_SEC = 1.0;
constexpr o3d::Double TF_MIN = 60.0;
constexpr o3d::Double TF_2MIN = 60*2.0;
constexpr o3d::Double TF_3MIN = 60*3.0;
constexpr o3d::Double TF_5MIN = 60*5.0;
constexpr o3d::Double TF_10MIN = 60*10.0;
constexpr o3d::Double TF_15MIN = 60*15.0;
constexpr o3d::Double TF_30MIN = 60*30.0;
constexpr o3d::Double TF_HOUR = 60*60.0;
constexpr o3d::Double TF_2HOUR = 60*60*2.0;
constexpr o3d::Double TF_3HOUR = 60*60*3.0;
constexpr o3d::Double TF_4HOUR = 60*60*4.0;
constexpr o3d::Double TF_6HOUR = 60*60*6.0;
constexpr o3d::Double TF_8HOUR = 60*60*8.0;
constexpr o3d::Double TF_12HOUR = 60*60*12.0;
constexpr o3d::Double TF_DAY = 60*60*24.0;
constexpr o3d::Double TF_2DAY = 60*60*24*2.0;
constexpr o3d::Double TF_3DAY = 60*60*24*3.0;
constexpr o3d::Double TF_WEEK = 60*60*24*7.0;
constexpr o3d::Double TF_MONTH = 60*60*24*30.0;
constexpr o3d::Double TF_QUARTER = 60*60*24*90.0;
constexpr o3d::Double TF_YEAR = 60*60*24*365.0;

constexpr o3d::Int32 UNDEFINED = 0;
constexpr o3d::Int32 LONG = 1;
constexpr o3d::Int32 SHORT = -1;

/**
 * @brief The differents moving average type, used in the configuration of some indicators.
 */
enum MAType {
    MA_SMA = 0,  //!< Simple Moving Average
    MA_EMA,      //!< Exponential Average
    MA_WMA,      //!< Weighted Moving Average
    MA_DEMA,     //!<Double Exponential Moving Average
    MA_TEMA,     //!< Triple Exponential Moving Average
    MA_TRIMA,    //!< Triangular Moving Average
    MA_KAMA,     //!< Kaufman Adaptive Moving Average
    MA_MAMA,     //!< MESA Adaptive Moving Average
    MA_T3        //!< Triple Exponential Moving Average
};

/**
 * @brief Price type for trade policy like entry, breakeven, dynamic-stop-loss and such mores.
 */
enum PriceType {
    PRICE_NONE = 0,
    PRICE_CUSTOM = 1,       //!< price is custom by design
    PRICE_FIXED = 2,        //!< price is in a fixed distance from the entry (pips are is converted in percentil)
    PRICE_LAST = 3,         //!< entry at market price
    PRICE_BEST1 = 4,        //!< entry on limit (maker if possible) using the first ticker best price
    PRICE_BEST2 = 5         //!< entry on limit (maker if possible) using the first ticker best price plus a spread
};

/**
 * @brief Distance type for trade policy like breakeven, dynamic-stop-loss and such mores.
 */
enum DistanceType {
    DIST_NONE = 0,
    DIST_PERCENTIL = 1,    //!< distance is in percentil from the entry (pips are is converted in percentil)
    DIST_PRICE = 2         //!< distance is in a fixed price delta
};

/**
 * @brief Adjust policy for trades exits like breakeven, dynamic-stop-loss and such mores.
 */
enum AdjustPolicy {
    ADJ_NONE = 0,
    ADJ_CUSTOM = 1,   //!< adjust is a custom by design
    ADJ_PRICE = 2,    //!< adjust at each price change
    ADJ_CLOSE = 3     //!< adjust after each timeframe close
};

} // namespace siis

#endif // SIIS_CONSTS_H
