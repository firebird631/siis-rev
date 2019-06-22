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

namespace siis {

class Handler;
class Connector;
class Database;

/**
 * @brief Strategy base class from which to inherit.
 * @author Frederic Scherma
 * @date 2019-03-05
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

    //
    // preparing
    //

    /**
     * @brief prepareMarket Query market data before the strategy be in running state.
     */
    virtual void prepareMarketData(Connector *connector, Database *db) = 0;

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
     * @param ohlcType Ohlc type (mid, bid, ofr).
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
     * @brief processing Return true if the strategie is currently processing.
     */
    inline o3d::Bool processing() const { return m_processing; }

    /**
     * @brief log Log a message throught the message logger of the handler.
     * @param timeframe Timeframe of the message or -1 if none.
     * @param channel Mapped name of the channel.
     * @param msg Message content.
     */
    void log(o3d::Double timeframe, const o3d::String &channel, const o3d::String &msg);

    //
    // accessors
    //

    /**
     * @brief market Related market details.
     */
    const Market* market() const { return m_market; }

    /**
     * @brief lastTimestamp Last performed timestamp.
     */
    o3d::Double lastTimestamp() const { return m_lastTimestamp; }

protected:

    void setProperty(const o3d::String propertyName, const o3d::String value);

    //
    // data sources (input stream)
    //

    void addTickDataSource();
    void addMidOhlcDataSource(o3d::Double timeframe);
    void addBidOhlcDataSource(o3d::Double timeframe);
    void addOfrOhlcDataSource(o3d::Double timeframe);
    void addOrderBookDataSource(o3d::Int32 depth);

    //
    // state
    //

    State state() const { return m_curState; }

    void setInitialized();
    void setMarketDataPrepared();
    void setReady();
    void setRunning();
    void setTerminated();

private:

    Handler *m_handler;

    o3d::String m_identifier;
    o3d::String m_brokerId;

    o3d::FastMutex m_mutex;

    State m_curState;
    State m_nextState;

    o3d::StringMap<o3d::String> m_properties;
    std::list<DataSource> m_dataSources;

    Market *m_market;

    o3d::Double m_lastTimestamp;
    o3d::Bool m_processing;
};

} // namespace siis

#endif // SIIS_STRATEGY_H
