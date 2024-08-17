 /**
 * @brief SiiS tick volume profile indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-02
 */

#ifndef SIIS_VOLUMEPROFILE_H
#define SIIS_VOLUMEPROFILE_H

#include "volumeprofiledata.h"
#include "../../tick.h"

#include "../indicator.h"
#include "../../dataarray.h"

#include <deque>

namespace siis {

/**
 * @brief SiiS tick volume profile indicator.
 * @author Frederic Scherma
 * @date 2024-08-02
 */
class SIIS_API VolumeProfile : public Indicator
{
public:

    typedef std::vector<std::pair<o3d::Double, std::pair<o3d::Double, o3d::Double>>> T_VolumeByPrice;
    typedef T_VolumeByPrice::iterator IT_VolumeByPrice;

    // TYPE_VOLUME
    // CLS_CUMULATIVE

    /**
     * @brief VolumeProfile
     * @param name
     * @param timeframe Related bar timeframe or 0
     * @param historySize min 1
     * @param sensibility bin width
     * @param valueAreaSize In percent ]0..100]
     * @param computePeaksAndValleys Auto compute at finalize (default false)
     * @param tickScale Supplementary tick scalar to adjust price computation
     * @param sessionFilter If defined, ticks received out of the session are ignored
     */
    VolumeProfile(const o3d::String &name, o3d::Double timeframe,
                  o3d::Int32 historySize=10,
                  o3d::Double sensibility=1.0,
                  o3d::Double valueAreaSize=70.0,
                  o3d::Bool computePeaksAndValleys=false,
                  o3d::Double tickScale=1.0,
                  o3d::Bool sessionFilter=false);

    VolumeProfile(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf);

    ~VolumeProfile();

    void setConf(IndicatorConfig conf);

    /**
     * @brief init Initialize from instrument price limit.
     * @param pricePrecision 1 or greater decimal place else default to 8
     * @param tickSize Tick size (price limit step) positive value or default to 0.00000001
     */
    void init(o3d::Int32 pricePrecision, o3d::Double tickSize);

    void setSession(o3d::Double sessionOffset, o3d::Double sessionDuration);

    o3d::Int32 historySize() const { return m_historySize; }
    o3d::Double sensibility() const { return m_sensibility; }
    o3d::Double valueAreaSize() const { return m_valueAreaSize; }
    o3d::Bool isComputeValueArea() const { return m_valueAreaSize > 0 && m_valueAreaSize <= 100; }
    o3d::Bool isComputePeaksAndValleys() const { return m_computePeaksAndValleys; }
    o3d::Bool hasSessionFilter() const { return m_sessionFilter; }

    o3d::Bool hasValues() const { return !m_vp.empty(); }

    const std::deque<VolumeProfileData*> vp() const { return m_vp; }

    o3d::Bool hasCurrent() const { return m_pCurrent != nullptr; }

    const VolumeProfileData* current() const { return m_pCurrent; }

    /**
     * @brief adjustPrice Adjust give price according to price precision, tick size and scale.
     */
    o3d::Double adjustPrice(o3d::Double price) const;

    /**
     * @brief previous A previous volume profile
     * @param at Negative index or positive works
     * @return
     * @exception IndexOutOfRange
     */
    const VolumeProfileData* previous(o3d::Int32 n) const;

    /**
     * @brief pocPrice
     * @param n 0 for curreny profile, else positive or negative index base (@see previous)
     */
    o3d::Double pocPrice(o3d::Int32 n=0) const;

    /**
     * @brief valPrice
     * @param n 0 for curreny profile, else positive or negative index base (@see previous)
     * @return
     */
    o3d::Double valPrice(o3d::Int32 n=0) const;

    /**
     * @brief vahPrice
     * @param n 0 for curreny profile, else positive or negative index base (@see previous)
     * @return
     */
    o3d::Double vahPrice(o3d::Int32 n=0) const;

    /**
     * @brief compute Compute a volume profile per tick.
     * @param finalize Once the related bar closed it must finalize the current profile.
     */
    void update(const Tick &tick, o3d::Bool finalize=false);

    void updateValueArea();
    void updatePeaksAndValleys();

    void finalize();

    void computeVolumeByPrices(T_VolumeByPrice &outVolumeByPrices) const;

    std::pair<o3d::Double, o3d::Double> computeValueArea(const T_VolumeByPrice& volumeByPrices,
                                                         o3d::Double pocPrice,
                                                         o3d::Double valueAreaSize=70.0) const;

    void findPeaksAndValleys(const T_VolumeByPrice& volumeByPrices,
                             std::vector<o3d::Double> &outPeaks,
                             std::vector<o3d::Double> &outValleys) const;

private:

    o3d::Double m_sessionOffset;     //!< 0 means starts at 00:00 UTC
    o3d::Double m_sessionDuration;   //!< 0 means full day

    o3d::Double m_tickSize;          //!< from market data
    o3d::Int32 m_pricePrecision;     //!< from market data

    o3d::Int32 m_historySize;
    o3d::Double m_sensibility;

    o3d::Double m_valueAreaSize;
    o3d::Bool m_computePeaksAndValleys;

    o3d::Double m_tickScale;
    o3d::Bool m_sessionFilter;

    VolumeProfileData *m_pCurrent;

    o3d::Double m_currentMinPrice;
    o3d::Double m_currentMaxPrice;

    o3d::Bool m_consolidated;

    std::deque<VolumeProfileData*> m_vp;

    void createVolumeProfile(o3d::Double timestamp, o3d::Double price);
    o3d::Double basePrice(o3d::Double price) const;

    VolumeProfileData::IT_BinHashMap setAt(o3d::Double price);
    void updatePoc(VolumeProfileData::IT_BinHashMap basePriceIt);

    void addAsk(o3d::Double price, o3d::Double volume);
    void addBid(o3d::Double price, o3d::Double volume);

    /**
     * @brief addBoth
     * @param price
     * @param volume 50% set on bid, 50% set on ask
     */
    void addBoth(o3d::Double price, o3d::Double volume);
};

} // namespace siis

#endif // SIIS_TICKVOLUMEPROFILE_H
