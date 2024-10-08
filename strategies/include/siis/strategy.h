/**
 * @brief SiiS strategy base class from which to inherit.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_STRATEGY_H
#define SIIS_STRATEGY_H

#include "base.h"

#include <o3d/core/string.h>
#include <o3d/core/mutex.h>
#include <o3d/core/runnable.h>
#include <o3d/core/stringmap.h>

#include "siis/market.h"
#include "siis/config/config.h"
#include "siis/datasource.h"
#include "siis/statistics/statistics.h"
#include "siis/trade/trade.h"
#include "siis/tradingsession.h"

namespace siis {

class Handler;
class Connector;
class Database;
class OrderSignal;
class PositionSignal;

/**
 * @brief Strategy base class from which to inherit.
 * @author Frederic Scherma
 * @date 2019-03-05
 * @todo About strategy report, having a report with details of the used parameters, and the revision + name of the
 * strategy
 */
class SIIS_API Strategy
{
public:

    enum State {
        STATE_NEW = 0,
        STATE_INITIALIZED,
        STATE_PREPARED_MARKET_DATA,
        STATE_READY,
        STATE_RUNNING,
        STATE_TERMINATED,
        STATE_LAST
    };

    Strategy(Handler *handler, const o3d::String &identifier);
    virtual ~Strategy();

    const o3d::String& identifier() const { return m_identifier; }
    const o3d::String& brokerId() const { return m_brokerId; }

    /**
     * @brief property Get a strategy global property (not the parameters).
     */
    o3d::String property(const o3d::String &propName) const;

    //
    // initialization
    //

    /**
     * @brief init Strategy initialization.
     * Default, defines the global property for this instance.
     * Override this method to setup strategy parameters set from config source and
     * call the this default implementation before.
     */
    virtual void init(Config *config);

    /**
     * @brief terminate Strategy termination.
     */
    virtual void terminate(Connector *connector, Database *db) = 0;

    /**
     * @brief getDataSources
     * @return
     */
    const std::list<DataSource> getDataSources() const;

    /**
     * @brief setMarket Set the related market model.
     */
    void setMarket(Market *market);

    /**
     * @brief setBaseQuantity Set per trade base quantity.
     */
    void setBaseQuantity(o3d::Double qty);

    /**
     * @brief baseQuantity Get traded base quantity (could be different for a context).
     */
    o3d::Double baseQuantity() const { return m_baseQuantity; }

    //
    // preparing
    //

    /**
     * @brief prepareMarket Query market data before the strategy be in running state.
     */
    virtual void prepareMarketData(Connector *connector, Database *db, o3d::Double fromTs, o3d::Double toTs) = 0;

    /**
     * @brief finalizeMarketData Once market data are receive do the rest here, and set the ready state.
     */
    virtual void finalizeMarketData(Connector *connector, Database *db) = 0;

    //
    // processing
    //

    /**
     * @brief acquire Strategy mutex lock.
     */
    void acquire() const { m_mutex.lock(); }

    /**
     * @brief release Strategy mutex unlock.
     */
    void release() const { m_mutex.unlock(); }

    /**
     * @brief iterate Process the current iteration. First call prepare, then compute,
     * and then finalize.
     * @param timestamp
     */
    void process(o3d::Double timestamp);

    /**
     * @brief onTicksUpdate Update from the market data at each new input ticks array.
     * @param timestamp Current timestamp.
     * Do here the iteration data preparation (OHLC, ticks array, and other any data sources)
     * that need to be ready for the process method.
     */
    virtual void onTickUpdate(o3d::Double timestamp, const TickArray &ticks) = 0;

    /**
     * @brief onTicksUpdate Update from the market data at each new Ohlc data.
     * @param timestamp Current timestamp.
     * @param timeframe Timeframe of the Ohlc.
     * @param ohlcType Ohlc type (mid, bid, ask).
     * Do here the iteration data preparation (OHLC, ticks array, and other any data sources)
     * that need to be ready for the process method.
     */
    virtual void onOhlcUpdate(o3d::Double timestamp,
                              o3d::Double timeframe,
                              Ohlc::Type ohlcType,
                              const OhlcArray &ohlc) = 0;

