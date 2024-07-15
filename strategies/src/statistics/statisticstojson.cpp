/**
 * @brief SiiS statistics models to JSON value.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#include "siis/statistics/statisticstojson.h"

using namespace siis;


void dumpsSampler(const Sampler &sampler, Json::Value &output)
{
//    to_dict[self.name] = {
//        'min': self.fmt_value(self.min_value),
//        'max': self.fmt_value(self.max_value),
//        'cum': self.fmt_value(self.cumulated),
//        'avg': self.fmt_value(self.average()),
//        'std-dev': self.fmt_value(self.std_dev())
//    }
}

void dumpsPercentSampler(const PercentSampler &sampler, Json::Value &output)
{

}

void dumpsCurrencyStatToken(const CurrencyStatToken &token, Json::Value &output)
{

}

void dumpsPercentStatToken(const PercentStatToken &token, Json::Value &output)
{

}

void dumpsGlobalStatistics(const GlobalStatistics &globalStats, Json::Value &output)
{

}
