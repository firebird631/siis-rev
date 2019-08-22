/**
 * @brief SiiS strategy price indicator.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-15
 */

#include "siis/indicators/price/price.h"

using namespace siis;

static Price::Method methodFromString(const o3d::String &method)
{
    if (method == "CLOSE" || method == "close") {
        return Price::PRICE_CLOSE;
    } else if (method == "HLC" || method == "hlc") {
        return Price::PRICE_HLC;
    } else {
        return Price::PRICE_CLOSE;
    }
}

Price::Price(const o3d::String &name, o3d::Double timeframe, Method method) :
    Indicator(name, timeframe),
    m_method(method),
    m_open(20, 20),
    m_high(20, 20),
    m_low(20, 20),
    m_close(20, 20),
    m_price(20, 20),
    m_timestamp(20, 20),
    m_consolidated(false),
    m_min(0),
    m_max(0),
    m_prev(0),
    m_last(0)
{

}

Price::Price(const o3d::String &name, o3d::Double timeframe, IndicatorConfig conf) :
    Indicator(name, timeframe),
    m_method(PRICE_CLOSE),
    m_open(20, 20),
    m_high(20, 20),
    m_low(20, 20),
    m_close(20, 20),
    m_price(20, 20),
    m_timestamp(20, 20),
    m_consolidated(false),
    m_min(0),
    m_max(0),
    m_prev(0),
    m_last(0)
{
    m_method = methodFromString(conf.data().get("method", "CLOSE").asCString());
}

void Price::setConf(IndicatorConfig conf)
{
    m_method = methodFromString(conf.data().get("method", "CLOSE").asCString());
}

void Price::compute(const OhlcCircular &ohlc)
{
    m_prev = m_last;

    const o3d::Int32 size = ohlc.size();

    if (size != m_price.getSize()) {
        m_price.setSize(size);

        m_open.setSize(size);
        m_high.setSize(size);
        m_low.setSize(size);
        m_close.setSize(size);

        m_timestamp.setSize(size);
    }

    m_min = o3d::Limits<o3d::Double>::max();
    m_max = 0;

    if (m_method == PRICE_CLOSE) {
        // timestamp, timeframe, open, high, low, close, volume, ended
        o3d::Int32 i = 0;
        const Ohlc *cur;

        for (auto cit = ohlc.cbegin(); cit != ohlc.cend(); ++cit) {
            cur = *cit;

            m_timestamp[i] = cur->timestamp();
            m_open[i] = cur->open();
            m_high[i] = cur->high();
            m_low[i] = cur->low();

            m_price[i] = m_close[i] = cur->close();

            m_min = o3d::min(m_min, m_price[i]);
            m_max = o3d::max(m_max, m_price[i]);

            m_consolidated = cur->consolidated();

            ++i;
        }
    } else if (m_method == PRICE_HLC) {
        // timestamp, timeframe, open, high, low, close, volume, ended
        o3d::Int32 i = 0;
        const Ohlc *cur;
        const o3d::Double third = 1.0 / 3.0;

        for (auto cit = ohlc.cbegin(); cit != ohlc.cend(); ++cit) {
            cur = *cit;

            m_timestamp[i] = cur->timestamp();
            m_open[i] = cur->open();
            m_high[i] = cur->high();
            m_low[i] = cur->low();
            m_close[i] = cur->close();

            m_price[i] = (m_high[i] + m_low[i] + m_close[i]) * third;

            m_min = o3d::min(m_min, m_price[i]);
            m_max = o3d::max(m_max, m_price[i]);

            m_consolidated = cur->consolidated();

            ++i;
        }
    } else if (m_method == PRICE_OHLC) {
        // timestamp, timeframe, open, high, low, close, volume, ended
        o3d::Int32 i = 0;
        const Ohlc *cur;
        const o3d::Double forth = 1.0 / 4.0;

        for (auto cit = ohlc.cbegin(); cit != ohlc.cend(); ++cit) {
            cur = *cit;

            m_timestamp[i] = cur->timestamp();
            m_open[i] = cur->open();
            m_high[i] = cur->high();
            m_low[i] = cur->low();
            m_close[i] = cur->close();

            m_price[i] = (m_open[i] + m_high[i] + m_low[i] + m_close[i]) * forth;

            m_min = o3d::min(m_min, m_price[i]);
            m_max = o3d::max(m_max, m_price[i]);

            m_consolidated = cur->consolidated();

            ++i;
        }
    }

    m_last = m_price[m_price.getSize()-1];
    done(m_timestamp[m_timestamp.getSize()-1]);
}

//void Price::compute(o3d::Double timestamp, const OhlcArray &ohlc, o3d::Int32 ofs)
//{
//    m_prev = m_last;

//    const o3d::Int32 n = ohlc.getSize();
//    const o3d::Int32 size = ohlc.getSize() - ofs;

//    m_price.setSize(size);

//    m_open.setSize(size);
//    m_high.setSize(size);
//    m_low.setSize(size);
//    m_close.setSize(size);

//    m_timestamp.setSize(size);

//    m_min = o3d::Limits<o3d::Double>::max();
//    m_max = 0;

//    if (m_method == PRICE_CLOSE) {
//        // timestamp, timeframe, open, high, low, close, volume, ended
//        const o3d::Double *d = ohlc.getData();
//        for (o3d::Int32 i = ofs; i < n; ++i) {
//            m_timestamp[i] = *d; d += 2;
//            m_open[i] = *d++;
//            m_high[i] = *d++;
//            m_low[i] = *d++;
//            m_price[i] = m_close[i] = *d;

//            m_min = o3d::min(m_min, m_price[i]);
//            m_max = o3d::max(m_max, m_price[i]);

//            d += 3;
//        }
//    } else if (m_method == PRICE_HLC) {
//        // timestamp, timeframe, open, high, low, close, volume, ended
//        const o3d::Double *d = ohlc.getData();
//        for (o3d::Int32 i = ofs; i < n; ++i) {
//            m_timestamp[i] = *d; d += 2;
//            m_open[i] = *d++;
//            m_high[i] = *d++;
//            m_low[i] = *d++;
//            m_close[i] = *d;

//            m_price[i] = (m_high[i] + m_low[i] + m_close[i]) / 3.0;

//            m_min = o3d::min(m_min, m_price[i]);
//            m_max = o3d::max(m_max, m_price[i]);

//            d += 3;
//        }
//    }

//    // want to keep this information
//    m_consolidated = ohlc.get(n-1)->consolidated();

//    m_last = m_price[n-1];
//    done(timestamp);
//}
