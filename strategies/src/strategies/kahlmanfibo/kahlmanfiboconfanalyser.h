 /**
 * @brief SiiS strategy KahlmanFibo strategy confirmation analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#ifndef SIIS_KAHLMANFIBOCONFANALYSER_H
#define SIIS_KAHLMANFIBOCONFANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

namespace siis {

/**
 * @brief SiiS strategy KahlmanFibo strategy confirmation analyser.
 * @author Frederic Scherma
 * @date 2023-08-31
 * Confirmation analyser
 */
class SIIS_API KahlmanFiboConfAnalyser : public TimeframeBarAnalyser
{
public:

    KahlmanFiboConfAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~KahlmanFiboConfAnalyser() override;

    virtual void init(AnalyserConfig conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 confirmation() const { return m_confirmation; }

private:

    o3d::Int32 m_confirmation;
};

} // namespace siis

#endif // SIIS_KAHLMANFIBOCONFANALYSER_H
