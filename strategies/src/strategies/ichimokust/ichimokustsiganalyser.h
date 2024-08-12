/**
 * @brief SiiS Ichimoku strategy signal analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#ifndef SIIS_ICHIMOKUSTSIGANALYSER_H
#define SIIS_ICHIMOKUSTSIGANALYSER_H

#include "siis/analysers/timeframebaranalyser.h"
#include "siis/indicators/ichimoku/ichimoku.h"

namespace siis {

/**
 * @brief SiiS Ichimoku strategy confirmation analyser.
 * @author Frederic Scherma
 * @date 2024-07-15
 * Signal analyser
 */
class SIIS_API IchimokuStSigAnalyser : public TimeframeBarAnalyser
{
public:

    IchimokuStSigAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~IchimokuStSigAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;

    inline o3d::Int32 priceCross() const { return m_priceCross; }
    inline o3d::Int32 priceDir() const { return m_priceDir; }

    inline o3d::Int32 chikouCross() const { return m_chikouCross; }
    inline o3d::Int32 chikouDir() const { return m_chikouDir; }

    inline o3d::Int32 tenkanDir() const { return m_tenkanDir; }
    inline o3d::Int32 cloudDir() const { return m_cloudDir; }

    o3d::Double stopPrice(o3d::Int32 direction) const;

private:

    Ichimoku m_ichimoku;

    o3d::Int32 m_span_cross_ofs;
    o3d::Int32 m_chikou_ofs;

    o3d::Int32 m_priceCross;
    o3d::Int32 m_priceDir;

    o3d::Int32 m_chikouCross;
    o3d::Int32 m_chikouDir;

    o3d::Int32 m_tenkanDir;
    o3d::Int32 m_cloudDir;
};

} // namespace siis

#endif // SIIS_ICHIMOKUSTSIGANALYSER_H
