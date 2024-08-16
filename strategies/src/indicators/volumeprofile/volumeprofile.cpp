 /**
 * @brief SiiS tick volume profile indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-02
 */

#include "siis/indicators/volumeprofile/volumeprofile.h"
#include "siis/utils/common.h"
#include "siis/utils/math.h"

#include <vector>
#include <algorithm>

using namespace siis;
using o3d::Logger;
using o3d::Debug;


VolumeProfile::VolumeProfile(const o3d::String &name,
                             o3d::Double timeframe,
                             o3d::Int32 historySize,
                             o3d::Double sensibility,
                             o3d::Double valueAreaSize,
                             o3d::Bool computePeaksAndValleys,
                             o3d::Double tickScale) :
    Indicator(name, timeframe),
    m_sessionOffset(0.0),
    m_sessionDuration(0.0),
    m_tickSize(1.0),
    m_pricePrecision(1),
    m_historySize(historySize),
    m_sensibility(sensibility),
    m_valueAreaSize(valueAreaSize),
    m_computePeaksAndValleys(computePeaksAndValleys),
    m_tickScale(tickScale),
    m_pCurrent(nullptr),
    m_currentMinPrice(0.0),
    m_currentMaxPrice(0.0),
    m_consolidated(false)
{

}

VolumeProfile::VolumeProfile(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_sessionOffset(0.0),
    m_sessionDuration(0.0),
    m_tickSize(1.0),
    m_pricePrecision(1),
    m_historySize(10),
    m_sensibility(1.0),
    m_valueAreaSize(70.0),
    m_computePeaksAndValleys(false),
    m_tickScale(1.0),
    m_pCurrent(nullptr),
    m_currentMinPrice(0.0),
    m_currentMaxPrice(0.0),
    m_consolidated(false)
{
    if (conf.data().isObject()) {
        m_historySize = conf.data().get("history", 10).asInt();
        m_sensibility = conf.data().get("sensibility", 1.0).asDouble();
        m_valueAreaSize = conf.data().get("value-area", 70.0).asDouble();
        m_computePeaksAndValleys = conf.data().get("peaks-and-valleys", false).asBool();
        m_tickScale = conf.data().get("tick-scale", 1.0).asDouble();
    } else if (conf.data().isArray()) {
        m_historySize = conf.data().get((Json::ArrayIndex)1, 10).asInt();
        m_sensibility = conf.data().get((Json::ArrayIndex)2, 1.0).asDouble();
        m_valueAreaSize = conf.data().get((Json::ArrayIndex)3, 70.0).asDouble();
        m_computePeaksAndValleys = conf.data().get((Json::ArrayIndex)4, false).asBool();
        m_tickScale = conf.data().get((Json::ArrayIndex)5, 1.0).asDouble();
    }
}

VolumeProfile::~VolumeProfile()
{
    if (m_pCurrent) {
        o3d::deletePtr(m_pCurrent);
    }

    for (auto vp : m_vp) {
        o3d::deletePtr(vp);
    }
}

void VolumeProfile::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_historySize = conf.data().get("history", 10).asInt();
        m_sensibility = conf.data().get("sensibility", 1.0).asDouble();
        m_valueAreaSize = conf.data().get("value-area", 70.0).asDouble();
        m_computePeaksAndValleys = conf.data().get("peaks-and-valleys", false).asBool();
        m_tickScale = conf.data().get("tick-scale", 1.0).asDouble();
    } else if (conf.data().isArray()) {
        m_historySize = conf.data().get((Json::ArrayIndex)1, 10).asInt();
        m_sensibility = conf.data().get((Json::ArrayIndex)2, 1.0).asDouble();
        m_valueAreaSize = conf.data().get((Json::ArrayIndex)3, 70.0).asDouble();
        m_computePeaksAndValleys = conf.data().get((Json::ArrayIndex)4, false).asBool();
        m_tickScale = conf.data().get((Json::ArrayIndex)5, 1.0).asDouble();
    }
}

void VolumeProfile::init(o3d::Int32 pricePrecision, o3d::Double tickSize)
{
    if (pricePrecision == 0) {
        pricePrecision = 8;
    }

    if (tickSize == 0) {
        tickSize = 0.00000001;
    }

    m_pricePrecision = pricePrecision;
    m_tickSize = tickSize * m_tickScale;  // pre-mult
}

