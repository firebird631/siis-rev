/**
 * @brief SiiS MAADX range-bar strategy session analyser.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-09
 */

#ifndef SIIS_MAADXRBSESSIONANALYSER_H
#define SIIS_MAADXRBSESSIONANALYSER_H

#include "siis/analysers/rangebaranalyser.h"

#include "siis/indicators/volumeprofile/volumeprofile.h"
#include "siis/indicators/volumeprofile/compositevolumeprofile.h"
#include "siis/indicators/bollinger/bollinger.h"
#include "siis/indicators/wma/wma.h"

#include "siis/datacircular.h"

namespace siis {

/**
 * @brief SiiS MAADX range-bar strategy session analyser.
 * @author Frederic Scherma
 * @date 2024-08-09
 * Session analyser
 */
class SIIS_API MaAdxRbSessionAnalyser : public RangeBarAnalyser
{
public:

    MaAdxRbSessionAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Int32 rangeSize,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod=Price::PRICE_CLOSE);

    virtual ~MaAdxRbSessionAnalyser() override;

    virtual o3d::String typeName() const override;

    virtual void init(const AnalyserConfig &conf) override;
    virtual void terminate() override;
    virtual void compute(o3d::Double timestamp, o3d::Double lastTimestamp) override;
    virtual void updateTick(const Tick& tick, o3d::Bool finalize) override;

    inline const VolumeProfile& vp() const { return m_vp; }

    inline o3d::Int32 vPocBreakout() const { return m_vPocBreakout; }
    inline o3d::Int32 vPocTrend() const { return m_vPocTrend; }
    inline o3d::Int32 vpCross() const { return m_vpCross; }

private:

    VolumeProfile m_vp;
    Bollinger m_vpoc_bollinger;
    CompositeVolumeProfile m_compositeVP;
    Wma m_ma;

    o3d::Int32 m_vPocBreakout;
    o3d::Int32 m_vPocTrend;
    o3d::Int32 m_vpCross;

    DataCircular m_vPocs;
};

} // namespace siis

#endif // SIIS_MAADXRBSESSIONANALYSER_H
