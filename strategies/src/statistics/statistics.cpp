/**
 * @brief SiiS statistics models and compatible for export.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-28
 */

#include "siis/statistics/statistics.h"

using namespace siis;


void GlobalStatistics::reset()
{
    performance = 0.0;

    maxAdjacentWin = 0;
    maxAdjacentLoss = 0;

    worst = 0.0;
    best = 0.0;

    maxDrawDown = 0.0;

    succeedTrades = 0;
    failedTrades = 0;
    roeTrades = 0;
    totalTrades = 0;
    openTrades = 0;
    activeTrades = 0;
    stopLossInLoss = 0;
    takeProfitInLoss = 0;
    stopLossInGain = 0;
    takeProfitInGain = 0;
}

void GlobalStatistics::add(const Statistics &stats)
{
    performance += stats.performance;

    maxAdjacentWin = o3d::max(stats.maxAdjacentWin, maxAdjacentWin);
    maxAdjacentLoss = o3d::max(stats.maxAdjacentLoss, maxAdjacentLoss);

    worst += o3d::max(stats.worst, worst);
    best += o3d::max(stats.best, best);

    maxDrawDown = o3d::max(stats.maxDrawDown, maxDrawDown);

    succeedTrades += stats.succeedTrades;
    failedTrades += stats.failedTrades;
    roeTrades += stats.roeTrades;
    totalTrades += stats.totalTrades;
    openTrades += stats.openTrades;
    activeTrades += stats.activeTrades;
    stopLossInLoss += stats.stopLossInLoss;
    takeProfitInLoss += stats.takeProfitInLoss;
    stopLossInGain += stats.stopLossInGain;
    takeProfitInGain += stats.takeProfitInGain;
}