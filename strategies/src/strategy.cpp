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
#include <map>

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
    m_baseQuantity(1.0),
    m_timezone(0.0),
    m_sessionOffset(0.0),
    m_sessionDuration(0.0)
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

    if (config == nullptr) {
        return;
    }

    m_brokerId = config->getBrokerId();
}

static void tradingSessionFromStr(Json::Value &trading, std::vector<TradingSession> &out)
{
    if (!trading.isString()) {
        // @todo format error
        return;
    }

    // ISO Monday 1..7
    const std::map<o3d::String, o3d::Int32> DAYS_OF_WEEK = {
        {"any", -2},
        {"dow", -1},
        {"mon", 1},
        {"tue", 2},
        {"wed", 3},
        {"thu", 4},
        {"fri", 5},
        {"sat", 6},
        {"sun", 7}
    };

    o3d::String value = trading.asCString();

    if (value.count('/') != 1) {
        // @todo format error
        return;
    }

    o3d::Int32 slash = value.find('/');
    o3d::Int32 dash = value.find('-');

    o3d::String dow = value.sub(0, slash);
    if (DAYS_OF_WEEK.find(dow) == DAYS_OF_WEEK.cend()) {
        // @todo format error
        return;
    }

    // "dow/15:30-21:30"
    o3d::Double fromDuration = durationFromStr(value.sub(slash+1, dash));
    o3d::Double toDuration = durationFromStr(value.sub(dash+1));

    if (dow == "any") {
        // any days
        for (o3d::Int32 d = 1; d < 8; ++d) {
            TradingSession session;
            session.dayOfWeek = d;
            session.fromTime = fromDuration;
            session.toTime = toDuration;
            out.push_back(session);
        }
    } else if (dow == "dow") {
        // any days of week
        for (o3d::Int32 d = 1; d < 6; ++d) {
            TradingSession session;
            session.dayOfWeek = d;
            session.fromTime = fromDuration;
            session.toTime = toDuration;
            out.push_back(session);
        }
    } else {
        // day is defined
        TradingSession session;
        session.dayOfWeek = DAYS_OF_WEEK.find(dow)->second;
        session.fromTime = fromDuration;
        session.toTime = toDuration;
        out.push_back(session);
    }
}

void Strategy::initBasicsParameters(StrategyConfig &conf)
{
    // eventually parse per market overrides
    conf.parseMarketOverrides(market()->marketId());

    m_reversal = conf.root().get("reversal", true).asBool();
    m_hedging = conf.root().get("hedging", false).asBool();
    m_allowShort = conf.root().get("allow-short", true).asBool();
    m_maxTrades = conf.root().get("max-trades", 1).asInt();
    m_tradeDelay = conf.root().get("trade-delay", 30).asDouble();
    m_needUpdate = conf.root().get("need-update", false).asBool();
    m_tradeType = conf.tradeType();

    m_baseTimeframe = conf.baseTimeframe();

    // trading sessions
    if (conf.root().isMember("sessions")) {
        Json::Value sessions = conf.root().get("sessions", Json::Value());

        m_timezone = sessions.get("timezone", 0.0).asDouble();
        m_sessionOffset = durationFromStr(sessions.get("offset", 0.0).asString().c_str());
        m_sessionDuration = durationFromStr(sessions.get("duration", 0.0).asString().c_str());

        if (sessions.isMember("trading")) {
            Json::Value trading = sessions.get("trading", Json::Value());
            if (!trading.empty() && trading.isArray()) {
                for (auto it = trading.begin(); it != trading.end(); ++it) {
                    tradingSessionFromStr(*it, m_tradingSessions);
                }
            } else if (trading.isString()) {
                tradingSessionFromStr(trading, m_tradingSessions);
            }
        }
    }

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


void Strategy::setTimezone(o3d::Double tz)
{
    m_timezone = tz;
}

void Strategy::setSessionOffset(o3d::Double offset)
{
    m_sessionOffset = offset;
}

void Strategy::setSessionDuration(o3d::Double duration)
{
    m_sessionDuration = duration;
}

void Strategy::addTradingSession(o3d::Int8 dayOfWeek, o3d::Double fromTime, o3d::Double toTime)
{
    TradingSession tradeingSession;
    tradeingSession.dayOfWeek = dayOfWeek;
    tradeingSession.fromTime = fromTime;
    tradeingSession.toTime = toTime;

    m_tradingSessions.push_back(tradeingSession);
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

void Strategy::log(const o3d::String &unit, const o3d::String &channel, const o3d::String &msg,
                   o3d::System::MessageLevel type)
{
    m_handler->log(unit, m_market->marketId(), channel, msg, type);
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

                o3d::String msg = o3d::String("Daily perf {0}% on {1}%")
                                      .arg(m_stats.dailyPerformance * 100, 2)
                                      .arg(m_stats.performance * 100, 2);

                log("1d", "daily-report", msg, o3d::System::MSG_CRITICAL);

                m_stats.dailyPerformance = 0.0;
            }

            m_stats.dailyPerformance += rpnl;

            // keep some interesting values for final stats
            m_stats.addTrade(trade);
        }
    }
}

