/**
 * @brief SiiS common utils.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#include "siis/utils/common.h"
#include "siis/utils/math.h"
#include "siis/market.h"
#include "siis/order.h"

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
    if (tf == TF_TICK) {
        return "t";
    } else if (tf > TF_TICK && tf < TF_MIN) {
        return o3d::String::print("%is", static_cast<o3d::Int32>(tf));
    } else if (tf >= TF_MIN && tf < TF_HOUR) {
        return o3d::String::print("%im", static_cast<o3d::Int32>(tf / TF_MIN));
    } else if (tf >= TF_HOUR && tf < TF_DAY) {
        return o3d::String::print("%ih", static_cast<o3d::Int32>(tf / TF_HOUR));
    } else if (tf >= TF_DAY && tf < TF_WEEK) {
        return o3d::String::print("%id", static_cast<o3d::Int32>(tf / TF_DAY));
    } else if (tf >= TF_WEEK && tf < TF_MONTH) {
        return o3d::String::print("%iw", static_cast<o3d::Int32>(tf / TF_WEEK));
    } else if (tf >= TF_MONTH && tf < TF_YEAR) {
        return o3d::String::print("%iM", static_cast<o3d::Int32>(tf / TF_MONTH));
    } else if (tf >= TF_YEAR) {
        return o3d::String::print("%iy", static_cast<o3d::Int32>(tf / TF_YEAR));
    }

    return "";
}

o3d::String siis::marketContractToStr(o3d::Int32 marketContract)
{
    switch (marketContract) {
        case Market::CONTRACT_SPOT: return "spot";
        case Market::CONTRACT_CFD: return "cfd";
        case Market::CONTRACT_FUTURE: return "future";
        case Market::CONTRACT_OPTION: return "option";
        case Market::CONTRACT_WARRANT: return "warrant";
        case Market::CONTRACT_TURBO: return "turbo";
    }

    return "undefined";
}

o3d::String siis::marketUnitToStr(o3d::Int32 marketUnit)
{
    switch (marketUnit) {
        case Market::UNIT_AMOUNT: return "amount";
        case Market::UNIT_CONTRACTS: return "contracts";
        case Market::UNIT_SHARES: return "shares";
        case Market::UNIT_INVERSE: return "inverse";
    }

    return "undefined";
}

o3d::String siis::marketTypeToStr(o3d::Int32 marketType)
{
    switch (marketType) {
        case Market::TYPE_STOCK: return "stock";
        case Market::TYPE_CRYPTO: return "crypto";
        case Market::TYPE_INDICE: return "indice";
        case Market::TYPE_CURRENCY: return "currency";
        case Market::TYPE_COMMODITY: return "commodity";
    }

    return "undefined";
}

//! Convert date to number of days
//! @warning Dates before Oct. 1582 are inaccurate.
static o3d::Int64 gday(const o3d::DateTime &d)
{
    o3d::Int64 y, m;

    m = (d.month + 9) % 12;
    y = (d.year - m / 10);

    return y*365 + y/4 - y/100 + y/400 + (m*306 + 5)/10 + (d.mday - 1);
}

//! Convert number of day to legal datetime
//! @warning Dates before Oct. 1582 are inaccurate.
static o3d::DateTime dft(o3d::Int64 d)
{
    o3d::Int64 y, ddd, mi;

    y = (10000*d + 14780)/3652425;
    ddd = d - (y*365 + y/4 - y/100 + y/400);
    if (ddd < 0) {
        --y;
        ddd = d - (y*365 + y/4 - y/100 + y/400);
    }
    mi = (52 + 100*ddd)/3060;

    o3d::DateTime dt;
    dt.year = y + (mi + 2)/12;
    dt.month = (mi + 2)%12 + 1;
    dt.mday = ddd - (mi*306 + 5)/10 + 1;
    return dt;
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

        dt.fromTime(timestamp, true); // true utc
        dt.hour = 0;
        dt.minute = 0;
        dt.second = 0;
        dt.microsecond = 0;

        // first day of the week (1 based on monday ISO8601)
        // dt.mday = o3d::max<o3d::Int8>(1, dt.mday - (dt.getIsoDayOfWeek()-1));
        // dt.wday = o3d::DAY_MONDAY;
        dt = dft(gday(dt) - dt.getDayOfWeek());

        return dt.toTimestamp(true);
    }
    else if (timeframe == TF_MONTH) {
        // replace by first day of month at 00h00 UTC
        o3d::DateTime dt;

        dt.fromTime(timestamp, true);
        dt.mday = 1;
        dt.wday = dt.getIsoDayOfWeek();  // adjust day of week
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

o3d::String siis::formatPrice(o3d::Double price, o3d::Int32 precision, o3d::Double step)
{
    o3d::Double adjustedPrice = truncate<o3d::Double>(::round(price / step) * step, precision);
    o3d::String formattedPrice;
    formattedPrice.concat(adjustedPrice, precision);

    // remove trailing 0s and dot
    if (formattedPrice.find('.') > 0) {
        formattedPrice.trimRight('0', true);
        formattedPrice.trimRight('.');
    }

    return formattedPrice;
}

o3d::String siis::orderTypeToStr(o3d::Int32 orderType)
{
    switch (orderType) {
        case Order::ORDER_UNDEFINED: return "undefined";
        case Order::ORDER_MARKET: return "market";
        case Order::ORDER_LIMIT: return "limit";
        case Order::ORDER_STOP: return "stop";
        case Order::ORDER_STOP_LIMIT: return "stop-limit";
        case Order::ORDER_TAKE_PROFIT: return "take-profit";
        case Order::ORDER_TAKE_PROFIT_LIMIT: return "take-profit-limit";
    }

    return "undefined";
}

o3d::String siis::orderReturnCodeToStr(o3d::Int32 returnCode)
{
    switch (returnCode) {
        case Order::RET_OK: return "success";
        case Order::RET_UNDEFINED: return "undefined";
        case Order::RET_INSUFFICIENT_FUNDS: return "insufficient-funds";
        case Order::RET_INSUFFICIENT_MARGIN: return "insufficient-margin";
        case Order::RET_ERROR: return "error";
        case Order::RET_INVALID_ARGS: return "invalid-arguments";
        case Order::RET_DENIED: return "denied";
        case Order::RET_UNREACHABLE_SERVICE: return "unreachable";
        case Order::RET_RATE_LIMIT: return "rate-limit";
        case Order::RET_ORDER_LIMIT: return "order-limit";
        case Order::RET_POSITION_LIMIT: return "position-limit";
        case Order::RET_INVALID_NONCE: return "invalid-nonce";
        case Order::RET_CANCEL_ONLY: return "cancel-only";
        case Order::RET_POST_ONLY: return "post-only";
    }

    return "undefined";
}

o3d::String siis::timestampToStr(o3d::Double ts)
{
    o3d::DateTime dt;
    dt.fromTime(ts, true);

    return dt.buildString("%Y-%m-%d %H:%M:%S");
}

o3d::Double siis::durationFromStr(const o3d::String &deltaTime)
{
    if (deltaTime.count(':') != 1) {
        // @todo format error
        return 0.0;
    }

    o3d::Int32 sign = 1;
    o3d::Int32 baseOfs = 0;

    if (deltaTime.startsWith("-")) {
        sign = -1;
        baseOfs += 1;
    } else if (deltaTime.startsWith("+")) {
        baseOfs += 1;
    }

    o3d::Int32 separator = deltaTime.find(':');
    o3d::Int32 hours = deltaTime.sub(baseOfs, separator).toInt32();
    o3d::Int32 minutes = deltaTime.sub(separator+1).toInt32();

    if ((hours < 0) || (hours > 23)) {
        // @todo format error
        return 0.0;
    }

    if ((minutes < 0) || (minutes > 59)) {
        // @todo format error
        return 0.0;
    }

    return sign * (hours * 3600.0 + minutes * 60.0);
}
