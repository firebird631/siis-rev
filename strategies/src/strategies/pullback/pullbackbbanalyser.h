/**
 * @brief SiiS Pullback strategy bollinger analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#ifndef SIIS_PULLBACKBBANALYSER_H
#define SIIS_PULLBACKBBANALYSER_H

#include "siis/analysers/stdanalyser.h"

#include "siis/indicators/bbands/bbands.h"

namespace siis {

/**
 * @brief SiiS Pullback strategy bollinger analyser.
 * @author Frederic Scherma
 * @date 2023-05-09
 * Bollinger analyser
 */
class SIIS_API PullbackBBAnalyser : public StdAnalyser
{
public:

    PullbackBBAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~PullbackBBAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual TradeSignal compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 trend() const { return m_trend; }
    inline o3d::Int32 sig() const { return m_sig; }

    o3d::Double takeProfit(o3d::Double profitScale) const;
    o3d::Double stopLoss(o3d::Double lossScale, o3d::Double riskReward) const;

private:

    BBands m_bollinger;

    o3d::Int32 m_trend;
    o3d::Int32 m_sig;

    o3d::Int32 m_confirmation;
};

} // namespace siis

#endif // SIIS_PULLBACKBBANALYSER_H
