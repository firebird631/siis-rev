/**
 * @brief SiiS strategy analyser per timeframe.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_ANALYSER_H
#define SIIS_ANALYSER_H

#include "../trade/tradesignal.h"
#include "../tick.h"
#include "../ohlc.h"

namespace siis {

class Strategy;
class Market;
class AnalyserConfig;

/**
 * @brief Strategy analyser per timeframe
 * @author Frederic Scherma
 * @date 2019-03-15
 * @todo replace virtual by a template Analyser<FaaAnalyser> ...
 * std::unique_ptr<...> return std::make_unique<...<..>>()
 */
class SIIS_API Analyser
{
public:

    Analyser(Strategy *strategy, o3d::Double timeframe, o3d::Double subTimeframe, o3d::Int32 depth, o3d::Int32 history);
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
     * @brief onTickUpdate On new ohlc are received, update the price/volume data to be ready for the next process.
     * @param timestamp Current timestamp.
     */
    virtual void onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc) = 0;

    /**
     * @brief process Process the analyse.
     * @param timestamp Current timestamp (must be >= next timestamp)
     * @return True if a new signal was emitted.
     */
    virtual o3d::Bool process(o3d::Double timestamp, o3d::Double lastTimestamp) = 0;

    /**
     * @brief exportConditions Export the condition of the last generated signal.
     * @param conditions A Conditions object where to push the analyser specifics conditions.
     * @todo Conditions and implementation
     */
    // virtual void exportConditions(Conditions &conditions) = 0;

    /**
     * @brief isNeedUpdate If timestamp >= next timestamp
     */
    virtual o3d::Bool isNeedUpdate(o3d::Double timestamp) const;

    const Strategy* strategy() const { return m_strategy; }
    Strategy* strategy() { return m_strategy; }

    o3d::Double timeframe() const { return m_timeframe; }
    o3d::Double tf() const { return m_timeframe; }

    o3d::Double subTimeframe() const { return m_subTimeFrame; }
    o3d::Double subTf() const { return m_subTimeFrame; }

    o3d::Int32 depth() const { return m_depth; }
    o3d::Int32 history() const { return m_history; }

    o3d::Double nextTimestamp() const { return m_nextTimestamp; }
    o3d::Double nextTs() const { return m_nextTimestamp; }

    o3d::Bool hasLastSignal() const { return m_lastSignal.type() != TradeSignal::NONE; }
    const TradeSignal& lastSignal() const { return m_lastSignal; }

    /**
     * @brief log Log a message throught the message logger of the strategy.
     * @param channel Mapped name of the channel.
     * @param msg Message content.
     */
    void log(const o3d::String &channel, const o3d::String &msg);

protected:

    void processCompleted(o3d::Double timestamp) { m_nextTimestamp = timestamp; }

    void signalCompleted(const TradeSignal &tradeSignal) { m_lastSignal = tradeSignal; }
    void clearSignal() { m_lastSignal = TradeSignal(m_timeframe, 0); }

private:

    Strategy *m_strategy;

    o3d::Double m_timeframe;
    o3d::Double m_subTimeFrame;

    o3d::Int32 m_depth;
    o3d::Int32 m_history;

    o3d::Double m_nextTimestamp;

    TradeSignal m_lastSignal;
};

} // namespace siis

#endif // SIIS_ANALYSER_H
