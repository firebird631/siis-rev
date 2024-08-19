/**
 * @brief SiiS MAADX strategy profile analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#ifndef SIIS_MAADXPROFILEANALYSER_H
#define SIIS_MAADXPROFILEANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"

#include "siis/indicators/imbalance/barimbalance.h"

namespace siis {

/**
 * @brief SiiS MAADX strategy profile analyser.
 * @author Frederic Scherma
 * @date 2024-08-18
 * Profile analyser
 */
class SIIS_API MaAdxProfileAnalyser : public TimeframeBarAnalyser
{
public:

    MaAdxProfileAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~MaAdxProfileAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline const BarImbalance& imbalance() const { return m_imbalance; }

private:

    BarImbalance m_imbalance;
};

} // namespace siis

#endif // SIIS_MAADXPROFILEANALYSER_H
