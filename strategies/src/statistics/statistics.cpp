/**
 * @brief SiiS statistics models and compatible for export.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-28
 */

#include "siis/statistics/statistics.h"
#include "siis/trade/trade.h"

#include <o3d/core/math.h>

#include <algorithm>

using namespace siis;

o3d::Double average(const std::vector<o3d::Double> &samples) {
    o3d::Double avg = 0.0;

    if (samples.size() > 0) {
        for (o3d::Double x : samples) {
            avg += x;
        }

        avg /= samples.size();
    }

    return avg;
}

o3d::Double stdDeviation(const std::vector<o3d::Double> &samples) {
    if (samples.size() < 2) {
        return 0.0;
    }

    o3d::Double sqrSum = 0.0;
    o3d::Double avg = average(samples);

    for (o3d::Double x : samples) {
        sqrSum += o3d::sqr(x - avg);
    }

    o3d::Double variance = sqrSum / (samples.size() - 1);  // n - 1 degree of freedom
    return o3d::Math::sqrt(variance);
}

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

Sampler& Sampler::finalize()
{
    avg = average(samples);
    stdDev = stdDeviation(samples);

    return *this;
}

PercentSampler& PercentSampler::finalize()
{
    Sampler::finalize();
    return *this;
}

void BaseStatToken::reset()
{
    maxTimeToRecover = 0.0;
    estimateProfitPerMonth = 0.0;
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

PercentSampler::PercentSampler(const o3d::String &_name) :
    Sampler(_name)
{
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

    results.direction = trade->direction();

    results.entryPrice = trade->entryPrice();
    results.exitPrice = trade->exitPrice();

    results.entryTimestamp = trade->stats().firstRealizedEntryTimestamp;
    results.exitTimestamp = trade->stats().lastRealizedExitTimestamp;

    results.bestPrice = trade->stats().bestPrice;
    results.worstPrice = trade->stats().worstPrice;

    results.profitLossPct = trade->profitLossRate();
    results.profitLoss = trade->stats().unrealizedProfitLoss;

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
    avgTradePerDayIncWe = 0.0;

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

// Comparison operator for TradeResults
static bool compare(const TradeResults &a, const TradeResults &b)
{
    return a.exitTimestamp < b.exitTimestamp;
}

void GlobalStatistics::computeStats(const AccountStatistics &accountStats)
{
    const o3d::Double RISK_FREE_RATE_OF_RETURN = 0.0;

    std::vector<o3d::Double> timeInMarketSamples;

    std::vector<o3d::Double> profitPerMonthPct {0.0};
    std::vector<o3d::Double> profitPerMonth {0.0};
    std::vector<o3d::Double> drawDownPerMonthPct {0.0};
    std::vector<o3d::Double> drawDownPerMonth {0.0};

    std::vector<o3d::Double> drawDownsSqrPct;
    std::vector<o3d::Double> drawDownsSqr;

    o3d::Double firstTradeTs = 0.0;
    o3d::Double lastTradeTs = 0.0;

    Sampler anyTradePnl("trade-pnl");
    PercentSampler anyTradePnlPct("trade-pnl");

    Sampler winningTradePnl("winning-pnl");
    PercentSampler winningTradePnlPct("winning-pnl");

    Sampler loosingTradePnl("loosing-pnl");
    PercentSampler loosingTradePnlPct("loosing-pnl");

    PercentSampler mfePct("mfe");
    PercentSampler maePct("mae");
    PercentSampler etdPct("etd");

    PercentSampler eefPct("entry-efficiency");
    PercentSampler xefPct("exit-efficiency");
    PercentSampler tefPct("total-efficiency");

    o3d::Double cumPnlPct = 0.0;
    o3d::Double cumPnl = 0.0;

    o3d::Double maxPnlPct = 0.0;
    o3d::Double maxPnl = 0.0;

    o3d::Double maxPnlPctTs = 0.0;
    o3d::Double maxPnlTs = 0.0;

    std::vector<TradeResults>::iterator prevTradeIt = tradesResults.end();

    // sort trades results by exit timestamp
    std::sort(tradesResults.begin(), tradesResults.end(), &compare);

    o3d::Int32 numTrades = static_cast<o3d::Int32>(tradesResults.size());

    for (auto it = tradesResults.begin(); it != tradesResults.end(); ++it) {
        const TradeResults &trade = *it;

        // cumulative PNL
        anyTradePnlPct.addSample(trade.profitLossPct);
        anyTradePnl.addSample(trade.profitLoss);

        // but this cumulative is used for max time to recover
        cumPnlPct += trade.profitLossPct;
        cumPnl += trade.profitLoss;

        // max time to recover (by percentage)
        if (cumPnlPct >= maxPnlPct) {
            maxPnlPct = cumPnlPct;
            percent.maxTimeToRecover = o3d::max(percent.maxTimeToRecover, trade.exitTimestamp - maxPnlPctTs);
            maxPnlPctTs = trade.exitTimestamp;
        }

        // max time to recover (by currency)
        if (cumPnl >= maxPnl) {
            maxPnl = cumPnl;
            currency.maxTimeToRecover = o3d::max(currency.maxTimeToRecover, trade.exitTimestamp - maxPnlTs);
            maxPnlTs = trade.exitTimestamp;
        }

        // longest flat period and average trades per day
        if (prevTradeIt != tradesResults.end()) {
            longestFlatPeriod = o3d::max(longestFlatPeriod, trade.entryTimestamp - prevTradeIt->exitTimestamp);

            // new monthly sample
            o3d::Int32 elapsedMonths = o3d::min(999, static_cast<o3d::Int32>(
                (baseTime(trade.entryTimestamp, TF_MONTH) - baseTime(prevTradeIt->entryTimestamp, TF_MONTH)) / TF_MONTH));

            if (elapsedMonths > 0) {
                while (elapsedMonths-- > 0) {
                    profitPerMonthPct.push_back(0.0);
                    profitPerMonth.push_back(0.0);
                }
            }
        }

        // average time in market
        timeInMarketSamples.push_back(trade.exitTimestamp - trade.entryTimestamp);

        //  for avg num trades per day
        if (firstTradeTs == 0.0) {
            firstTradeTs = trade.entryTimestamp;
        }

        lastTradeTs = trade.entryTimestamp;

        // winning, loosing trade profit/loss
        if (trade.profitLossPct > 0.0) {
            winningTradePnlPct.addSample(trade.profitLossPct);
        } else if (trade.profitLossPct < 0.0) {
            loosingTradePnlPct.addSample(trade.profitLossPct);
        }

        if (trade.profitLoss > 0.0) {
            winningTradePnl.addSample(trade.profitLoss);
        } else if (trade.profitLoss < 0.0) {
            loosingTradePnl.addSample(trade.profitLoss);
        }

        // cumulative per month
        profitPerMonthPct.back() += trade.profitLossPct;
        profitPerMonth.back() += trade.profitLoss;

        // draw-downs square samples for Ulcer ratio (relative or absolute percentage)
        // drawDownsSqrPct.push_back(o3d::sqr((1.0 + cumPnlPct) / (1.0 + maxPnlPct) - 1.0));
        drawDownsSqrPct.push_back(o3d::sqr(cumPnlPct - maxPnlPct));
        drawDownsSqr.push_back(o3d::sqr(cumPnl - maxPnl));

        // MFE, MAE, ETD (gross value, no trade fees)
        mfePct.addSample(trade.direction * (trade.bestPrice - trade.entryPrice) / trade.entryPrice);
        maePct.addSample(trade.direction * (trade.entryPrice - trade.worstPrice) / trade.entryPrice);
        etdPct.addSample(trade.direction * (trade.bestPrice - trade.exitPrice) / trade.exitPrice);

        // efficiency
        eefPct.addSample((trade.bestPrice - trade.entryPrice) / (trade.bestPrice - trade.worstPrice));
        xefPct.addSample((trade.exitPrice - trade.worstPrice) / (trade.bestPrice - trade.worstPrice));
        tefPct.addSample((trade.exitPrice - trade.entryPrice) / (trade.bestPrice - trade.worstPrice));

        // keep the previous trade details
        prevTradeIt = it;
    }

    // per month draw-down from trader account samples
    o3d::Int32 prevSampleMonth = 0;

    for (size_t n = 0; n < accountStats.samples.size(); ++n) {
        if (prevSampleMonth == 0) {
            prevSampleMonth = baseTime(accountStats.samples[n].timestamp, TF_MONTH);
        }

        o3d::Double sampleBt = baseTime(accountStats.samples[n].timestamp, TF_MONTH);
        o3d::Double elapsedMonths = static_cast<o3d::Int32>((sampleBt - prevSampleMonth) / TF_MONTH);
        if (elapsedMonths > 0) {
            while (elapsedMonths-- > 0) {
                drawDownPerMonthPct.push_back(0.0);
                drawDownPerMonth.push_back(0.0);
            }
        }

        // a positive value
        drawDownPerMonthPct.back() = o3d::max(drawDownPerMonthPct.back(), accountStats.samples[n].drawDownRate);
        drawDownPerMonth.back() = o3d::max(drawDownPerMonth.back(), accountStats.samples[n].drawDown);
    }

    // results
    avgTimeInMarket = average(timeInMarketSamples);

    o3d::Double firstDayTs = baseTime(firstTradeTs, TF_DAY);
    o3d::Double lastDayTs = baseTime(lastTradeTs, TF_DAY);

    // at least one day because of min one trade
    numTradedDays = static_cast<o3d::Int32>(((lastDayTs - firstDayTs) / TF_DAY) + 1);

    avgTradePerDayIncWe = static_cast<o3d::Double>(numTrades) / numTradedDays;
    avgTradePerDay = avgTradePerDayIncWe * (252.0 / 365.0);

    // estimate profitability per month
    // percent.estimateProfitPerMonth = pow((1.0 + cumPnlPct), (1.0 * (30.5 / numTradedDays))) - 1.0;
    percent.estimateProfitPerMonth = cumPnlPct * (30.5 / numTradedDays);
    currency.estimateProfitPerMonth = cumPnl * (30.5 / numTradedDays);

    // Sharpe Ratio
    if (profitPerMonthPct.size() > 1) {
        // o3d::Int32 dof = static_cast<o3d::Int32>(profitPerMonthPct.size() - 1);

        // Sharpe ratio (Student t distribution)
        o3d::Double tmp = stdDeviation(profitPerMonthPct);
        if (tmp != 0.0) {
            percent.sharpeRatio = ((percent.estimateProfitPerMonth - RISK_FREE_RATE_OF_RETURN) / tmp);
        }
        tmp = stdDeviation(profitPerMonth);
        if (tmp != 0.0) {
            currency.sharpeRatio = ((currency.estimateProfitPerMonth - RISK_FREE_RATE_OF_RETURN) / tmp);
        }

        // Sortino ratio (Student t distribution)
        tmp = stdDeviation(drawDownPerMonthPct);
        if (tmp != 0.0) {
            percent.sortinoRatio = ((percent.estimateProfitPerMonth - RISK_FREE_RATE_OF_RETURN) / tmp);
        }
        tmp = stdDeviation(drawDownPerMonth);
        if (tmp != 0.0) {
            currency.sortinoRatio = ((currency.estimateProfitPerMonth - RISK_FREE_RATE_OF_RETURN) / tmp);
        }

        // Ulcer index
        percent.ulcerIndex = o3d::Math::sqrt(average(drawDownsSqrPct));
        currency.ulcerIndex = o3d::Math::sqrt(average(drawDownsSqr));
    }

    // Total PNL, Winning PNL, Loosing PNL
    percent.samplers.push_back(anyTradePnlPct.finalize());
    currency.samplers.push_back(anyTradePnl.finalize());
    percent.samplers.push_back(winningTradePnlPct.finalize());
    currency.samplers.push_back(winningTradePnl.finalize());
    percent.samplers.push_back(loosingTradePnlPct.finalize());
    currency.samplers.push_back(loosingTradePnl.finalize());

    // Win/Loss Rate (after finalize)
    percent.avgWinLossRate = loosingTradePnlPct.avg != 0.0 ? winningTradePnlPct.avg / loosingTradePnlPct.avg : 1.0;
    currency.avgWinLossRate = loosingTradePnl.avg != 0.0 ? winningTradePnl.avg / loosingTradePnl.avg : 1.0;

    // MFE, MAE, ETD
    percent.samplers.push_back(mfePct.finalize());
    percent.samplers.push_back(maePct.finalize());
    percent.samplers.push_back(etdPct.finalize());

    // efficiency
    percent.samplers.push_back(eefPct.finalize());
    percent.samplers.push_back(xefPct.finalize());
    percent.samplers.push_back(tefPct.finalize());
}
