/**
 * @brief SiiS strategy volume indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#include "siis/indicators/volume/volume.h"

using namespace siis;

Volume::Volume(const o3d::String &name, o3d::Double timeframe) :
    Indicator(name, timeframe),
    m_volume(20, 20),
    m_min(0),
    m_max(0),
    m_prev(0),
    m_last(0)
{

}

Volume::Volume(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_volume(20, 20),
    m_min(0),
    m_max(0),
    m_prev(0),
    m_last(0)
{

}

void Volume::setConf(IndicatorConfig conf)
{

}

void Volume::compute(const OhlcCircular &ohlc)
{
    m_prev = m_last;

    const o3d::Int32 size = ohlc.size();

    if (size != m_volume.getSize()) {
        m_volume.setSize(ohlc.size());
    }

    o3d::Double timestamp = 0.0;

    // timestamp, timeframe, open, high, low, close, volume, ended
    o3d::Int32 i = 0;
    const Ohlc *cur;

    for (auto cit = ohlc.cbegin(); cit != ohlc.cend(); ++cit) {
        cur = (*cit);

        m_volume[i++] = cur->volume();
        timestamp = cur->timestamp();
    }

    m_last = m_volume[m_volume.getSize()-1];
    done(timestamp);
}

//void Volume::compute(o3d::Double timestamp, const OhlcArray &ohlc, o3d::Int32 ofs)
//{
//    m_prev = m_last;

//    const o3d::Int32 n = ohlc.getSize();
//    const o3d::Int32 size = ohlc.getSize() - ofs;

//    m_volume.setSize(size);

//    for (o3d::Int32 i = ofs; i < n; ++i) {
//        m_volume[i] = *d;
//        d+= 8;
//    }

//    m_last = m_volume[n-1];
//    done(timestamp);
//}
