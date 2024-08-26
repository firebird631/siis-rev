/**
 * @brief SiiS Composite volume profile indicator based on volume profile indicator.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-26
 */

#include "siis/indicators/volumeprofile/compositevolumeprofile.h"
#include "siis/utils/common.h"
#include "siis/utils/math.h"

#include <vector>
#include <algorithm>

using namespace siis;
using o3d::Logger;
using o3d::Debug;


CompositeVolumeProfile::CompositeVolumeProfile(
    const o3d::String &name,
    o3d::Double timeframe,
    o3d::Int32 length,
    o3d::Bool mergeCurrent) :
    Indicator(name, timeframe),
    m_pVolumeProfile(nullptr),
    m_length(length),
    m_mergeCurrent(mergeCurrent),
    m_consolidated(false)
{

}

CompositeVolumeProfile::CompositeVolumeProfile(
    const o3d::String &name,
    o3d::Double timeframe,
    IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_pVolumeProfile(nullptr),
    m_length(0),
    m_mergeCurrent(0),
    m_consolidated(false)
{
    if (conf.data().isObject()) {
        m_length = conf.data().get("length()", 10).asInt();
        m_mergeCurrent = conf.data().get("merge-current", false).asBool();
    } else if (conf.data().isArray()) {
        m_length = conf.data().get((Json::ArrayIndex)1, 10).asInt();
        m_mergeCurrent = conf.data().get((Json::ArrayIndex)2, false).asBool();
    }
}

CompositeVolumeProfile::~CompositeVolumeProfile()
{

}

void CompositeVolumeProfile::setConf(IndicatorConfig conf)
{
    if (conf.data().isObject()) {
        m_length = conf.data().get("length()", 10).asInt();
        m_mergeCurrent = conf.data().get("merge-current", false).asBool();
    } else if (conf.data().isArray()) {
        m_length = conf.data().get((Json::ArrayIndex)1, 10).asInt();
        m_mergeCurrent = conf.data().get((Json::ArrayIndex)2, false).asBool();
    }
}

void CompositeVolumeProfile::setVolumeProfile(VolumeProfile *volumeProfile)
{
    m_pVolumeProfile = volumeProfile;
    O3D_ASSERT(volumeProfile != nullptr);
}

void CompositeVolumeProfile::composite()
{
    if (m_pVolumeProfile == nullptr) {
        return;
    }

    if (m_pVolumeProfile->vp().size() == 0) {
        return;
    }

    m_vp.bins.clear();
    m_vp.peaks.setSize(0);
    m_vp.valleys.setSize(0);

    o3d::Int32 beginIdx = o3d::max<o3d::Int32>(m_pVolumeProfile->vp().size() - m_length, 0);
    o3d::Int32 size = m_pVolumeProfile->vp().size();

    m_vp.timestamp = m_pVolumeProfile->vp().at(beginIdx)->timestamp;
    m_vp.timeframe = timeframe() * (size - beginIdx);

    m_vp.sensibility = m_pVolumeProfile->sensibility();

    m_vp.pocPrice = 0.0;
    m_vp.pocVolume = 0.0;

    for (o3d::Int32 i = beginIdx; i < size; ++i) {
        const VolumeProfileData& vpData = *m_pVolumeProfile->vp().at(i);

        for (VolumeProfileData::CIT_BinHashMap cit = vpData.bins.cbegin(); cit != vpData.bins.cend(); ++cit) {
            if (m_vp.bins.find(cit->first) == m_vp.bins.end()) {
                m_vp.bins.insert(std::make_pair(cit->first, std::make_pair(0.0, 0.0)));
            }

            VolumeProfileData::IT_BinHashMap it = m_vp.bins.find(cit->first);
            it->second.first += cit->second.first;
            it->second.second += cit->second.second;

            if (it->second.first + it->second.second > m_vp.pocVolume) {
                m_vp.pocVolume = it->second.first + it->second.second;
                m_vp.pocPrice = cit->first;
            }
        }
    }

    if (m_mergeCurrent && m_pVolumeProfile->current()) {
        const VolumeProfileData& vpData = *m_pVolumeProfile->current();

        for (VolumeProfileData::CIT_BinHashMap cit = vpData.bins.cbegin(); cit != vpData.bins.cend(); ++cit) {
            if (m_vp.bins.find(cit->first) == m_vp.bins.end()) {
                m_vp.bins.insert(std::make_pair(cit->first, std::make_pair(0.0, 0.0)));
            }

            VolumeProfileData::IT_BinHashMap it = m_vp.bins.find(cit->first);
            it->second.first += cit->second.first;
            it->second.second += cit->second.second;

            if (it->second.first + it->second.second > m_vp.pocVolume) {
                m_vp.pocVolume = it->second.first + it->second.second;
                m_vp.pocPrice = cit->first;
            }
        }
    }

    if (m_vp.timeframe == 0.0) {
        if (m_mergeCurrent) {
            m_vp.timeframe = m_pVolumeProfile->lastTimestamp() - m_vp.timestamp;
        } else {
            m_vp.timeframe = m_pVolumeProfile->vp().at(size-1)->timestamp - m_vp.timestamp;
        }
    }

    m_consolidated = true;
}

void CompositeVolumeProfile::update()
{
    if (m_pVolumeProfile == nullptr) {
        return;
    }
}

void CompositeVolumeProfile::finalize()
{
    if (m_pVolumeProfile == nullptr) {
        return;
    }


}

