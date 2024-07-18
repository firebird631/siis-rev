/**
 * @brief SiiS Ichimoku strategy signal range-bar analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#ifndef SIIS_ICHIMOKUSTRBSIGANALYSER_H
#define SIIS_ICHIMOKUSTRBSIGANALYSER_H

#include "siis/analysers/rangebaranalyser.h"
#include "siis/indicators/ichimoku/ichimoku.h"

namespace siis {

/**
 * @brief SiiS Ichimoku strategy confirmation range-bar analyser.
 * @author Frederic Scherma
 * @date 2024-07-15
 * Signal analyser
 */
class SIIS_API IchimokuStRbSigAnalyser : public RangeBarAnalyser
{
public:

    IchimokuStRbSigAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 barSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~IchimokuStRbSigAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(AnalyserConfig conf) override;
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

#endif // SIIS_ICHIMOKUSTRBSIGANALYSER_H
