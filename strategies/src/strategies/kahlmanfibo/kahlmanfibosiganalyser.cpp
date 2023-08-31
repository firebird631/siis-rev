/**
 * @brief SiiS strategy KahlmanFibo strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#include "kahlmanfibosiganalyser.h"

#include "siis/config/strategyconfig.h"

#include <o3d/core/math.h>

using namespace siis;

KahlmanFiboSigAnalyser::KahlmanFiboSigAnalyser(
            Strategy *strategy,
            o3d::Double timeframe,
            o3d::Double subTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    StdAnalyser(strategy, timeframe, subTimeframe, depth, history, priceMethod),
    m_gain(0.7),  // step 0.01 min 0.0001
    m_kahlman(false),
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
    configureIndictor(conf, "donchian", m_donchian);

    // half len of the configured HMA
    m_hma3.setLength(m_hma.len() / 2);

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

    if (1) {  // price().consolidated()) {
        // compute only at close
        m_hma.compute(timestamp, price().price());   // compute on HL2 price
        m_hma3.compute(timestamp, price().close());  // compute on close price
        m_donchian.compute(timestamp, price().high(), price().low());

        if (m_kahlman) {
            // @todo it reduce the lag and then improve matchin with the donchian+fibo based signal
            kahlmanHma();
            kahlmanHma3();

            m_trend = m_kHma3.last() > m_kHma.last() ? 1 : -1;

            /*o3d::Int32 cr = DataArray::cross(m_kHma3, m_kHma);

            // changes of trend
            if (cr > 0) {
                m_trend = 1;
            } else if (cr < 0) {
                m_trend = -1;
            }*/
        } else {
            m_trend = m_hma3.last() > m_hma.last() ? 1 : -1;

            /*o3d::Int32 cr = DataArray::cross(m_hma3.hma3(), m_hma.hma());

            // changes of trend
            if (cr > 0) {
                m_trend = 1;
            } else if (cr < 0) {
                m_trend = -1;
            }*/

            // maybee donchian based indicator can m_trend = 0 if range is detected
        }
    }

    return signal;
}

o3d::Double KahlmanFiboSigAnalyser::takeProfit(o3d::Double profitScale) const
{
    if (m_trend > 0) {
        return price().close().last() + profitScale * (m_donchian.lastUpper() - m_donchian.lastLower());
    } else if (m_trend < 0) {
        return price().close().last() - profitScale * (m_donchian.lastUpper() - m_donchian.lastLower());
    }

    return 0.0;
}

o3d::Double KahlmanFiboSigAnalyser::stopLoss(o3d::Double lossScale, o3d::Double riskReward) const
{
    if (m_trend > 0) {
        return price().close().last() - riskReward * lossScale * (m_donchian.lastUpper() - m_donchian.lastLower());
    } else if (m_trend < 0) {
        return price().close().last() + riskReward * lossScale * (m_donchian.lastUpper() - m_donchian.lastLower());
    }

    return 0.0;
}

void KahlmanFiboSigAnalyser::kahlmanHma()
{
    const o3d::Double G2_sqrt = o3d::Math::sqrt(m_gain*2);

    if (m_kHma.getSize() != m_hma.hma().getSize()) {
        m_kHma.setSize(m_hma.hma().getSize());
    }

    // on a donc les DataArray : kf, nz(kf[1], x), dk, dk*G2_sqrt, velo => m_kHma

    // @todo
    //    kf = 0.0   // array size hma a 0
    //    dk = m_hma.hma() - nz(kf[1], x)    // nz set kf[n-1], a la valeur de x[n] si x[n] > 0
    //    smooth = nz(kf[1],x)+dk*sqrt(m_gain*2)  // on a encore le meme terme,
    //    velo = 0.0
    //    velo := nz(velo[1],0) + (m_gain*dk)
    //    m_kHma := smooth+velo
}

void KahlmanFiboSigAnalyser::kahlmanHma3()
{
    const o3d::Double G2_sqrt = o3d::Math::sqrt(m_gain*2);

    if (m_kHma3.getSize() != m_hma3.hma3().getSize()) {
        m_kHma3.setSize(m_hma3.hma3().getSize());
    }

    // on a donc les DataArray : kf, nz(kf[1], x), dk, dk*G2_sqrt, velo => m_kHma3
    // reuse the temporary data arrays

    // @todo
    //    kf = 0.0   // array size hma3 a 0
    //    dk = m_hma.hma() - nz(kf[1], x)    // nz set kf[n-1], a la valeur de x[n] si x[n] > 0
    //    smooth = nz(kf[1],x)+dk*sqrt(m_gain*2)  // on a encore le meme terme,
    //    velo = 0.0   // array size hma3 a 0
    //    velo := nz(velo[1],0) + (m_gain*dk)
    //    m_kHma3 := smooth+velo
}

