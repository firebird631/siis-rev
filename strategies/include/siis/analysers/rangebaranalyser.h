/**
 * @brief SiiS strategy analyser for a range-bar series
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-12
 */

#ifndef SIIS_RANGEBARANALYSER_H
#define SIIS_RANGEBARANALYSER_H

#include "analyser.h"

#include "../utils/rangeohlcgen.h"
#include "../indicators/price/price.h"
#include "../indicators/volume/volume.h"

namespace siis {

/**
 * @brief Strategy analyser for a range-bar serie.
 * @author Frederic Scherma
 * @date 2024-07-12
 * Standard implementation works on a specific range-bar size, on the last price and volume data coming
 * from the market OHLC.
 * Price and volume indicators are implemented and only the compute method has to be overrided.
 * @note Does not work with input array of OHLC but only from a source of ticks.
 */
class SIIS_API RangeBarAnalyser : public Analyser
{
public:

    RangeBarAnalyser(
        Strategy *strategy,
        o3d::Int32 rangeSize,
        o3d::Int32 depth,
        o3d::Int32 history,
        Price::Method priceMethod=Price::PRICE_CLOSE,
        o3d::Double tickScale=1.0);

    virtual ~RangeBarAnalyser() override;

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

    RangeOhlcGen m_ohlcGen;
    OhlcCircular m_ohlc;

    Price m_price;
    Volume m_volume;
};

} // namespace siis

#endif // SIIS_RANGEBARANALYSER_H
