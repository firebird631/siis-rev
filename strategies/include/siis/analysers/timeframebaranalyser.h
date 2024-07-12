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
 * Standard implementation works on a specific timeframe, on the last price and volume data coming
 * from the market OHLC.
 * Price and volume indicators are implemented and only the compute method has to be overrided.
 */
class SIIS_API TimeframeBarAnalyser : public Analyser
{
public:

    TimeframeBarAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~TimeframeBarAnalyser() override;

    virtual void onTickUpdate(o3d::Double timestamp, const TickArray &ticks) override;
    virtual void onOhlcUpdate(o3d::Double timestamp, o3d::Double timeframe, const OhlcArray &ohlc) override;

    virtual void init(AnalyserConfig conf) override;
    virtual void prepare(o3d::Double timestamp) override;

    virtual o3d::Bool process(o3d::Double timestamp, o3d::Double lastTimestamp) override;
    virtual TradeSignal compute(o3d::Double timestamp, o3d::Double lastTimestamp) = 0;

    virtual o3d::Double lastPrice() const override;

protected:

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
