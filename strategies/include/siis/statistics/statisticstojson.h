/**
 * @brief SiiS statistics models to JSON value.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#ifndef SIIS_STATISTICSTOJSON_H
#define SIIS_STATISTICSTOJSON_H

#include "statistics.h"
#include <json/value.h>

namespace siis {

class SIIS_API StatisticsToJson
{
public:

    static void dumpsSampler(const Sampler &sampler, Json::Value &output);
    static void dumpsPercentSampler(const PercentSampler &sampler, Json::Value &output);
    static void dumpsCurrencyStatToken(const CurrencyStatToken &token, Json::Value &output);
    static void dumpsPercentStatToken(const PercentStatToken &token, Json::Value &output);
    static void dumpsGlobalStatistics(const GlobalStatistics &globalStats, Json::Value &output);
};

} // namespace siis

#endif // SIIS_STATISTICSTOJSON_H
