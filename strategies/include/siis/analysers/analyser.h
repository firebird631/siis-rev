/**
 * @brief SiiS strategy analyser per timeframe.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_ANALYSER_H
#define SIIS_ANALYSER_H

#include "../tick.h"
#include "../ohlc.h"

namespace siis {

class Strategy;
class Market;
class AnalyserConfig;

/**
 * @brief Strategy analyser per bar
 * @author Frederic Scherma
 * @date 2019-03-15
 * @todo replace virtual by a template Analyser<FaaAnalyser> ...
 * std::unique_ptr<...> return std::make_unique<...<..>>()
 * @todo remove subTimeframe
 */
class SIIS_API Analyser
{
public:

    Analyser(Strategy *strategy, o3d::Double timeframe, o3d::Double subTimeframe,
             o3d::Int32 barSize,
             o3d::Int32 depth, o3d::Int32 history);

    virtual ~Analyser();

    /**
     * @brief init Initialize using strategy anaylser configuration.
     */
    virtual void init(AnalyserConfig conf) = 0;

    /**
     * @brief init Terminate the analyser.
     */
    virtual void terminate() = 0;

    /**
     * @brief prepare Prepare data before processing.
     * @param timestamp Current timestamp (must be >= next timestamp)
     */
    virtual void prepare(o3d::Double timestamp) = 0;

    /**
     * @brief onTickUpdate On new ticks are received, update the price/volume data to be ready for the next process.
     * @param timestamp Current timestamp.
     */
    virtual void onTickUpdate(o3d::Double timestamp, const TickArray &ticks) = 0;

    /**
     * @brief onOhlcUpdate On new ohlc are received, update the price/volume data to be ready for the next process.
     * @param timestamp Current timestamp.
     */
    virtual void onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc) = 0;

    /**
     * @brief process Process the analyse.
     * @param timestamp Current timestamp (must be >= next timestamp)
     */
    virtual void process(o3d::Double timestamp, o3d::Double lastTimestamp) = 0;

    /**
     * @brief isNeedUpdate If timestamp >= next timestamp
     */
    virtual o3d::Bool isNeedUpdate(o3d::Double timestamp) const;

    /**
     * @brief lastPrice Last updated price (last close price).
     */
    virtual o3d::Double lastPrice() const = 0;

    const Strategy* strategy() const { return m_strategy; }
    Strategy* strategy() { return m_strategy; }

    o3d::Double timeframe() const { return m_timeframe; }
    o3d::Double tf() const { return m_timeframe; }

    o3d::Double subTimeframe() const { return m_subTimeFrame; }
    o3d::Double subTf() const { return m_subTimeFrame; }

    o3d::Double barSize() const { return m_barSize; }

    o3d::Int32 depth() const { return m_depth; }
    o3d::Int32 history() const { return m_history; }

    o3d::Double nextTimestamp() const { return m_nextTimestamp; }
    o3d::Double nextTs() const { return m_nextTimestamp; }

    /**
     * @brief log Log a message throught the message logger of the strategy.
     * @param channel Mapped name of the channel.
     * @param msg Message content.
     */
    void log(const o3d::String &channel, const o3d::String &msg);

protected:

    void processCompleted(o3d::Double timestamp) { m_nextTimestamp = timestamp; }

private:

    Strategy *m_strategy;

    o3d::Double m_timeframe;     //!< timeframe or bar size
    o3d::Double m_subTimeFrame;

    o3d::Int32 m_barSize;        //!< bar size or timeframe

    o3d::Int32 m_depth;
    o3d::Int32 m_history;

    o3d::Double m_nextTimestamp;
};

} // namespace siis

#endif // SIIS_ANALYSER_H
