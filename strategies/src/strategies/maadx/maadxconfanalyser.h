/**
 * @brief SiiS MAADX strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_MAADXCONFANALYSER_H
#define SIIS_MAADXCONFANALYSER_H

#include "siis/analysers/stdanalyser.h"

#include "siis/indicators/hma/hma.h"
#include "siis/indicators/adx/adx.h"


namespace siis {

/**
 * @brief SiiS MAADX strategy confirmation analyser.
 * @author Frederic Scherma
 * @date 2023-04-24
 * Confirmation analyser
 */
class SIIS_API MaAdxConfAnalyser : public StdAnalyser
{
public:

    MaAdxConfAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~MaAdxConfAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual TradeSignal compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

private:

};

} // namespace siis

#endif // SIIS_MAADXCONFANALYSER_H