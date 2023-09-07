/**
 * @brief SiiS statistics models and compatible for export.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-28
 */

#ifndef SIIS_STATISTICS_H
#define SIIS_STATISTICS_H

#include <vector>

#include "../base.h"

#include <o3d/core/base.h>

namespace siis {

/**
 * @brief SiiS statistics base model and compatible for export.
 * @author Frederic Scherma
 * @date 2023-04-28
 * It is a per strategy/market statistics model.
 */
class SIIS_API Statistics
{
public:

    o3d::Double unrealizedPerformance = 0.0;  //! in percentiles
    o3d::Double performance = 0.0;  //! realized cumulated performance in percentiles

    o3d::Int32 maxAdjacentWin = 0;
    o3d::Int32 maxAdjacentLoss = 0;

    o3d::Double worst = 0.0;  //! in percentiles
    o3d::Double best = 0.0;   //! in percentiles

    o3d::Double maxDrawDown = 0.0;  //! in percentile

    o3d::Int32 succeedTrades = 0;
    o3d::Int32 failedTrades = 0;
    o3d::Int32 roeTrades = 0;
    o3d::Int32 totalTrades = 0;
    o3d::Int32 canceledTrades = 0;
    o3d::Int32 openTrades = 0;
    o3d::Int32 activeTrades = 0;
    o3d::Int32 stopLossInLoss = 0;
    o3d::Int32 takeProfitInLoss = 0;
    o3d::Int32 stopLossInGain = 0;
    o3d::Int32 takeProfitInGain = 0;

    o3d::Int32 prevDir = 0;

    o3d::Double dailyStartTimestamp = 0.0;
    o3d::Double dailyPerformance = 0.0;
};

/**
 * @brief SiiS statistics base model and compatible for export.
 * @author Frederic Scherma
 * @date 2023-04-28
 * It is a global version of Statistics
 */
class SIIS_API GlobalStatistics
{
public:

    o3d::Double unrealizedPerformance = 0.0;  //! in percentiles
    o3d::Double performance = 0.0;  //! in percentiles

    o3d::Int32 maxAdjacentWin = 0;
    o3d::Int32 maxAdjacentLoss = 0;

    o3d::Double worst = 0.0;  //! in percentiles
    o3d::Double best = 0.0;   //! in percentiles

    o3d::Double maxDrawDown = 0.0;  //! in percentile

    o3d::Int32 succeedTrades = 0;
    o3d::Int32 failedTrades = 0;
    o3d::Int32 roeTrades = 0;
    o3d::Int32 totalTrades = 0;
    o3d::Int32 canceledTrades = 0;
    o3d::Int32 openTrades = 0;
    o3d::Int32 activeTrades = 0;
    o3d::Int32 stopLossInLoss = 0;
    o3d::Int32 takeProfitInLoss = 0;
    o3d::Int32 stopLossInGain = 0;
    o3d::Int32 takeProfitInGain = 0;

    void reset();
    void add(const Statistics &stats);
};

/**
 * @brief The AccountSample class
 * In this order for the output tuple.
 */
class SIIS_API AccountSample
{
public:

    o3d::Double timestamp = 0.0;
    o3d::Double equity = 0.0;
    o3d::Double profitLoss = 0.0;
    o3d::Double drawDown = 0.0;  //! in percentile of equity
};

class SIIS_API AccountStatistics
{
public:

    o3d::Double initialEquity = 0.0;
    o3d::Double finalEquity = 0.0;
    o3d::Double maxDrawDown = 0.0;  //! in percentile of equity
    o3d::Double profitLoss = 0.0;

    std::vector<AccountSample> samples;  //! per day sample of the state of the account
};

} // namespace siis

#endif // SIIS_STATISTICS_H