    /**
     * @brief onTicksUpdate Update from the market data at each order book update (insert, update, remove).
     * @param timestamp Current timestamp.
     * @param orderBook @todo
     * Do here the storage of the order bookr for the strategy.
     */
    // virtual void onOrderBookUpdate(o3d::Double timestamp, o3d::Double timeframe, const OrderBook &orderBook) = 0;

    /**
     * @brief onOrderSignal Insert/update/delete an order on the strategy.
     */
    virtual void onOrderSignal(const OrderSignal &orderSignal) = 0;

    /**
     * @brief onPositionSignal Insert/update/delete a position on the strategy.
     */
    virtual void onPositionSignal(const PositionSignal &positionSignal) = 0;

    /**
     * @brief prepare Prepare data before each process.
     * @param timestamp Current timestamp.
     * Do here the iteration data preparation that need to be ready for the compute method.
     */
    virtual void prepare(o3d::Double timestamp) = 0;

    /**
     * @brief process Computation processing.
     * @param timestamp Current timestamp.
     * Do here the strategy computations, orders execution, cancelation, and trade updates.
     */
    virtual void compute(o3d::Double timestamp) = 0;

    /**
     * @brief finalize Post computation processing.
     * @param timestamp Current timestamp.
     * Do here some cleanup,
     */
    virtual void finalize(o3d::Double timestamp) = 0;

    /**
     * @brief processing Return true if the strategy is currently processing.
     */
    inline o3d::Bool processing() const { return m_processing; }

    /**
     * @brief running Return true if the strategy is in running state.
     */
    inline o3d::Bool running() const { return m_curState == STATE_RUNNING; }

    /**
     * @brief updateTrade Update a trade state (adjustment of stop, limit, breakeven, timeout...)
     * @param trade A valid trade related to the strategy.
     */
    virtual void updateTrade(Trade *trade) = 0;

    /**
     * @brief log Log a message throught the message logger of the handler.
     * @param unit Bar model (timeframe or non-temporal bar) related to the message or en empty string.
     * @param channel Mapped name of the channel.
     * @param msg Message content.
     */
    void log(const o3d::String &unit, const o3d::String &channel, const o3d::String &msg,
             o3d::System::MessageLevel type = o3d::System::MSG_INFO);

    //
    // accessors
    //

    /**
     * @brief handler Related handler (read only).
     */
    const Handler* handler() const { return m_handler; }

    /**
     * @brief handler Related handler.
     */
    Handler* handler() { return m_handler; }

    /**
     * @brief market Related market details (read only).
     */
    const Market* market() const { return m_market; }

    /**
     * @brief market Related market details.
     */
    Market* market() { return m_market; }

    o3d::Bool reversal() const { return m_reversal; }
    o3d::Bool hedging() const { return m_hedging; }
    o3d::Bool allowShort() const { return m_allowShort; }
    o3d::Int32 maxTrades() const { return m_maxTrades; }
    o3d::Double tradeDelay() const { return m_tradeDelay; }
    o3d::Bool needUpdate() const { return m_needUpdate; }
    o3d::Double baseTimeframe() const { return m_baseTimeframe; }

    /**
     * @brief tradeType Strategy trade type to instanciate.
     */
    Trade::Type tradeType() { return m_tradeType; }

    /**
     * @brief lastTimestamp Last performed timestamp.
     */
    o3d::Double lastTimestamp() const { return m_lastTimestamp; }

    /**
     * @brief statistics per strategy statistics.
     */
    const Statistics& statistics() const { return m_stats; }

    /**
     * @brief updateStats Update max drawn down and some others stuffs.
     */
    virtual void updateStats() = 0;

    /**
     * @brief addClosedTrade Add a terminated trade (close or canceled) to statistics.
     */
    void addClosedTrade(Trade *trade);

