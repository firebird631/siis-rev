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
                             o3d::Double tickScale, o3d::Bool sessionFilter) :
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
    m_sessionFilter(sessionFilter),
    m_pCurrent(nullptr),
    m_openTimestamp(0.0),
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
    m_sessionFilter(false),
    m_pCurrent(nullptr),
    m_openTimestamp(0.0),
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
        m_sessionFilter = conf.data().get("session-filter", false).asBool();
    } else if (conf.data().isArray()) {
        m_historySize = conf.data().get((Json::ArrayIndex)1, 10).asInt();
        m_sensibility = conf.data().get((Json::ArrayIndex)2, 1.0).asDouble();
        m_valueAreaSize = conf.data().get((Json::ArrayIndex)3, 70.0).asDouble();
        m_computePeaksAndValleys = conf.data().get((Json::ArrayIndex)4, false).asBool();
        m_tickScale = conf.data().get((Json::ArrayIndex)5, 1.0).asDouble();
        m_sessionFilter = conf.data().get((Json::ArrayIndex)6, 1.0).asBool();
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
        m_sessionFilter = conf.data().get("session-filter", false).asBool();
    } else if (conf.data().isArray()) {
        m_historySize = conf.data().get((Json::ArrayIndex)1, 10).asInt();
        m_sensibility = conf.data().get((Json::ArrayIndex)2, 1.0).asDouble();
        m_valueAreaSize = conf.data().get((Json::ArrayIndex)3, 70.0).asDouble();
        m_computePeaksAndValleys = conf.data().get((Json::ArrayIndex)4, false).asBool();
        m_tickScale = conf.data().get((Json::ArrayIndex)5, 1.0).asDouble();
        m_sessionFilter = conf.data().get((Json::ArrayIndex)6, 1.0).asBool();
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
            m_openTimestamp = baseTime(tick.timestamp(), timeframe());
        } else {
            m_openTimestamp = tick.timestamp();
        }

        // session offset and duration only apply to a daily VP
        if (timeframe() == TF_DAY && m_sessionFilter) {
            m_openTimestamp += m_sessionOffset;
        }

        createVolumeProfile(m_openTimestamp, tick.last());
    }

    // ignore ticks out of the daily session
    if (m_sessionFilter && timeframe() == TF_DAY && (m_sessionOffset > 0 || m_sessionDuration > 0)) {
        if (tick.timestamp() < m_openTimestamp) {
            return;
        }

        if (tick.timestamp() >= m_openTimestamp + (m_sessionDuration > 0.0 ? m_sessionDuration : timeframe())) {
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

    if (m_pCurrent->timeframe <= 0) {
        m_pCurrent->timeframe = timeframe() > 0.0 ? timeframe() : (lastTimestamp() - m_pCurrent->timestamp);
    }

    T_MergedVolumeByPriceVector mergedVolumesByPrice;

    if (m_computePeaksAndValleys) {
        if (mergedVolumesByPrice.empty()) {
            computeMergedVolumesByPrice(mergedVolumesByPrice);
        }

        std::vector<o3d::Int32> peaksIdx;
        std::vector<o3d::Int32> valleysIdx;

        findPeaksAndValleys(mergedVolumesByPrice, m_pCurrent->pocVolume, peaksIdx, valleysIdx);

        for (o3d::Int32 i = 0; i < peaksIdx.size(); ++i) {
            m_pCurrent->peaks.push(mergedVolumesByPrice[i].first);
        }

        for (o3d::Int32 i = 0; i < valleysIdx.size(); ++i) {
            m_pCurrent->valleys.push(mergedVolumesByPrice[i].first);
        }
    }

    if (isComputeValueArea()) {
        if (mergedVolumesByPrice.empty()) {
            computeMergedVolumesByPrice(mergedVolumesByPrice);
        }

        std::make_pair(m_pCurrent->valPrice, m_pCurrent->vahPrice) = computeValueArea(
            mergedVolumesByPrice, m_pCurrent->pocPrice, m_valueAreaSize);
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

void VolumeProfile::computeVolumesByPrice(T_VolumeByPriceVector &outVolumesByPrice) const
{
    if (m_pCurrent == nullptr) {
        return;
    }

    size_t size = m_pCurrent->bins.size();

    outVolumesByPrice.resize(size);

    for (VolumeProfileData::CIT_BinHashMap cit = m_pCurrent->bins.cbegin(); cit != m_pCurrent->bins.cend(); ++cit) {
        outVolumesByPrice.push_back(std::make_pair(cit->first, std::make_pair(cit->second.first, cit->second.second)));
    }

    std::sort(outVolumesByPrice.begin(), outVolumesByPrice.end(),
              [](const T_VolumeByPrice& a, const T_VolumeByPrice &b) { return a.first < b.first; });
}

void VolumeProfile::computeMergedVolumesByPrice(T_MergedVolumeByPriceVector &outMergedVolumesByPrice) const
{
    if (m_pCurrent == nullptr) {
        return;
    }

    size_t size = m_pCurrent->bins.size();

    outMergedVolumesByPrice.resize(size);

    for (VolumeProfileData::CIT_BinHashMap cit = m_pCurrent->bins.cbegin(); cit != m_pCurrent->bins.cend(); ++cit) {
        outMergedVolumesByPrice.push_back(std::make_pair(cit->first, cit->second.first + cit->second.second));
    }

    std::sort(outMergedVolumesByPrice.begin(), outMergedVolumesByPrice.end(),
              [](const T_MergedVolumeByPrice& a, const T_MergedVolumeByPrice &b) { return a.first < b.first; });
}

std::pair<o3d::Double, o3d::Double> VolumeProfile::computeValueArea(
    const T_MergedVolumeByPriceVector &mergedVolumesByPrice,
    o3d::Double pocPrice,
    o3d::Double valueAreaSize) const
{
    if (mergedVolumesByPrice.empty()) {
        return {0, 0};
    }

    o3d::Int32 pocIdx = -1;
    o3d::Double sumVols = 0.0;

    for (o3d::Int32 i = 0; i < mergedVolumesByPrice.size(); ++i) {
        if (mergedVolumesByPrice[i].first == pocPrice) {
            pocIdx = i;
        }

        // sumVols += volumesByPrice[i].second.first + volumesByPrice[i].second.second;
        sumVols += mergedVolumesByPrice[i].second + mergedVolumesByPrice[i].second;
    }

    if (pocIdx < 0) {
        return {0, 0};
    }

    o3d::Double supPrice = pocPrice;
    o3d::Double infPrice = pocPrice;

    o3d::Double inArea = sumVols * valueAreaSize * 0.01;

    o3d::Int32 maxIndex = mergedVolumesByPrice.size() - 1;
    // o3d::Double summed = volumesByPrice[pocIdx].second.first + volumesByPrice[pocIdx].second.second;
    o3d::Double summed = mergedVolumesByPrice[pocIdx].second;

    // start from left and right of the POC
    o3d::Int32 left = pocIdx - 1;
    o3d::Int32 right = pocIdx + 1;

    while (summed < inArea) {
        if (left < 0 && right > maxIndex) {
            break;
        }

        if (left >= 0 && ((right <= maxIndex && mergedVolumesByPrice[left].second > mergedVolumesByPrice[right].second)
                          || right > maxIndex)) {
            summed += mergedVolumesByPrice[left].second;
            infPrice = mergedVolumesByPrice[left].first;
            left -= 1;
        } else if (right <= maxIndex) {
            summed += mergedVolumesByPrice[right].second;
            supPrice = mergedVolumesByPrice[right].first;
            right += 1;
        }
    }

    return {o3d::min(supPrice, infPrice), o3d::max(supPrice, infPrice)};
}

void findPeaks(const DataArray &weights, o3d::Double minHeight, o3d::Int32 minDistance,
               std::vector<o3d::Int32> &peaksIdx)
{
    for (o3d::Int32 i = 1; i < weights.getSize() - 1; ++i) {
        // detect a peak of min height
        if (weights[i] > weights[i-1] && weights[i] > weights[i+1] && weights[i] >= minHeight) {
            // check for min distance
            if (!peaksIdx.empty()) {
                o3d::Int32 lastPeakIdx = peaksIdx.back();
                if (o3d::abs(i - lastPeakIdx) >= minDistance) {
                    peaksIdx.push_back(i);
                } else if (weights[i] > weights[lastPeakIdx]) {
                    // if current peak is higher than previous peak but to near, replace by current peak
                    peaksIdx.back() = i;
                }
            } else {
                // first peak, add it
                peaksIdx.push_back(i);
            }
        }
    }
}

void findValleys(const DataArray &weights, o3d::Double minHeight, o3d::Int32 minDistance,
                 std::vector<o3d::Int32> &peaksIdx)
{
    // same as findPeakcs but negate all weights and minHeight
    for (o3d::Int32 i = 1; i < weights.getSize() - 1; ++i) {
        // detect a peak of min height
        if (-weights[i] > -weights[i-1] && -weights[i] > -weights[i+1] && -weights[i] >= -minHeight) {
            // check for min distance
            if (!peaksIdx.empty()) {
                o3d::Int32 lastPeakIdx = peaksIdx.back();
                if (o3d::abs(i - lastPeakIdx) >= minDistance) {
                    peaksIdx.push_back(i);
                } else if (-weights[i] > -weights[lastPeakIdx]) {
                    // if current peak is higher than previous peak but to near, replace by current peak
                    peaksIdx.back() = i;
                }
            } else {
                // first peak, add it
                peaksIdx.push_back(i);
            }
        }
    }
}

void VolumeProfile::findPeaksAndValleys(
    const T_MergedVolumeByPriceVector &mergedVolumeByPrice,
    o3d::Double pocVolume,
    std::vector<o3d::Int32> &outPeaksIdx,
    std::vector<o3d::Int32> &outValleysIdx) const
{
    DataArray weights(mergedVolumeByPrice.size());

    for (size_t i = 0; i < mergedVolumeByPrice.size(); ++i) {
        // weights[i] = mergedVolumeByPrice[i].second.first + mergedVolumeByPrice[i].second.second;
        weights[i] = mergedVolumeByPrice[i].second;
    }

    o3d::Double minHeight = pocVolume * 0.25;
    o3d::Int32 minDistance = o3d::max(2, static_cast<o3d::Int32>(weights.getSize() / 6));

    findPeaks(weights, minHeight, minDistance, outPeaksIdx);
    findValleys(weights, minHeight, minDistance, outValleysIdx);
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
