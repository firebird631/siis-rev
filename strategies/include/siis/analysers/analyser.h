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
  */
class SIIS_API Analyser
{
public:

    Analyser(Strategy *strategy, const o3d::String &name,
             o3d::Double timeframe, o3d::Int32 barSize,
             o3d::Int32 depth, o3d::Double history);

    virtual ~Analyser();

    /**
     * @brief typeName Internal type name as used into the "mode" fields.
     * @return
     */
    virtual o3d::String typeName() const = 0;

    /**
     * @brief init Initialize using strategy anaylser configuration.
     */
    virtual void init(const AnalyserConfig &conf) = 0;

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
     * @brief updateTick Per tick processer. Called by the bar generator to have in sync indicators.
     * @param tick Last tick to process.
     * @param finalize true when a bar just consolidate.
     */
    virtual void updateTick(const Tick& tick, o3d::Bool finalize);

    /**
     * @brief updateBar Per bar processer. Called by the bar generator to have in sync indicators.
     * @param bar Last bar to process.
     * @param finalize true when a bar just consolidate.
     */
    virtual void updateBar(const Ohlc& bar, o3d::Bool finalize);

    /**
     * @brief isNeedUpdate If timestamp >= next timestamp
     */
    virtual o3d::Bool isNeedUpdate(o3d::Double timestamp) const;

    /**
     * @brief lastPrice Last updated price (last close price).
     */
    virtual o3d::Double lastPrice() const = 0;

    /**
     * @brief formatUnit Return a formatted string about the timeframe or range size.
     */
    virtual o3d::String formatUnit() const = 0;

    /**
     * @brief isComputeAtclose If configured (default is false) it will only to allow indicators computation at bar
     * close.
     * @note This is a free to uses method because it is possible to combine indicator at close with some at each
     * process.
     * There is pros and cons :
     * - pros : faster computation because only compute once over many, filter noise.
     * - cons : could loss wanted crossing during a bar.
     */
    o3d::Bool isUpdateAtclose() const { return m_updateAtClose; }

    /**
     * @brief setUpdateAtClose Default is false. Each analyser must implements his own usage of isUpdateAtClose().
     */
    void setUpdateAtClose(o3d::Bool b) { m_updateAtClose = b; }

    const Strategy* strategy() const { return m_strategy; }
    Strategy* strategy() { return m_strategy; }

    const o3d::String& name() const { return m_name; }

    o3d::Double timeframe() const { return m_timeframe; }
    o3d::Double tf() const { return m_timeframe; }

    o3d::Int32 barSize() const { return m_barSize; }

    o3d::Int32 depth() const { return m_depth; }
    o3d::Double history() const { return m_history; }

    o3d::Double nextTimestamp() const { return m_nextTimestamp; }
    o3d::Double nextTs() const { return m_nextTimestamp; }

    /**
     * @brief numLastBars Number of process bar at the last iteration. 1 mean at least the current bar.
     * More than 1 means it generated (and append) new bars.
     */
    o3d::Int32 numLastBars() const { return m_numLastBars; }

    /**
     * @brief log Log a message throught the message logger of the strategy.
     * @param channel Mapped name of the channel.
     * @param msg Message content.
     */
    void log(const o3d::String &channel, const o3d::String &msg);

protected:

    inline void processCompleted(o3d::Double timestamp) { m_nextTimestamp = timestamp; }

    inline void incNumLastBars(o3d::Int32 num) { m_numLastBars += num; }
    inline void resetNumLastBars() { m_numLastBars = 0; }

private:

    Strategy *m_strategy;

    o3d::String m_name;

    o3d::Double m_timeframe;     //!< timeframe
    o3d::Int32 m_barSize;        //!< bar size

    o3d::Int32 m_depth;
    o3d::Double m_history;

    o3d::Bool m_updateAtClose;

    o3d::Double m_nextTimestamp;
    o3d::Int32 m_numLastBars;
};

} // namespace siis

#endif // SIIS_ANALYSER_H
