/**
 * @brief SiiS statistics models and compatible for export.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-28
 */

#include "siis/statistics/statistics.h"
#include "siis/trade/trade.h"

using namespace siis;

Sampler::Sampler(const o3d::String &_name) :
    name(_name)
{
}

void Sampler::addSample(o3d::Double value)
{
    samples.push_back(value);
    cumulated += value;

    if (samples.size() > 0) {
        maxValue = o3d::max(maxValue, value);
        minValue = o3d::min(minValue, value);
    } else {
        minValue = maxValue = value;
    }
}

o3d::Int32 Sampler::count() const
{
    return static_cast<o3d::Int32>(samples.size());
}

o3d::Double Sampler::average() const
{
    o3d::Double avg = 0.0;

    if (samples.size() > 0) {
        for (size_t i = 0; i < samples.size(); ++i) {
            avg += samples[i];
        }

        avg /= samples.size();
    }

    return avg;
}

o3d::Double Sampler::stdDev() const
{
    o3d::Double stdDev = 0.0;

    // @todo

    return stdDev;
}

void BaseStatToken::reset()
{
    maxTimeToRecover = 0.0;
    estimateProfitPerMonth = 0.0;
    avgTrade = 0.0;
    avgWinningTrade = 0.0;
    avgLoosingTrade = 0.0;
    avgWinLossRate = 0.0;
    sharpeRatio = 1.0;
    sortinoRatio = 1.0;
    ulcerIndex = 0.0;
}

void CurrencyStatToken::reset()
{
    BaseStatToken::reset();
    samplers.clear();
}

const o3d::Char* PercentSampler::formatValue(o3d::Double value) const
{
    return o3d::String::print("%.2f%%", value * 100).toAscii().getData();
}

const o3d::Char *PercentStatToken::formatValue(o3d::Double value) const
{
    return o3d::String::print("%.2f%%", value * 100).toAscii().getData();
}

void PercentStatToken::reset()
{
    BaseStatToken::reset();
    samplers.clear();
}

void Statistics::addTrade(const Trade *trade)
{
    if (trade == nullptr) {
        return;
    }

    if (trade->exitPrice() <= 0.0) {
        return;
    }

    // keep important values and convert for convenience
    TradeResults results;

    results.entryPrice = trade->entryPrice();
    results.exitPrice = trade->exitPrice();

    results.entryTimestamp = trade->stats().firstRealizedEntryTimestamp;
    results.exitTimestamp = trade->stats().lastRealizedExitTimestamp;

    results.bestPrice = trade->stats().bestPrice;
    results.worstPrice = trade->stats().worstPrice;

    tradesResults.push_back(results);
}

void GlobalStatistics::reset()
{
    unrealizedPerformance = 0.0;
    performance = 0.0;

    maxAdjacentWin = 0;
    maxAdjacentLoss = 0;

    worst = 0.0;
    best = 0.0;

    maxDrawDownRate = 0.0;
    maxDrawDown = 0.0;

    succeedTrades = 0;
    failedTrades = 0;
    roeTrades = 0;
    totalTrades = 0;
    canceledTrades = 0;
    openTrades = 0;
    activeTrades = 0;
    stopLossInLoss = 0;
    takeProfitInLoss = 0;
    stopLossInGain = 0;
    takeProfitInGain = 0;

    tradesResults.clear();

    longestFlatPeriod = 0.0;
    avgTimeInMarket = 0.0;

    numTradedDays = 0;
    avgTradePerDay = 0.0;

    currency.reset();
    percent.reset();
}

void GlobalStatistics::add(const Statistics &stats)
{
    unrealizedPerformance += stats.unrealizedPerformance;
    performance += stats.performance;

    maxAdjacentWin = o3d::max(stats.maxAdjacentWin, maxAdjacentWin);
    maxAdjacentLoss = o3d::max(stats.maxAdjacentLoss, maxAdjacentLoss);

    worst = o3d::min(stats.worst, worst);
    best = o3d::max(stats.best, best);

    // not correct because it will need an instant from all concurents running strategies
    maxDrawDownRate = o3d::max(stats.maxDrawDownRate, maxDrawDownRate);
    maxDrawDown = o3d::max(stats.maxDrawDown, maxDrawDown);

    succeedTrades += stats.succeedTrades;
    failedTrades += stats.failedTrades;
    roeTrades += stats.roeTrades;
    totalTrades += stats.totalTrades;
    canceledTrades += stats.canceledTrades;
    openTrades += stats.openTrades;
    activeTrades += stats.activeTrades;
    stopLossInLoss += stats.stopLossInLoss;
    takeProfitInLoss += stats.takeProfitInLoss;
    stopLossInGain += stats.stopLossInGain;
    takeProfitInGain += stats.takeProfitInGain;

    tradesResults.insert(tradesResults.end(), stats.tradesResults.begin(), stats.tradesResults.end());
}

void GlobalStatistics::computeStats()
{
    // @todo
}
