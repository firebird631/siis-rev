/**
 * @brief SiiS forexalpha strategy base analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_FAANALYSER_H
#define SIIS_FAANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"
#include "siis/trade/tradesignal.h"

namespace siis {

/**
 * @brief SiiS forexalpha strategy A analyser.
 * @author Frederic Scherma
 * @date 2019-03-19
 * Base model.
 */
class SIIS_API FaAnalyser : public TimeframeBarAnalyser
{
public:

    FaAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~FaAnalyser() override;

    TradeSignal lastSignal() const;

protected:

    TradeSignal m_lastSignal;
};

} // namespace siis

#endif // SIIS_FAANALYSER_H
