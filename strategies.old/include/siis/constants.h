/**
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

constexpr o3d::Int32 UNDEFINED = 0;
constexpr o3d::Int32 LONG = 1;
constexpr o3d::Int32 SHORT = -1;

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

} // namespace siis

#endif // SIIS_CONSTS_H
