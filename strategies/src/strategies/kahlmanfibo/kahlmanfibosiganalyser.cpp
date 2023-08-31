/**
 * @brief SiiS strategy KahlmanFibo strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#include "kahlmanfibosiganalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

KahlmanFiboSigAnalyser::KahlmanFiboSigAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_donchian("donchian", timeframe),
    m_hma("hma", timeframe),
    m_hma3("hma3", timeframe),
    m_trend(0),
    m_sig(0),
    m_confirmation(0)
{

}

KahlmanFiboSigAnalyser::~KahlmanFiboSigAnalyser()
{

}

void KahlmanFiboSigAnalyser::init(AnalyserConfig conf)
{
    configureIndictor(conf, "hma", m_hma);
    configureIndictor(conf, "hma", m_hma3);   // same config as for hma
    configureIndictor(conf, "donchian", m_donchian);

    m_confirmation = 0;
    m_trend = 0;
    m_sig = 0;

    StdAnalyser::init(conf);
}

void KahlmanFiboSigAnalyser::terminate()
{

}

TradeSignal KahlmanFiboSigAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    TradeSignal signal(timeframe(), timestamp);

    m_confirmation = 0;

    if (price().consolidated()) {
        if (price().close().last() > price().close().prev()) {
            m_confirmation = 1;
        } else if (price().close().last() < price().close().prev()) {
            m_confirmation = -1;
        }
    }
/*
hma3(x, len) => p = len/2, wma(wma(x, p/3)*3 - wma(x, p/2) - wma(x, p), p)

kahlman(x, g) =>
    kf = 0.0
    dk = x - nz(kf[1], x)
    smooth = nz(kf[1],x)+dk*sqrt(g*2)
    velo = 0.0
    velo := nz(velo[1],0) + (g*dk)
    kf := smooth+velo

a = k ? kahlman(hma(price, length), gain) : hma(price, length)
b = k ? kahlman(hma3(close, length), gain) : hma3(close, length)
c = b > a ? color.lime : color.red
crossdn = a > b and a[1] < b[1]
crossup = b > a and b[1] < a[1]

fill(plot(a,color=c,linewidth=1,transp=75), plot(b,color=c,linewidth=1,transp=75), color=c, transp=55)
plotshape(labels and crossdn ? a : na, location=location.abovebar, style=shape.labeldown, color=color.red, size=size.tiny, text="S", textcolor=color.white, transp=0, offset=0)
plotshape(labels and crossup ? a : na, location=location.belowbar, style=shape.labelup, color=color.green, size=size.tiny, text="B", textcolor=color.white, transp=0, offset=8)
*/

/*
// initialise mutatables
hftrue = true   pour uprend  -> trend 1
lftrue = true   pour downtrend  -> trend -1
cftrue = true   pour range  -> trend 0

evupin = crossover(close, fi)    // market enters up trend
evupout = crossunder(close, hf)  // market leaves up trend

hftrue := evupin ? true : evupout ? false : hftrue[1]  // mutatable true if in up trend

evdownin = crossunder(close, lf)  // market enters down trend
evdownout = crossover(close, lf)  // market leaves down trend

lftrue := evdownin ? true : evdownout ? false : lftrue[1]  // mutatable true if in down trend
cftrue := not hftrue and not lftrue ? true : false         // mutatable true if instrument is ranging

evupout -> red / sell
evdownout -> green / buy
*/

//    if (1) {  // price().consolidated()) {
//        // compute only at close
//        m_fast_h_ma.compute(timestamp, price().high());
//        m_fast_m_ma.compute(timestamp, price().price());
//        m_fast_l_ma.compute(timestamp, price().low());

//        m_adx.compute(timestamp, price().high(), price().low(), price().close());

//        o3d::Int32 hc = DataArray::cross(price().close(), m_fast_h_ma.hma());
//        o3d::Int32 lc = DataArray::cross(price().close(), m_fast_l_ma.hma());

//        if (hc > 0) {
//            m_trend = 1;
//            m_sig = 1;
//        } else if (lc < 0) {
//            m_trend = -1;
//            m_sig = -1;
//        } else {
//            m_sig = 0;
//        }
//    }

    return signal;
}

o3d::Double KahlmanFiboSigAnalyser::takeProfit(o3d::Double profitScale) const
{
//    if (m_trend > 0) {
//        return price().close().last() + profitScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
//    } else if (m_trend < 0) {
//        return price().close().last() - profitScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
//    }

    return 0.0;
}

o3d::Double KahlmanFiboSigAnalyser::stopLoss(o3d::Double lossScale, o3d::Double riskReward) const
{
//    if (m_trend > 0) {
//        return price().close().last() - riskReward * lossScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
//    } else if (m_trend < 0) {
//        return price().close().last() + riskReward * lossScale * (m_fast_h_ma.last() - m_fast_l_ma.last());
//    }

    return 0.0;
}
