/**
 * @brief SiiS Pullback strategy bollinger analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#ifndef SIIS_PULLBACKBBANALYSER_H
#define SIIS_PULLBACKBBANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

#include "siis/indicators/bollinger/bollinger.h"

namespace siis {

/**
 * @brief SiiS Pullback strategy bollinger analyser.
 * @author Frederic Scherma
 * @date 2023-05-09
 * Bollinger analyser
 */
class SIIS_API PullbackBBAnalyser : public TimeframeBarAnalyser
{
public:

    PullbackBBAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~PullbackBBAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 breakout() const { return m_breakout; }
    inline o3d::Int32 integrate() const { return m_integrate; }

    inline o3d::Bool isPriceBelowLower() const {
        if (price().last() <= 0.0 || m_bollinger.lower().last() <= 0.0) {
            return false;
        }

        return price().last() < m_bollinger.lower().last();
    }

    inline o3d::Bool isPriceAboveUpper() const {
        if (price().last() <= 0.0 || m_bollinger.upper().last() <= 0.0) {
            return false;
        }

        return price().last() > m_bollinger.upper().last();
    }

    inline o3d::Bool isPriceBelowUpper() const {
        if (price().last() <= 0.0 || m_bollinger.upper().last() <= 0.0) {
            return false;
        }

        return price().last() < m_bollinger.upper().last();
    }

    inline o3d::Bool isPriceAboveLower() const {
        if (price().last() <= 0.0 || m_bollinger.lower().last() <= 0.0) {
            return false;
        }

        return price().last() > m_bollinger.lower().last();
    }

    o3d::Double entryPrice() const;
    o3d::Double takeProfit(o3d::Int32 dir, o3d::Double profitScale) const;
    o3d::Double stopLoss(o3d::Int32 dir, o3d::Double lossScale, o3d::Double riskReward) const;

private:

    BBands m_bollinger;

    o3d::Int32 m_breakout;
    o3d::Int32 m_integrate;

    o3d::Int32 m_confirmation;
};

} // namespace siis

#endif // SIIS_PULLBACKBBANALYSER_H