o3d::Bool Strategy::allowedTradingSession(o3d::Double timestamp) const
{
    if (hasTradingSessions()) {
        // compute daily offset in seconds
        o3d::DateTime today;

        // add the timezone offset in hours
        today.fromTime(timestamp + timezone() * 3600, true);

        o3d::Double todayTime = today.hour * 3600 + today.minute * 60 + today.second;
        o3d::Bool allow = false;

        for (const TradingSession &session : m_tradingSessions) {
            // printf("%f %i -> %i %i\n", timestamp, session.dayOfWeek, today.getIsoDayOfWeek(), today.getDayOfWeek());
            if (session.dayOfWeek == today.getIsoDayOfWeek()) {
                if (session.fromTime <= todayTime && todayTime <= session.toTime) {
                    allow = true;
                    break;
                }
            }
        }

        if (!allow) {
            return false;
        }
    }

    return true;
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

//! Convert date to number of days
//! @warning Dates before Oct. 1582 are inaccurate.
static o3d::Int64 gday(const o3d::DateTime &d)
{
    o3d::Int64 y, m;

    m = (d.month + 9) % 12;
    y = (d.year - m / 10);

    return y*365 + y/4 - y/100 + y/400 + (m*306 + 5)/10 + (d.mday - 1);
}

o3d::Double Strategy::adjustHistoryDuration(o3d::Double history, o3d::Double toTs) const
{
    if (market()->type() == Market::TYPE_CRYPTO) {
        // h24/d7
        return history;
    } else if (market()->type() == Market::TYPE_STOCK) {
        // @todo 6.5h per week-day
        return history;
    } else if (market()->type() == Market::TYPE_INDICE) {
        // @todo 1h day off, week-day
/*
            o3d::DateTime firstDay;
            firstDay.fromTime(fromTs);

            o3d::DateTime lastDay;
            lastDay.fromTime(toTs);

            o3d::Int64 numDays = gday(lastDay) - gday(firstDay);

            // @todo
            if (lastDay.getIsoDayOfWeek() > 5) {
                // market open at 20:00 sunday, so minus 4h
                fromTs -= (lastDay.getIsoDayOfWeek() - 5) * TF_DAY;

                if (lastDay.getDayOfWeek() == 7) {
                    if (lastDay.hour >= 20) {
                        fromTs += (lastDay.hour - 20) * TF_HOUR;
                    }
                }
            }

            if (numDays >= 7) {
                // add two days per week
                fromTs -= (2 * (TF_DAY - TF_4HOUR)) * (numDays / 7);
            }

            // 1 hour off per week-day
            if (numDays >= 7) {
                fromTs -= ((numDays * 5) / 7) * TF_HOUR;
            } else if (numDays >= 1) {
                fromTs -= numDays * TF_HOUR;
            } else {
                if (history > TF_HOUR) {
                    if (lastDay.hour == 0) {
                        // if start
                        fromTs -= TF_HOUR;
                    }
                }
            }
            */

        return history;
    } else if (market()->type() == Market::TYPE_COMMODITY) {
        // @todo a little bit different than indices, week-day
        return history;
    } else if (market()->type() == Market::TYPE_CURRENCY) {
        // @todo 1h day off, week-day, gold, silver might be currency type
        return history;
    }

    return history;
}

void Strategy::adjustOhlcFetchRange(o3d::Double history, o3d::Int32 depth,
                                    o3d::Double &fromTs, o3d::Double &toTs,
                                    o3d::Int32 &nLast) const
{
    if (market()->type() == Market::TYPE_CRYPTO) {
        // crypto are h24, d7 market
        if (fromTs && toTs) {
            // from till date
            // fromTs don't change
            // toTs don't change
            nLast = 0;
            return;
        } else if (!fromTs && toTs) {
            // n last till date
            fromTs = 0;
            // toTs don't change
            if (history > 0) {
                fromTs = toTs - history + 1.0;
            } else {
                nLast = depth;
            }
            return;
        } else if (fromTs && !toTs) {
            // from date till now
            // fromTs don't change
            toTs = 0;
            nLast = 0;
            return;
        } else {
            // n last till now
            fromTs = 0;
            toTs = 0;
            nLast = depth;
            return;
        }
    } else {
        if (toTs > 0 && history > 0) {
            o3d::Double adjustedHistory = adjustHistoryDuration(history, toTs);
            fromTs = toTs - adjustedHistory;
        } else {
            // either n last till date, or n last till now
            fromTs = 0.0;
            // toTs don't change
            nLast = depth;
        }
    }
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
