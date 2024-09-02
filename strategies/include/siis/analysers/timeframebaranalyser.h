/**
 * @brief SiiS strategy analyser for a timeframe bar serie.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_TIMEFRAMEBARANALYSER_H
#define SIIS_TIMEFRAMEBARANALYSER_H

#include "analyser.h"

#include "../utils/timeframeohlcgen.h"
#include "../indicators/price/price.h"
#include "../indicators/volume/volume.h"

namespace siis {

/**
 * @brief Strategy analyser for a timeframe bar serie.
 * @author Frederic Scherma
 * @date 2019-03-16
 * Compute a bar serie for a specific timefram and process different indicators and more.
 * Price and volume indicators are implemented and only the compute method has to be overrided.
 */
class SIIS_API TimeframeBarAnalyser : public Analyser
{
public:

    TimeframeBarAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Double history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~TimeframeBarAnalyser() override;

    virtual void onTickUpdate(o3d::Double timestamp, const TickArray &ticks) override;
    virtual void onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc) override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void prepare(o3d::Double timestamp) override;

    virtual void process(o3d::Double timestamp, o3d::Double lastTimestamp) override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) = 0;

    virtual o3d::Double lastPrice() const override;

    virtual o3d::String formatUnit() const override;

    o3d::Double sourceTimeframe() const;

    inline const Price& price() const { return m_price; }
    inline const Volume& volume() const { return m_volume; }

private:

    TimeframeOhlcGen m_ohlcGen;
    OhlcCircular m_ohlc;

    Price m_price;
    Volume m_volume;
};

} // namespace siis

#endif // SIIS_TIMEFRAMEBARANALYSER_H
