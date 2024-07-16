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

#include <o3d/core/string.h>

namespace siis {

class Trade;

/**
 * @brief SiiS statistic sampler for notional or currency values.
 * @author Frederic Scherma
 * @date 2024-07-15
 */
class SIIS_API Sampler
{
public:

    o3d::String name;
    std::vector<o3d::Double> samples;
    o3d::Double minValue = 0.0;
    o3d::Double maxValue = 0.0;
    o3d::Double cumulated = 0.0;

    o3d::Double avg = 0.0;
    o3d::Double stdDev = 0.0;

    Sampler(const o3d::String &_name);

    void addSample(o3d::Double value);

    o3d::Int32 count() const;

    Sampler& finalize();
};


/**
 * @brief SiiS statistic sampler for percentage values.
 * @author Frederic Scherma
 * @date 2024-07-15
 */
class SIIS_API PercentSampler : public Sampler
{
public:

    PercentSampler(const o3d::String &_name);

    PercentSampler& finalize();
};


/**
 * @brief SiiS statistic token for notional values.
 * @author Frederic Scherma
 * @date 2024-07-15
 */
class SIIS_API BaseStatToken
{
public:

    o3d::Double maxTimeToRecover = 0.0;

    o3d::Double estimateProfitPerMonth = 0.0;

    o3d::Double avgTrade = 0.0;

    o3d::Double avgWinningTrade = 0.0;
    o3d::Double avgLoosingTrade = 0.0;
    o3d::Double avgWinLossRate = 0.0;    //!< avgWinningTrade / avgLoosingTrade

    o3d::Double sharpeRatio = 1.0;
    o3d::Double sortinoRatio = 1.0;
    o3d::Double ulcerIndex = 0.0;

    void reset();
};


/**
 * @brief SiiS statistic token for currency values.
 * @author Frederic Scherma
 * @date 2024-07-15
 */
class SIIS_API CurrencyStatToken : public BaseStatToken
{
public:

    std::vector<Sampler> samplers;

    void reset();
};


/**
 * @brief SiiS statistic token for percentage values.
 * @author Frederic Scherma
 * @date 2024-07-15
 */
class SIIS_API PercentStatToken : public BaseStatToken
{
public:

    std::vector<PercentSampler> samplers;

     void reset();
};


/**
 * @brief SiiS results data for a trade.
 * @author Frederic Scherma
 * @date 2024-07-15
 */
class SIIS_API TradeResults
{
public:

    o3d::Int32 direction;

    o3d::Double entryPrice;      //!< average entry price
    o3d::Double exitPrice;       //!< average exit price

    o3d::Double entryTimestamp;  //!< first realized entry timestamp
    o3d::Double exitTimestamp;   //!< last realized entry timestamp

    o3d::Double bestPrice;
    o3d::Double worstPrice;

    o3d::Double profitLossPct;
    o3d::Double profitLoss;
};


/**
 * @brief SiiS statistics base model and compatible for export for a strategy (per market).
 * @author Frederic Scherma
 * @date 2023-04-28
 * It is a per strategy/market statistics model.
 */
class SIIS_API Statistics
{
public:

    o3d::Double unrealizedPerformance = 0.0;  //!< in percentiles
    o3d::Double performance = 0.0;  //!< realized cumulated performance in percentiles

    o3d::Int32 maxAdjacentWin = 0;
    o3d::Int32 maxAdjacentLoss = 0;

    o3d::Double worst = 0.0;  //!< in percentiles
    o3d::Double best = 0.0;   //!< in percentiles

    o3d::Double maxDrawDownRate = 0.0;  //!< in percentile
    o3d::Double maxDrawDown = 0.0;      //!< in currency

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

    std::vector<TradeResults> tradesResults;   //!< any trades
    void addTrade(const Trade *trade);

    // the following members are only used for internals processes
    o3d::Int32 prevDir = 0;

    o3d::Double dailyStartTimestamp = 0.0;
    o3d::Double dailyPerformance = 0.0;
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

    o3d::Double drawDownRate = 0.0;  //!< in percentile of equity
    o3d::Double drawDown = 0.0;      //!< in currency
};


/**
 * @brief The AccountStatistics class
 * Statistics for the account in currency, with equity and pnl.
 * Also contains samples at a daily frequency.
 */
class SIIS_API AccountStatistics
{
public:

    o3d::Double initialEquity = 0.0;
    o3d::Double finalEquity = 0.0;
    o3d::Double profitLoss = 0.0;

    o3d::Double maxDrawDownRate = 0.0;  //!< in percentile of equity
    o3d::Double maxDrawDown = 0.0;      //!< in currency

    std::vector<AccountSample> samples;  //!< per day sample of the state of the account
};

/**
 * @brief SiiS statistics base model and compatible for export, global to any strategies (for all markets).
 * @author Frederic Scherma
 * @date 2023-04-28
 * It is a global version of Statistics
 */
class SIIS_API GlobalStatistics
{
public:

    o3d::Double unrealizedPerformance = 0.0;  //!< in percentiles
    o3d::Double performance = 0.0;  //!< in percentiles

    o3d::Int32 maxAdjacentWin = 0;
    o3d::Int32 maxAdjacentLoss = 0;

    o3d::Double worst = 0.0;  //!< in percentiles
    o3d::Double best = 0.0;   //!< in percentiles

    o3d::Double maxDrawDownRate = 0.0;  //!< in percentile
    o3d::Double maxDrawDown = 0.0;      //!< in currency

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

    std::vector<TradeResults> tradesResults;

    o3d::Double longestFlatPeriod = 0.0;     //!< longest duration between two trade, but issue with weekend or other holidays
    o3d::Double avgTimeInMarket = 0.0;

    o3d::Int32 numTradedDays = 0;
    o3d::Double avgTradePerDay = 0.0;        //!< excluding weekend (majority of the markets)
    o3d::Double avgTradePerDayIncWe = 0.0;   //!< including weekend (crypto)

    CurrencyStatToken currency;
    PercentStatToken percent;

    void reset();
    void add(const Statistics &stats);

    void computeStats(const AccountStatistics& accountStats);
};

} // namespace siis

#endif // SIIS_STATISTICS_H