    //
    // trading sessions
    //


    /**
     * @brief timezone Market timezone.
     */
    o3d::Double timezone() const { return m_timezone; }

    /**
     * @brief sessionOffset Market trading sessions offset from UTC 00:00 in seconds.
     * @return
     */
    o3d::Double sessionOffset() const { return m_sessionOffset; }

    /**
     * @brief sessionDuration Market trading sessions duration from session offset in seconds.
     * @return
     */
    o3d::Double sessionDuration() const { return m_sessionOffset; }

    /**
     * @brief hasTradingSessions True if one or more trading sessions are defined.
     */
    o3d::Bool hasTradingSessions() const { return m_tradingSessions.size() > 0; }

    /**
     * @brief tradingSessions Empty vector where each tuple is three values for day of week, hour of day, minute of day.
     */
    const std::vector<TradingSession> tradingSessions() const { return m_tradingSessions; }

    /**
     * @brief allowedTradingSession Return true if sessions allow trading else false.
     */
    o3d::Bool allowedTradingSession(o3d::Double timestamp) const;

    /**
     * @brief adjustHistoryDuration According to the type of market increase the history duration when target date
     *   and history starts during an off day, off hour, have off day or off hours.
     * @param history
     * @param toTs
     * @return
     * @note It does not support nationals days off because need special calendar (Panda could offers that)
     */
    o3d::Double adjustHistoryDuration(o3d::Double history, o3d::Double toTs) const;

protected:

    void setProperty(const o3d::String propertyName, const o3d::String value);

    //
    // data sources (input stream)
    //

    void addTickDataSource();
    void addMidOhlcDataSource(o3d::Double timeframe);
    void addBidOhlcDataSource(o3d::Double timeframe);
    void addAskOhlcDataSource(o3d::Double timeframe);
    void addOrderBookDataSource(o3d::Int32 depth);

    void adjustOhlcFetchRange(o3d::Double history, o3d::Int32 depth,
                              o3d::Double &fromTs, o3d::Double &toTs,
                              o3d::Int32 &nLast) const;

    //
    // state
    //

    State state() const { return m_curState; }

    void setInitialized();
    void setMarketDataPrepared();
    void setReady();
    void setRunning();
    void setTerminated();

    void initBasicsParameters(StrategyConfig &conf);

    void setActiveStats(o3d::Double performance,
                        o3d::Double drawDownRate,
                        o3d::Double drawDown,
                        o3d::Int32 pending,
                        o3d::Int32 actives);

    void setTimezone(o3d::Double tz);
    void setSessionOffset(o3d::Double offset);
    void setSessionDuration(o3d::Double duration);
    void addTradingSession(o3d::Int8 dayOfWeek, o3d::Double fromTime, o3d::Double toTime);

private:

    Handler *m_handler;

    o3d::String m_identifier;
    o3d::String m_brokerId;

    o3d::FastMutex m_mutex;

    State m_curState;
    State m_nextState;

    Statistics m_stats;

    o3d::StringMap<o3d::String> m_properties;
    std::list<DataSource> m_dataSources;

    Market *m_market;

    o3d::Double m_lastTimestamp;
    o3d::Bool m_processing;

    o3d::Bool m_reversal;
    o3d::Bool m_hedging;
    o3d::Bool m_allowShort;
    o3d::Int32 m_maxTrades;
    o3d::Double m_tradeDelay;
    o3d::Bool m_needUpdate;

    o3d::Double m_baseTimeframe;

    Trade::Type m_tradeType;

    o3d::Double m_baseQuantity;

    o3d::Double m_timezone;         //!< market timezone UTC+N
    o3d::Double m_sessionOffset;    //!< day session offset from 00:00 in seconds
    o3d::Double m_sessionDuration;  //!< day session duration from session offset in seconds

    //! allowed trading session (empty mean anytime) else must be explicit. each session is a TradingSession model.
    std::vector<TradingSession> m_tradingSessions;
};

} // namespace siis

#endif // SIIS_STRATEGY_H