void VolumeProfile::setSession(o3d::Double sessionOffset, o3d::Double sessionDuration)
{
    m_sessionOffset = sessionOffset;
    m_sessionDuration = sessionDuration;
}

o3d::Double VolumeProfile::adjustPrice(o3d::Double price) const
{
    // adjusted price at precision and by step of pip meaning
    return truncate(::round(price / m_tickSize) * m_tickSize, m_pricePrecision);
}

const VolumeProfileData* VolumeProfile::previous(o3d::Int32 n) const
{
    o3d::Int32 size = static_cast<o3d::Int32>(m_vp.size());

    if (n >= size) throw o3d::E_IndexOutOfRange("");

    if (n < 0) {
        n = size + n;
        if (n < 0) throw o3d::E_IndexOutOfRange("");
    }

    return m_vp[n];
}

o3d::Double VolumeProfile::pocPrice(o3d::Int32 n) const
{
    if (n > -1) {
        return m_pCurrent != nullptr ? m_pCurrent->pocPrice : 0.0;
    } else {
        return previous(n)->pocPrice;
    }
}

o3d::Double VolumeProfile::valPrice(o3d::Int32 n) const
{
    if (n > -1) {
        return m_pCurrent != nullptr ? m_pCurrent->valPrice : 0.0;
    } else {
        return previous(n)->valPrice;
    }
}

o3d::Double VolumeProfile::vahPrice(o3d::Int32 n) const
{
    if (n > -1) {
        return m_pCurrent != nullptr ? m_pCurrent->vahPrice : 0.0;
    } else {
        return previous(n)->vahPrice;
    }
}

void VolumeProfile::update(const Tick &tick, o3d::Bool finalize)
{

    if (finalize) {
        this->finalize();
    }

    if (m_pCurrent == nullptr) {
        if (timeframe() > 0) {
            createVolumeProfile(baseTime(tick.timestamp(), timeframe()), tick.last());
        } else {
            createVolumeProfile(tick.timestamp(), tick.last());
        }
    }

    if (m_sessionOffset > 0 || m_sessionDuration > 0) {
        o3d::Double basetime = baseTime(TF_DAY, tick.timestamp());

        if (tick.timestamp() < basetime + m_sessionOffset) {
            // ignored, out of session
            return;
        }

        if (tick.timestamp() >= basetime + m_sessionOffset + m_sessionDuration) {
            // ignored, out of session
            return;
        }
    }

    // -1  for bid, 1 for ask, or 0 if no info
    if (tick.buyOrSell() < 0) {
        addBid(tick.last(), tick.volume());
    } else if (tick.buyOrSell() > 0) {
        addAsk(tick.last(), tick.volume());
    } else {
        addBoth(tick.last(), tick.volume());
    }

    // retain the last tick timestamp
    done(tick.timestamp());
}

void VolumeProfile::updateValueArea()
{

}

void VolumeProfile::updatePeaksAndValleys()
{

}

void VolumeProfile::finalize()
{
    if (m_pCurrent == nullptr) {
        return;
    }

    o3d::Double ltimeframe = timeframe() > 0.0 ? timeframe() : (lastTimestamp() - m_pCurrent->timestamp);

    T_VolumeByPrice volumeByPrices;
    computeVolumeByPrices(volumeByPrices);

    if (m_computePeaksAndValleys) {
        if (volumeByPrices.empty()) {
            computeVolumeByPrices(volumeByPrices);
        }

        // @todo
    }

    if (isComputeValueArea()) {
        if (volumeByPrices.empty()) {
            computeVolumeByPrices(volumeByPrices);
        }

        // @todo
    }

    // copy ptr
    m_vp.push_back(m_pCurrent);

    if (static_cast<o3d::Int32>(m_vp.size()) > m_historySize) {
        VolumeProfileData *pOldVP = m_vp.front();
        m_vp.pop_front();

        o3d::deletePtr(pOldVP);
    }

    // force to create a new one
    m_pCurrent = nullptr;
}

void VolumeProfile::computeVolumeByPrices(T_VolumeByPrice &outVolumeByPrices) const
{
    if (m_pCurrent == nullptr) {
        return;
    }

    // @todo
}

