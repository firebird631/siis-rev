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
    m_maxTrades(1),
    m_tradeDelay(0.0),
    m_needUpdate(false),
    m_tradeType(Trade::TYPE_ASSET)
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

void Strategy::setActiveStats(o3d::Double performance, o3d::Double drawDown, o3d::Int32 pending, o3d::Int32 actives)
{
    m_stats.activeTrades = actives;
    m_stats.openTrades = pending;

    m_stats.unrealizedPerformance = performance;

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

    m_lastTimestamp = timestamp;
    m_processing = false;
}

void Strategy::log(o3d::Double timeframe, const o3d::String &channel, const o3d::String &msg)
{
    m_handler->log(timeframe, m_market->marketId(), channel, msg);
}

void Strategy::addClosedTrade(Trade *trade)
{
    if (trade) {
        if (trade->isCanceled()) {
            m_stats.canceledTrades += 1;

        } else if (trade->isClosed()) {
            m_stats.performance += trade->profitLossRate();

            if (trade->profitLossRate() < 0.0) {
                m_stats.failedTrades += 1;
            } else if (trade->profitLossRate() > 0.0) {
                m_stats.succeedTrades += 1;
            } else {
                // @todo
                m_stats.roeTrades += 1;
            }

            m_stats.totalTrades += 1;

            // @todo
//            m_stats.maxAdjacentLoss
//            m_stats.maxAdjacentWin

//            m_stats.stopLossInGain
//            m_stats.stopLossInLoss
//            m_stats.takeProfitInGain
//            m_stats.takeProfitInLoss

            m_stats.best = o3d::max(m_stats.best, trade->profitLossRate());
            m_stats.worst = o3d::min(m_stats.worst, trade->profitLossRate());
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
