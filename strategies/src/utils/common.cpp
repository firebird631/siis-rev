/**
 * @brief SiiS common utils.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#include "siis/utils/common.h"
#include "siis/market.h"

#include <o3d/core/datetime.h>
#include <ta-lib/ta_defs.h>

using namespace siis;

o3d::Double siis::timeframeFromStr(const o3d::String &tf)
{
    o3d::WChar c = tf.length() ? tf[tf.length()-1] : L'\0';

    switch(c) {
        case '0':
        case 't':
            return 0.0;
        case 's':
            return static_cast<o3d::Double>(tf.toInt32());
        case 'm':
            return static_cast<o3d::Double>(tf.toInt32()) * 60.0;
        case 'h':
            return static_cast<o3d::Double>(tf.toInt32()) * 60.0 * 60.0;
        case 'd':
            return static_cast<o3d::Double>(tf.toInt32()) * 60.0 * 60.0 * 24.0;
        case 'w':
            return static_cast<o3d::Double>(tf.toInt32()) * 60.0 * 60.0 * 24.0 * 7.0;
        case 'M':
            return static_cast<o3d::Double>(tf.toInt32()) * 60.0 * 60.0 * 24.0 * 30.0;
        default:
            // direct conversion from seconds if possible
            return static_cast<o3d::Double>(tf.toInt32());
    }
}

o3d::String siis::timeframeToStr(o3d::Double tf)
{
    switch (static_cast<o3d::Int32>(tf)) {
        case 0: return "t";
        case 1: return "1s";
        case 10: return "10s";
        case 60: return "1m";
        case 2*60: return "2m";
        case 3*60: return "3m";
        case 5*60: return "5m";
        case 10*60: return "10m";
        case 15*60: return "15m";
        case 60*60: return "1h";
        case 2*60*60: return "2h";
        case 3*60*60: return "3h";
        case 4*60*60: return "4h";
        case 6*60*60: return "6h";
        case 8*60*60: return "8h";
        case 24*60*60: return "1d";
        case 2*24*60*60: return "2d";
        case 3*24*60*60: return "3d";
        case 7*24*60*60: return "1w";
        case 30*24*60*60: return "1m";
    }

    return "";
}

o3d::String siis::marketContractToStr(o3d::Int32 marketContract)
{
    switch (marketContract) {
        case Market::CONTRACT_SPOT: return "spot";
        case Market::CONTRACT_CFD: return "cfd";
        case Market::CONTRACT_FUTUR: return "futur";
        case Market::CONTRACT_OPTION: return "option";
        case Market::CONTRACT_WARRANT: return "warrant";
        case Market::CONTRACT_TURBO: return "turbo";
    }

    return "undefined";
}

o3d::String siis::marketUnitToStr(o3d::Int32 marketUnit)
{
    switch (marketUnit) {
        case Market::UNIT_UNKNOWN: return "undefined";
        case Market::UNIT_LOT: return "lot";
        case Market::UNIT_CONTRACT: return "contract";
        case Market::UNIT_CURRENCY: return "currency";
    }

    return "undefined";
}

o3d::String siis::marketTypeToStr(o3d::Int32 marketType)
{
    switch (marketType) {
        case Market::TYPE_UNKNOWN: return "undefined";
        case Market::TYPE_STOCK: return "stock";
        case Market::TYPE_CRYPTO: return "crypto";
        case Market::TYPE_INDICE: return "indice";
        case Market::TYPE_CURRENCY: return "currency";
        case Market::TYPE_COMMODITY: return "commodity";
    }

    return "undefined";
}

o3d::Double siis::baseTime(o3d::Double timestamp, o3d::Double timeframe)
{
    if (timeframe < TF_WEEK) {
        // simplest
        return o3d::Int32(timestamp / timeframe) * timeframe;
    }
    else if (timeframe == TF_WEEK) {
        // must find the UTC first day of week
        o3d::DateTime dt;

        dt.fromTime(timestamp);
        dt.hour = 0;
        dt.minute = 0;
        dt.second = 0;
        dt.microsecond = 0;

        // first day of the week (1 based on monday ISO8601)
        dt.mday = o3d::max<o3d::Int8>(1, dt.mday - (dt.getDayOfWeek()-1));
        dt.wday = o3d::DAY_MONDAY;

        return dt.toTimestamp(true);
    }
    else if (timeframe == TF_MONTH) {
        // replace by first day of month at 00h00 UTC
        o3d::DateTime dt;

        dt.fromTime(timestamp, true);
        dt.mday = 1;
        dt.wday = dt.getDayOfWeek();  // adjust day of week
        dt.hour = 0;
        dt.minute = 0;
        dt.second = 0;
        dt.microsecond = 0;

        return dt.toTimestamp(true);
    }
    else {
        return o3d::Int32(timestamp / timeframe) * timeframe;
    }
}

o3d::String siis::directionToStr(o3d::Int32 dir)
{
    if (dir > 0) {
        return "long";
    } else if (dir < 0) {
        return "short";
    } else {
        return "";
    }
}

o3d::String siis::taErrorToStr(o3d::Int32 error)
{
    switch(error) {
        case TA_SUCCESS:
            return "TA_SUCCESS";
        case TA_LIB_NOT_INITIALIZE:
            return "TA_LIB_NOT_INITIALIZE";
        case TA_BAD_PARAM:
            return "TA_BAD_PARAM";
        case TA_ALLOC_ERR:
            return "TA_ALLOC_ERR";
        case TA_GROUP_NOT_FOUND:
            return "TA_GROUP_NOT_FOUND";
        case TA_FUNC_NOT_FOUND:
            return "TA_FUNC_NOT_FOUND";
        case TA_INVALID_HANDLE:
            return "TA_INVALID_HANDLE";
        case TA_INVALID_PARAM_HOLDER:
            return "TA_INVALID_PARAM_HOLDER";
        case TA_INVALID_PARAM_HOLDER_TYPE:
            return "TA_INVALID_PARAM_HOLDER_TYPE";
        case TA_INVALID_PARAM_FUNCTION:
            return "TA_INVALID_PARAM_FUNCTION";
        case TA_INPUT_NOT_ALL_INITIALIZE:
            return "TA_INPUT_NOT_ALL_INITIALIZE";
        case TA_OUT_OF_RANGE_START_INDEX:
            return "TA_OUT_OF_RANGE_START_INDEX";
        case TA_OUT_OF_RANGE_END_INDEX:
            return "TA_OUT_OF_RANGE_END_INDEX";
        case TA_INVALID_LIST_TYPE:
            return "TA_INVALID_LIST_TYPE";
        case TA_BAD_OBJECT:
            return "TA_BAD_OBJECT";
        case TA_NOT_SUPPORTED:
            return "TA_NOT_SUPPORTED";
        case TA_INTERNAL_ERROR:
            return "TA_INTERNAL_ERROR";
        case TA_UNKNOWN_ERR:
        default:
            return "TA_UNKNOWN_ERR";
    }
}

o3d::String siis::maTypeToStr(MAType maType)
{
    switch(maType) {
        case MA_SMA:
            return "SMA";
        case MA_EMA:
            return "EMA";
        case MA_WMA:
            return "WMA";
        case MA_DEMA:
            return "DEMA";
        case MA_TRIMA:
            return "TRIMA";
        case MA_KAMA:
            return "KAMA";
        case MA_MAMA:
            return "MAMA";
        case MA_T3:
            return "T3";
        default:
            return "SMA";
    }
}

MAType siis::maTypeFromStr(const o3d::String maType)
{
    if (maType == "SMA" || maType == "sma") {
        return MA_SMA;
    } else if (maType == "EMA" || maType == "ema") {
        return MA_EMA;
    } else if (maType == "WMA" || maType == "wma") {
        return MA_WMA;
    } else if (maType == "DEMA" || maType == "dema") {
        return MA_DEMA;
    } else if (maType == "TEMA" || maType == "tema") {
        return MA_TEMA;
    } else if (maType == "TRIMA" || maType == "trima") {
        return MA_TRIMA;
    } else if (maType == "KAMA" || maType == "kama") {
        return MA_KAMA;
    } else if (maType == "MAMA" || maType == "mama") {
        return MA_MAMA;
    } else if (maType == "T3" || maType == "t3") {
        return MA_T3;
    } else {
        return MA_SMA;
    }
}
