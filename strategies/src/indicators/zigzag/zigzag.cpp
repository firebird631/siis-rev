/**
* @brief SiiS ZigZag indicator
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-08-17
 */

#include "siis/indicators/zigzag/zigzag.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;

ZigZag::ZigZag(const o3d::String &name, o3d::Double timeframe, o3d::Double threshold) :
    Indicator(name, timeframe),
    m_threshold(threshold)
{
}

ZigZag::ZigZag(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_threshold(0.0)
{
    m_threshold = conf.data().get("threshold", 0.05).asDouble();
}

void ZigZag::setConf(IndicatorConfig conf)
{
    m_threshold = conf.data().get("threshold", 0.05).asDouble();
}

void ZigZag::compute(o3d::Double timestamp, const DataArray &open, const DataArray &high, const DataArray &low, const DataArray &close)
{
    // @todo

    done(timestamp);
}
