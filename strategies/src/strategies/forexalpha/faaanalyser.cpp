/**
 * @brief SiiS forexalpha strategy A analyser.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "faaanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

FaAAnalyser::FaAAnalyser(Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    FaAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_atr("atr", timeframe),
    m_sma("sma", timeframe),
    m_ema("ema", timeframe),
    m_rsi("rsi", timeframe),
    m_td9("td9", timeframe)
{

}

FaAAnalyser::~FaAAnalyser()
{

}

o3d::String FaAAnalyser::typeName() const
{
    return "A";
}

void FaAAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "atr", m_atr);
    configureIndicator(conf, "sma", m_sma);
    configureIndicator(conf, "ema", m_ema);
    configureIndicator(conf, "rsi", m_rsi);
    configureIndicator(conf, "td9", m_td9);

    TimeframeBarAnalyser::init(conf);
}

void FaAAnalyser::terminate()
{

}

void FaAAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_lastSignal.reset();

    m_rsi.compute(lastTimestamp, price().price());
    m_sma.compute(lastTimestamp, price().price());
    m_ema.compute(lastTimestamp, price().price());
    m_atr.compute(lastTimestamp, price().high(), price().low(), price().close());
    m_td9.compute(lastTimestamp, price().timestamp(), price().high(), price().low(), price().close());

    o3d::Int32 lvl1Signal = 0;

    if (m_ema.last() < m_sma.last()) {
        // bear trend
        if (m_rsi.last() > 50) {
            lvl1Signal = -1;
        } else if (m_rsi.last() < 20) {
            lvl1Signal = 1;
        }
    } else {
        // bull trend
        if (m_rsi.last() > 80) {
            lvl1Signal = -1;
        } else if (m_rsi.last() < 60) {
            lvl1Signal = 1;
        }
    }

    // long entry on sell-setup
    if (m_td9.c().c >= 1 && m_td9.c().c <= 6 && m_td9.c().d < 0 && lvl1Signal > 0 /* && volumeSignal > 0*/) {
        m_lastSignal.setEntry();
        m_lastSignal.setLong();
        m_lastSignal.setPrice(price().close().getLast());

        if (m_td9.c().tdst > 0.0) {
            m_lastSignal.setStop(m_td9.c().tdst);
        }

        // log("content", o3d::String("Entry long tdst={0} stop={1}").arg(m_td9.c().tdst).arg(signal.sl()));
    }

    // short entry on buy-setup
    else if (m_td9.c().c >= 1 && m_td9.c().c <= 6 && m_td9.c().d > 0 && lvl1Signal < 0 /* && volumeSignal > 0*/) {
        m_lastSignal.setEntry();
        m_lastSignal.setShort();
        m_lastSignal.setPrice(price().close().getLast());

        if (m_td9.c().tdst > 0.0) {
            m_lastSignal.setStop(m_td9.c().tdst);
        }

        // log("content", o3d::String("Entry short tdst={0} stop={1}").arg(m_td9.c().tdst).arg(signal.sl()));
    }

    // aggressive long entry
    else if (m_td9.c().c >= 8 && m_td9.c().d > 0 && lvl1Signal > 0 /* && volumeSignal > 0*/) {
        m_lastSignal.setEntry();
        m_lastSignal.setLong();
        m_lastSignal.setPrice(price().close().getLast());

        // log("content", o3d::String("Aggressive long entry close={0} stop={1}").arg(signal.p()).arg(signal.sl()));
    }

    // aggressive short entry
    else if (m_td9.c().c >= 8 && m_td9.c().d < 0 && lvl1Signal < 0 /* && volumeSignal > 0*/) {
        m_lastSignal.setEntry();
        m_lastSignal.setShort();
        m_lastSignal.setPrice(price().close().getLast());

        // log("content", o3d::String("Aggressive short entry close={0} stop={1}").arg(signal.p()).arg(signal.sl()));
    }

    //
    // setup completed
    //

    // sell-setup
    else if (m_td9.c().c >= 8 && m_td9.c().d < 0 && lvl1Signal < 0) {
        m_lastSignal.setExit();
        m_lastSignal.setLong();
        m_lastSignal.setPrice(price().close().getLast());

        // log("content", o3d::String("Exit long c8p-c9 ({0}{1})").arg(m_td9.c().c).arg(m_td9.c().p ? "p" : ""));
    }

    // buy-setup
    else if (m_td9.c().c >= 8 && m_td9.c().d > 0 && lvl1Signal < 0) {
        m_lastSignal.setExit();
        m_lastSignal.setShort();
        m_lastSignal.setPrice(price().close().getLast());

        // log("content", o3d::String("Exit short c8p-c9 ({0}{1})").arg(m_td9.c().c).arg(m_td9.c().p ? "p" : ""));
    }

    //
    // setup aborted (@todo how to this correctly)
    //

    else if (((m_td9.c().c >= 4 && m_td9.c().c <= 7) && m_td9.c().d < 0) && lvl1Signal < 0) {
        m_lastSignal.setExit();
        m_lastSignal.setLong();
        m_lastSignal.setPrice(price().close().getLast());

        // log("content", o3d::String("Abort long c4-c7"));
    }

    else if (((m_td9.c().c >= 4 && m_td9.c().c <= 7) && m_td9.c().d > 0) && lvl1Signal > 0) {
        m_lastSignal.setExit();
        m_lastSignal.setShort();
        m_lastSignal.setPrice(price().close().getLast());

        // log("content", o3d::String("Abort short c4-c7"));
    }

    //
    // invalidation 2 of opposite setup
    //

//    else if (m_td9.c().c > 2 && m_td9.c().d > 0 && lvl1Signal < 0) {
//        signal.setExit();
//        signal.setLong();
//        signal.setPrice(price().close().getLast());

//        // log("content", o3d::String("Canceled long entry c2-c3 price={0}").arg(price().close().getLast()));
//    }

//    else if (m_td9.c().c > 2 && m_td9.c().d < 0 && lvl1Signal > 0) {
//        signal.setExit();
//        signal.setShort();
//        signal.setPrice(price().close().getLast());

//        // log("content", o3d::String("Canceled long entry c2-c3 price={0}").arg(price().close().getLast()));
//    }

    if (m_lastSignal.valid()) {
        // keep signal conditions for machine learning
        // @todo how to, another method storeConditions() when to do it ? we want to store any possible
        // valid signals, even thoose non traded because of somes criterias like max trade etc
        // 'price': prices[-1],
        // 'rsi': self.rsi.last,
        // 'stochrsi': self.stochrsi.last_k,
        // 'bollinger': (self.bollingerbands.last_bottom, self.bollingerbands.last_ma, self.bollingerbands.last_top),
        // 'td.c': self.tomdemark.c.c,
        // 'td.cd': self.tomdemark.cd.c,
    }
}
