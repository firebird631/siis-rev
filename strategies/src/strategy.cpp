/**
 * @brief SiiS strategy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/strategy.h"
#include "siis/handler.h"
#include "siis/config/strategyconfig.h"

#include "siis/trade/stdtrademanager.h"

#include <algorithm>

using namespace siis;

Handler::~Handler()
{

}

Strategy::Strategy(Handler *handler, const o3d::String &identifier) :
    m_handler(handler),
    m_identifier(identifier),
    m_curState(STATE_NEW),
    m_nextState(STATE_INITIALIZED),
    m_market(nullptr),
    m_lastTimestamp(0),
    m_processing(false),
    m_reversal(false),
    m_hedging(false),
    m_allowShort(true),
    m_maxTrades(1),
    m_tradeDelay(0.0),
    m_needUpdate(false),
    m_tradeType(Trade::TYPE_ASSET),
    m_baseQuantity(1.0)
{
    m_properties["name"] = "undefined";
    m_properties["author"] = "undefined";
    m_properties["date"] = "undefined";
    m_properties["revision"] = "0";
    m_properties["copyright"] = "undefined";
    m_properties["comment"] = "";
}

Strategy::~Strategy()
{

}

void Strategy::init(Config *config)
{
    // author defined strategy properties details (for purpose only)
    setProperty("name", "Strategy");
    setProperty("author", "Unknown author (john@doe.com)");
    setProperty("date", "2042-09-11");
    setProperty("revision", "1");
    setProperty("copyright", "2018-2019 Dream Overflow");
    setProperty("comment", "Sample strategy");
    // ...

    // needed data source
    // addDataSource(...)

    // default strategie parameters
    // ...

    m_brokerId = config->getBrokerId();
}

void Strategy::initBasicsParameters(StrategyConfig &conf)
{
    m_reversal = conf.root().get("reversal", true).asBool();
    m_hedging = conf.root().get("hedging", false).asBool();
    m_allowShort = conf.root().get("allow-short", true).asBool();
    m_maxTrades = conf.root().get("max-trades", 1).asInt();
    m_tradeDelay = conf.root().get("trade-delay", 30).asDouble();
    m_needUpdate = conf.root().get("need-update", false).asBool();
    m_tradeType = conf.tradeType();

    m_baseTimeframe = conf.baseTimeframe();

    // stream data sources
    if (m_baseTimeframe <= 0.0) {
        addTickDataSource();
    } else {
        addMidOhlcDataSource(m_baseTimeframe);
    }
}

void Strategy::setActiveStats(o3d::Double performance,
                              o3d::Double drawDownRate,
                              o3d::Double drawDown,
                              o3d::Int32 pending,
                              o3d::Int32 actives)
{
    m_stats.activeTrades = actives;
    m_stats.openTrades = pending;

    m_stats.unrealizedPerformance = performance;

    if (drawDownRate > m_stats.maxDrawDownRate) {
        m_stats.maxDrawDownRate = drawDownRate;
    }

    if (drawDown > m_stats.maxDrawDown) {
        m_stats.maxDrawDown = drawDown;
    }
}

const std::list<DataSource> Strategy::getDataSources() const
{
    return m_dataSources;
}

void Strategy::setMarket(Market *market)
{
    m_market = market;
}

void Strategy::setBaseQuantity(o3d::Double qty)
{
    m_baseQuantity = qty;
}

o3d::String Strategy::property(const o3d::String &propName) const
{
    auto it = m_properties.find(propName);
    if (it != m_properties.cend()) {
        return it->second;
    } else {
        return "";
    }
}

void Strategy::process(o3d::Double timestamp)
{
    m_processing = true;

    prepare(timestamp);
    compute(timestamp);
    finalize(timestamp);

    updateStats();

    m_lastTimestamp = timestamp;
    m_processing = false;
}

void Strategy::log(o3d::Double timeframe, const o3d::String &channel, const o3d::String &msg,
                   o3d::System::MessageLevel type)
{
    m_handler->log(timeframe, m_market->marketId(), channel, msg, type);
}

void Strategy::addClosedTrade(Trade *trade)
{
    if (trade) {
        if (trade->isCanceled()) {
            m_stats.canceledTrades += 1;

        } else if (trade->isClosed()) {
            o3d::Double rpnl = trade->profitLossRate();

            // managed fees
            rpnl -= trade->entryFeesRate();
            rpnl -= trade->exitFeesRate();

            m_stats.performance += rpnl;

            if (o3d::abs(rpnl*market()->mid()) < market()->spread()) {
                m_stats.roeTrades += 1;
                m_stats.prevDir = 0;

            } else if (rpnl < 0.0) {
                m_stats.failedTrades += 1;

                if (m_stats.prevDir <= 0) {
                    --m_stats.prevDir;
                } else {
                    m_stats.prevDir = -1;
                }

                m_stats.maxAdjacentLoss = o3d::max(m_stats.maxAdjacentLoss, -m_stats.prevDir);

                if (trade->stats().exitReason == TradeStats::REASON_STOP_LOSS_MARKET ||
                    trade->stats().exitReason == TradeStats::REASON_STOP_LOSS_LIMIT) {
                    m_stats.stopLossInLoss += 1;

                } else if (trade->stats().exitReason == TradeStats::REASON_TAKE_PROFIT_LIMIT ||
                           trade->stats().exitReason == TradeStats::REASON_TAKE_PROFIT_MARKET) {
                    m_stats.takeProfitInLoss += 1;
                }
            } else if (rpnl > 0.0) {
                m_stats.succeedTrades += 1;

                if (m_stats.prevDir >= 0) {
                    ++m_stats.prevDir;
                } else {
                    m_stats.prevDir = 1;
                }

                m_stats.maxAdjacentWin = o3d::max(m_stats.maxAdjacentWin, m_stats.prevDir);

                if (trade->stats().exitReason == TradeStats::REASON_STOP_LOSS_MARKET ||
                    trade->stats().exitReason == TradeStats::REASON_STOP_LOSS_LIMIT) {
                    m_stats.stopLossInGain += 1;

                } else if (trade->stats().exitReason == TradeStats::REASON_TAKE_PROFIT_LIMIT ||
                           trade->stats().exitReason == TradeStats::REASON_TAKE_PROFIT_MARKET) {
                    m_stats.takeProfitInGain += 1;
                }
            }

            m_stats.totalTrades += 1;

            m_stats.best = o3d::max(m_stats.best, rpnl);
            m_stats.worst = o3d::min(m_stats.worst, rpnl);

            // reset each EOD
            if (m_stats.dailyStartTimestamp == 0.0) {
                m_stats.dailyStartTimestamp = baseTime(lastTimestamp(), TF_DAY);
            }

            if (lastTimestamp() - m_stats.dailyStartTimestamp >= TF_DAY) {
                m_stats.dailyStartTimestamp = baseTime(lastTimestamp(), TF_DAY);

                o3d::String msg = o3d::String("Daily performance {0}% on a partial of {1}%")
                                      .arg(m_stats.dailyPerformance * 100, 2)
                                      .arg(m_stats.performance * 100, 2);

                log(TF_DAY, "daily-report", msg, o3d::System::MSG_CRITICAL);

                m_stats.dailyPerformance = 0.0;
            }

            m_stats.dailyPerformance += rpnl;

            // keep some interesting values for final stats
            m_stats.addTrade(trade);
        }
    }
}

void Strategy::setProperty(const o3d::String propertyName, const o3d::String value)
{
    m_properties[propertyName] = value;
}

void Strategy::addTickDataSource()
{
    for (DataSource ds : m_dataSources) {
        if (ds.type == DataSource::TICK) {
            return;
        }
    }

    m_dataSources.push_back(DataSource(DataSource::TICK));
}

void Strategy::addMidOhlcDataSource(o3d::Double timeframe)
{
    for (DataSource ds : m_dataSources) {
        if (ds.type == DataSource::OHLC_MID) {
            return;
        }
    }

    m_dataSources.push_back(DataSource(DataSource::OHLC_MID, timeframe));
}

void Strategy::addBidOhlcDataSource(o3d::Double timeframe)
{
    for (DataSource ds : m_dataSources) {
        if (ds.type == DataSource::OHLC_BID) {
            return;
        }
    }

    m_dataSources.push_back(DataSource(DataSource::OHLC_BID, timeframe));
}

void Strategy::addAskOhlcDataSource(o3d::Double timeframe)
{
    for (DataSource ds : m_dataSources) {
        if (ds.type == DataSource::OHLC_ASK) {
            return;
        }
    }

    m_dataSources.push_back(DataSource(DataSource::OHLC_ASK, timeframe));
}

void Strategy::addOrderBookDataSource(o3d::Int32 depth)
{
    for (DataSource ds : m_dataSources) {
        if (ds.type == DataSource::ORDER_BOOK) {
            return;
        }
    }

    m_dataSources.push_back(DataSource(DataSource::ORDER_BOOK, 0.0, depth));
}

void Strategy::adjustOhlcFetchRange(o3d::Int32 depth, o3d::Double &fromTs, o3d::Double &toTs, o3d::Int32 &nLast) const
{
    if (market()->type() == Market::TYPE_CRYPTO) {
        // nothing to do h24, d7 market
        return;
    }

    // there is multiples case, weekend off and nationals days off
    // and the case of stocks markets closed during the local night
    // but also some 15 min of twice on indices ...

    // so many complexes cases then we try to get the max of last n OHLCs
    // here simple direct solution but not correct in case of leaks of data
    nLast = depth;
    fromTs = 0.0;
}

void Strategy::setInitialized()
{
    if (m_nextState == STATE_INITIALIZED) {
        m_curState = m_nextState;
        m_nextState = STATE_PREPARED_MARKET_DATA;
    }
}

void Strategy::setMarketDataPrepared()
{
    if (m_nextState == STATE_PREPARED_MARKET_DATA) {
        m_curState = m_nextState;
        m_nextState = STATE_READY;
    }
}

void Strategy::setReady()
{
    if (m_nextState == STATE_READY) {
        m_curState = m_nextState;
        m_nextState = STATE_RUNNING;
    }
}

void Strategy::setRunning()
{
    if (m_nextState == STATE_RUNNING) {
        m_curState = m_nextState;
        m_nextState = STATE_TERMINATED;

        m_processing = false;
    }
}

void Strategy::setTerminated()
{
    if (m_nextState == STATE_TERMINATED) {
        m_curState = m_nextState;
        m_nextState = STATE_LAST;
    }
}