std::pair<o3d::Double, o3d::Double> VolumeProfile::computeValueArea(
    const T_VolumeByPrice &volumeByPrices,
    o3d::Double pocPrice,
    o3d::Double valueAreaSize) const
{
    if (volumeByPrices.empty()) {
        return {0, 0};
    }

    o3d::Double supPrice = 0.0;
    o3d::Double infPrice = 0.0;

    // @todo

    return {o3d::min(supPrice, infPrice), o3d::max(supPrice, infPrice)};
}

void VolumeProfile::findPeaksAndValleys(
    const T_VolumeByPrice &volumeByPrices,
    std::vector<o3d::Double> &outPeaks,
    std::vector<o3d::Double> &outValleys) const
{
    // @todo
}

void VolumeProfile::createVolumeProfile(o3d::Double timestamp, o3d::Double price)
{
    O3D_ASSERT(m_pCurrent == nullptr);

    m_pCurrent = new VolumeProfileData;
    m_pCurrent->timestamp = timestamp;
    m_pCurrent->sensibility = m_sensibility;

    o3d::Double adjPrice = adjustPrice(price);
    o3d::Double baseprice = o3d::Int32(adjPrice / m_sensibility) * m_sensibility;

    m_currentMinPrice = m_currentMaxPrice = baseprice;

    // centered base price
    o3d::Double centeredBasePrice = baseprice + m_sensibility * 0.5;

    // initial bin
    m_pCurrent->bins[centeredBasePrice] = std::make_pair(0.0, 0.0);

    // reset state
    m_consolidated = false;
}

o3d::Double VolumeProfile::basePrice(o3d::Double price) const
{
    return o3d::Int32(price / m_sensibility) * m_sensibility;
}

VolumeProfileData::IT_BinHashMap VolumeProfile::setAt(o3d::Double price)
{
    O3D_ASSERT(m_pCurrent != nullptr);

    VolumeProfileData::IT_BinHashMap it;

    o3d::Double adjPrice = adjustPrice(price);
    o3d::Double baseprice = o3d::Int32(adjPrice / m_sensibility) * m_sensibility;

    // centered base price
    o3d::Double centeredBasePrice = baseprice + m_sensibility * 0.5;

    if (baseprice < m_currentMinPrice) {
        m_currentMinPrice = baseprice;
        it = m_pCurrent->bins.insert(std::make_pair(centeredBasePrice, std::pair<o3d::Double, o3d::Double>(0.0, 0.0))).first;
    } else if (baseprice > m_currentMaxPrice) {
        m_currentMaxPrice = baseprice;
        it = m_pCurrent->bins.insert(std::make_pair(centeredBasePrice, std::pair<o3d::Double, o3d::Double>(0.0, 0.0))).first;
    } else if (m_pCurrent->bins.find(centeredBasePrice) == m_pCurrent->bins.end()) {
        // intermediate price;
        it = m_pCurrent->bins.insert(std::make_pair(centeredBasePrice, std::pair<o3d::Double, o3d::Double>(0.0, 0.0))).first;
    } else {
        it = m_pCurrent->bins.find(centeredBasePrice);
    }

    return it;
}

void VolumeProfile::updatePoc(VolumeProfileData::IT_BinHashMap basePriceIt)
{
    O3D_ASSERT(m_pCurrent != nullptr);

    o3d::Double volAtPrice = basePriceIt->second.first + basePriceIt->second.second;
    if (volAtPrice > m_pCurrent->pocVolume) {
        m_pCurrent->pocVolume = volAtPrice;
        m_pCurrent->pocPrice = basePriceIt->first;
    }
}

void VolumeProfile::addAsk(o3d::Double price, o3d::Double volume)
{
    auto it = setAt(price);
    it->second.second += volume;
    updatePoc(it);
}

void VolumeProfile::addBid(o3d::Double price, o3d::Double volume)
{
 auto it = setAt(price);
    it->second.first += volume;
    updatePoc(it);
}

void VolumeProfile::addBoth(o3d::Double price, o3d::Double volume)
{
    auto it = setAt(price);
    it->second.first += volume * 0.5;
    it->second.second += volume * 0.5;
    updatePoc(it);
}
