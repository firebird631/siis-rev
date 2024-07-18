/**
 * @brief SiiS indicealpha strategy base analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_IAANALYSER_H
#define SIIS_IAANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"
#include "siis/trade/tradesignal.h"

namespace siis {

/**
 * @brief SiiS indicealpha strategy base analyser.
 * @author Frederic Scherma
 * @date 2019-03-19
 * Base model.
 */
class SIIS_API IaAnalyser : public TimeframeBarAnalyser
{
public:

    IaAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~IaAnalyser() override;

    virtual o3d::String typeName() const override;

    TradeSignal lastSignal() const;

protected:

    TradeSignal m_lastSignal;
};

} // namespace siis

#endif // SIIS_IAANALYSER_H