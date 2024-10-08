/**
 * @brief SiiS MAADX strategy signal analyser.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#include "pullbackbbanalyser.h"

#include "siis/config/strategyconfig.h"

using namespace siis;

PullbackBBAnalyser::PullbackBBAnalyser(
            Strategy *strategy,
            const o3d::String &name,
            o3d::Double timeframe,
            o3d::Double sourceTimeframe,
            o3d::Int32 depth,
            o3d::Int32 history,
            Price::Method priceMethod) :
    TimeframeBarAnalyser(strategy, name, timeframe, sourceTimeframe, depth, history, priceMethod),
    m_bollinger("bollinger", timeframe),
    m_adx("adx", timeframe),
    m_breakout(0),
    m_integrate(0),
    m_confirmation(0),
    m_hasAdx(false)
{

}

PullbackBBAnalyser::~PullbackBBAnalyser()
{

}

o3d::String PullbackBBAnalyser::typeName() const
{
    return "bollinger";
}

void PullbackBBAnalyser::init(const AnalyserConfig &conf)
{
    configureIndicator(conf, "bollinger", m_bollinger);
    configureIndicator(conf, "adx", m_adx);

    if (conf.hasIndicator("adx")) {
        m_hasAdx = true;
    }

    m_breakout = 0;
    m_integrate = 0;

    TimeframeBarAnalyser::init(conf);
}

void PullbackBBAnalyser::terminate()
{

}

void PullbackBBAnalyser::compute(o3d::Double timestamp, o3d::Double lastTimestamp)
{
    m_confirmation = 0;

    if (price().consolidated()) {
        if (price().close().last() > price().close().prev()) {
            m_confirmation = 1;
        } else if (price().close().last() < price().close().prev()) {
            m_confirmation = -1;
        }
    }

    m_breakout = 0;
    m_integrate = 0;

    o3d::Bool compute = true;

    if (isUpdateAtclose()) {
        compute = price().consolidated();
    }

    if (compute) {
        m_bollinger.compute(timestamp, price().close());

        o3d::Int32 uc = DataArray::cross(price().close(), m_bollinger.upper());
        o3d::Int32 lc = DataArray::cross(price().close(), m_bollinger.lower());

        if (uc > 0) {
            m_breakout = 1;
        } else if (uc < 0) {
            m_integrate = -1;
        }

        if (lc < 0) {
            m_breakout = -1;
        } else if (lc > 0) {
            m_integrate = 1;
        }

        if (m_hasAdx) {
            m_adx.compute(timestamp, price().high(), price().low(), price().close());
        }
    }
}

o3d::Double PullbackBBAnalyser::entryPrice() const
{
    return 0.0;   // @todo for limit entry
}

o3d::Double PullbackBBAnalyser::takeProfit(o3d::Int32 dir, o3d::Double profitScale) const
{
    if (dir > 0) {
        // return m_bollinger.lastMiddle() + profitScale * (m_bollinger.lastMiddle() - m_bollinger.lastLower());
        return price().last() + profitScale * (m_bollinger.lastMiddle() - m_bollinger.lastLower());
        // si on met pas le meme delta alors ca risque de faire balotter le fitness
        // m_bollinger.lastUpper() - m_bollinger.lastMiddle())
    } else if (dir < 0) {
        // return m_bollinger.lastMiddle() - profitScale * (m_bollinger.lastUpper() - m_bollinger.lastMiddle());
        return price().last() - profitScale * (m_bollinger.lastUpper() - m_bollinger.lastMiddle());
    }

    return 0.0;
}

o3d::Double PullbackBBAnalyser::stopLoss(o3d::Int32 dir, o3d::Double lossScale, o3d::Double riskReward) const
{
    // return min(pullback.bb_tf.price.low) - price_epsilon

    if (dir > 0) {
        // return m_bollinger.lastMiddle() - riskReward * lossScale * (m_bollinger.lastMiddle() - m_bollinger.lastLower());
        return price().last() - riskReward * lossScale * (m_bollinger.lastMiddle() - m_bollinger.lastLower());
    } else if (dir < 0) {
        // return m_bollinger.lastMiddle() + riskReward * lossScale * (m_bollinger.lastUpper() - m_bollinger.lastMiddle());
        return price().last() + riskReward * lossScale * (m_bollinger.lastUpper() - m_bollinger.lastMiddle());
    }

    return 0.0;
}