/*
Entrée sur croix bleu
Et quand khalman est vert ou passe au vert
on entre de pref sur la bougie suivante qui open au prix de la close préc

pour le short pareil maix croix rouge
et khalman en rouge ou passe au rouge

on pourrait faire aussi attention qu'on soit pas plus haut que 50% des 2 ATR current
et inversement 50% plus bas pour le short pour (tester sans déjà)

on pourrait aussi implémenter la trend line et si on est trop proche d'une resi/support (opposé)
pareil on pourrait éviter d'entrer (mais ça devient complexe)

pour TP une croix opposée mais sans confirmation (à close ou immédiatiate)
ou encore inversion de khalman (à close ou immédiate)

on prend pas si le sig est plusieurs bougies apres la bougie de départ de la trend kahlman
car sinon on risque d'entrer apres la fête
pareil on pourrait aussi filtrer si on est trop loin de la bollinger mais ça rajoute
un indicateur et peut etre du flou

version python :
 - Hma3 (register it)
 - StrategyTrader, parameter, siganalyser, confanalyser, context
 - test sur DAX, GOLD, SPX en priorité
 - entrée market, entrée best+2, entrée custom en rapport avec la kahlman



code pour trigger :

//
// donchian channel + fibo
//

per = input(title="donchian L", type=integer, defval=24, minval=15, maxval=60)

hb = highest(high, per)  // High Border
lb = lowest(low, per)    // Low Border
dist = hb - lb           // range of the channel
med = (hb + lb) / 2      // median of the channel

hf = hb - dist*0.236     // Highest Fib
chf = hb - dist*0.382    // Center High Fib
clf = hb - dist*0.618    // Center Low Fib
lf = hb - dist*0.764     // Lowest Fib

//
// pullback levels
//

// entry markers
tol = atr(per) * 0.2  // tolerance for placing triangles and prediction candles at borders

// initialise mutatables
hftrue = true
lftrue = true
cftrue = true

evupin = crossover(close, hf)    // market enters up trend
evupout = crossunder(close, hf)  // market leaves up trend
hftrue := evupin ? true : evupout ? false : hftrue[1]  // mutatable true if in up trend
evdownin = crossunder(close, lf)  // market enters down trend
evdownout = crossover(close, lf)  // market leaves down trend
lftrue := evdownin ? true : evdownout ? false : lftrue[1]  // mutatable true if in down trend
cftrue := not hftrue and not lftrue ? true : false         // mutatable true if instrument is ranging

// identify last bar
lastbar = barstate.islast
lastbar := barstate.isrealtime and isintraday ? not barstate.isconfirmed :barstate.islast
// barstate.islast works also when all bars are confirmed and are history. However in real time and intradaycharts bars never lose this state.
// In that situation 'not barstate.isconfirmed' (like not ishistory) is the better alternative. if you use intraday charts after closing time of
// the markets "barstate.isrealtime and isintraday" will be false and the script will use barstate.islast.

// situation (plotting only)
hbdtrue = lastbar and hftrue  // up trend
lbdtrue = lastbar and lftrue  // down trend

// plot channel and fibzones
phb = plot(hbdtrue ? na : hb, title="high border", color=yellow)
plb = plot(lbdtrue ? na : lb, title="low border", color=yellow)

// plot entry markers
plotshape(evupout ? hb + tol : na, style=shape.cross, location=location.absolute, color=red, size=size.tiny)
plotshape(evdownout ? lb - tol : na, style=shape.cross, location=location.absolute, color=aqua, size=size.tiny)



////

code pour kahlman

//*** HMA-Kahlman Trend Module
price  = input(hl2,  "Price Data (hl2)")
length = input(22,   "Lookback Window", minval=2)
k      = input(true, "Use Kahlman?")
gain   = input(.7,   "Gain", minval=.0001, step=.01)
labels = input(true, "Show Labels?")

hma(x, len) => wma((2 * wma(x, len / 2)) - wma(x, len), round(sqrt(len)))

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
