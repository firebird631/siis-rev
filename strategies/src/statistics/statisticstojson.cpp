/**
 * @brief SiiS statistics models to JSON value.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#include "siis/statistics/statisticstojson.h"
#include "siis/utils/math.h"

using namespace siis;

void StatisticsToJson::dumpsSampler(const Sampler &sampler, Json::Value &output)
{
    //'min': self.fmt_value(self.min_value),
    //'max': self.fmt_value(self.max_value),
    //'cum': self.fmt_value(self.cumulated),
    //'avg': self.fmt_value(self.average()),
    //'std-dev': self.fmt_value(self.std_dev())
    output["min"] = sampler.minValue;
    output["max"] = sampler.maxValue;
    output["cum"] = sampler.cumulated;
    output["avg"] = sampler.avg;
    output["std-dev"] = sampler.stdDev;
}

void StatisticsToJson::dumpsPercentSampler(const PercentSampler &sampler, Json::Value &output)
{
    output["min"] = o3d::String::print("%.2f%%", sampler.minValue * 100).toAscii().getData();
    output["max"] = o3d::String::print("%.2f%%", sampler.maxValue * 100).toAscii().getData();
    output["cum"] = o3d::String::print("%.2f%%", sampler.cumulated * 100).toAscii().getData();
    output["avg"] = o3d::String::print("%.2f%%", sampler.avg * 100).toAscii().getData();
    output["std-dev"] = o3d::String::print("%.2f%%", sampler.stdDev * 100).toAscii().getData();
}

void StatisticsToJson::dumpsCurrencyStatToken(const CurrencyStatToken &token, Json::Value &output)
{
    output["max-time-to-recover"] = round(token.maxTimeToRecover, 2);
    output["estimate-profit-per-month"] = token.estimateProfitPerMonth;

    output["avg-trade"] = token.avgTrade;
    output["avg-winning-trade"] = token.avgWinningTrade;
    output["avg-loosing-trade"] = token.avgLoosingTrade;
    output["avg-win-loss-rate"] = round(token.avgWinLossRate, 2);

    output["sharpe-ratio"] = round(token.sharpeRatio, 2);
    output["sortino-ratio"] = round(token.sortinoRatio, 2);
    output["ulcer-index"] = round(token.ulcerIndex, 2);

    for (const Sampler &sampler : token.samplers) {
        Json::Value samplerOutput;
        dumpsSampler(sampler, samplerOutput);
        output[sampler.name.toAscii().getData()] = samplerOutput;
    }
}

void StatisticsToJson::dumpsPercentStatToken(const PercentStatToken &token, Json::Value &output)
{
    output["max-time-to-recover"] = round(token.maxTimeToRecover, 2);
    output["estimate-profit-per-month"] = o3d::String::print("%.2f%%", token.estimateProfitPerMonth * 100).toAscii().getData();

    output["avg-trade"] = o3d::String::print("%.2f%%", token.avgTrade * 100).toAscii().getData();
    output["avg-winning-trade"] = o3d::String::print("%.2f%%", token.avgWinningTrade * 100).toAscii().getData();
    output["avg-loosing-trade"] = o3d::String::print("%.2f%%", token.avgLoosingTrade * 100).toAscii().getData();
    output["avg-win-loss-rate"] = round(token.avgWinLossRate, 2);

    output["sharpe-ratio"] = round(token.sharpeRatio, 2);
    output["sortino-ratio"] = round(token.sortinoRatio, 2);
    output["ulcer-index"] = round(token.ulcerIndex, 2);

    for (const PercentSampler &sampler : token.samplers) {
        Json::Value samplerOutput;
        dumpsPercentSampler(sampler, samplerOutput);
        output[sampler.name.toAscii().getData()] = samplerOutput;
    }
}

void StatisticsToJson::dumpsGlobalStatistics(const GlobalStatistics &globalStats, Json::Value &output)
{
    output["longest-flat-period"] = round(globalStats.longestFlatPeriod, 2);
    output["avg-time-in-market"] = round(globalStats.avgTimeInMarket, 2);
    output["num-traded-days"] = globalStats.numTradedDays;
    output["avg-trade-per-day"] = round(globalStats.avgTradePerDay, 2);
    output["avg-trade-per-day-inc-we"] = round(globalStats.avgTradePerDayIncWe, 2);

    Json::Value percent;
    Json::Value currency;

    StatisticsToJson::dumpsPercentStatToken(globalStats.percent, percent);
    StatisticsToJson::dumpsCurrencyStatToken(globalStats.currency, currency);

    output["percent"] = percent;
    output["currency"] = currency;
}
