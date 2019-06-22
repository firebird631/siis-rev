/**
 * @brief SiiS strategy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/market.h"
#include "siis/utils/math.h"

using namespace siis;

Market::Market(const o3d::String &marketId,
               const o3d::String &symbol,
               const o3d::String &baseSymbol,
               const o3d::String &quoteSymbol) :
    m_marketId(marketId),
    m_symbol(symbol),
    m_baseSymbol(baseSymbol),
    m_quoteSymbol(quoteSymbol),
    m_mode(MODE_BUY_SELL),
    m_orderCaps(ORDER_ALL),
    m_lastTimestamp(0.0),
    m_tradeable(true),
    m_basePrecision(8),
    m_quotePrecision(8),
    m_contractSize(1.0),
    m_lotSize(1.0),
    m_valuePerPip(1.0),
    m_onePipMean(1.0),
    m_baseExchangeRate(1.0),
    m_marginFactor(1.0),
    m_bid(0.0),
    m_ofr(0.0),
    m_baseVol24h(0.0),
    m_quoteVol24h(0.0),
    m_hedging(false),
    m_makerFee(0.0),
    m_takerFee(0.0),
    m_commission(0.0),
    m_qtyFilter{0.0, 0.0, 0.0},
    m_notionalFilter{0.0, 0.0, 0.0},
    m_ticks(512)
{

}

Market::~Market()
{

}

void Market::setPrecisions(o3d::Int32 basePrecision, o3d::Int32 quotePrecision)
{
    m_basePrecision = basePrecision;
    m_quotePrecision = quotePrecision;
}

void Market::setDetails(
        o3d::Double contractSize,
        o3d::Double lotSize,
        o3d::Double valuePerPip,
        o3d::Double onePipMean,
        o3d::Bool hedging)
{
    m_contractSize = contractSize;
    m_lotSize = lotSize;
    m_valuePerPip = valuePerPip;
    m_onePipMean = onePipMean;
    m_hedging = hedging;
}

void Market::setFees(o3d::Double makerFee, o3d::Double takerFee, o3d::Double commission)
{
    m_makerFee = makerFee;
    m_takerFee = takerFee;
    m_commission = commission;
}

void Market::setVariables(o3d::Double baseExchangeRate, o3d::Double baseVol24h, o3d::Double quoteVol24h, o3d::Bool tradeable)
{
    m_baseExchangeRate = baseExchangeRate;
    m_baseVol24h = baseVol24h;
    m_quoteVol24h = quoteVol24h;
    m_tradeable = tradeable;
}

void Market::setPrice(o3d::Double bid, o3d::Double ofr, o3d::Double timestamp)
{
    m_bid = bid;
    m_ofr = ofr;
    m_lastTimestamp = timestamp;
}

void Market::setQtyFilter(o3d::Double min, o3d::Double max, o3d::Double step)
{
    m_qtyFilter[0] = min;
    m_qtyFilter[1] = max;
    m_qtyFilter[2] = step;
}

void Market::setNotionalFilter(o3d::Double min, o3d::Double max, o3d::Double step)
{
    m_notionalFilter[0] = min;
    m_notionalFilter[1] = max;
    m_notionalFilter[2] = step;
}

o3d::String Market::formatPrice(o3d::Double price, o3d::Bool useQuote, o3d::Bool displaySymbol) const
{
    o3d::Int32 precision = 0;

    if (useQuote) {
        precision = m_quotePrecision;
    } else {
        precision = m_basePrecision;
    }

    if (!precision) {
        precision = -static_cast<o3d::Int32>(log10(m_onePipMean));

        if (!precision) {
            precision = 8;
        }
    }

    o3d::String formattedPrice;
    formattedPrice.concat(truncate(price, precision), precision);

    if (formattedPrice.find('.') >= 0) {
        formattedPrice.trimRight('0', true);
        formattedPrice.trimRight('.');
    }

    if (!displaySymbol) {
        return formattedPrice;
    }

    if (useQuote) {
        return formattedPrice + m_quoteSymbol;
    } else {
        return formattedPrice + m_baseSymbol;
    }
}

o3d::Double Market::adjustQty(o3d::Double qty, o3d::Bool minIsZero) const
{
    if (m_qtyFilter[0] > 0.0 && qty < m_qtyFilter[0]) {
        if (minIsZero) {
            return 0.0;
        }

        return m_qtyFilter[0];
    }

    if (m_qtyFilter[1] > 0.0 && qty > m_qtyFilter[1]) {
        return m_qtyFilter[1];
    }

    if (m_qtyFilter[2] > 0.0) {
        o3d::Int32 precision = -static_cast<o3d::Int32>(log10(m_qtyFilter[2]));
        return o3d::max(round(m_qtyFilter[2] * ::round(qty / m_qtyFilter[2]), precision), m_qtyFilter[0]);
    }

    return qty;
}

o3d::String Market::formatQty(o3d::Double qty) const
{
    o3d::Int32 precision = -static_cast<o3d::Int32>(log10(m_qtyFilter[2]));
    o3d::String formattedQty;
    formattedQty.concat(truncate(qty, precision), precision);

    if (formattedQty.find('.') >= 0) {
        formattedQty.trimRight('0', true);
        formattedQty.trimRight('.');
    }

    return formattedQty;
}

o3d::Double Market::marginCost(o3d::Double qty) const
{
    o3d::Double realizedPositionCost = qty * (m_lotSize * m_contractSize);  // in base currency
    o3d::Double marginCost = realizedPositionCost * m_marginFactor / m_baseExchangeRate;

    return marginCost;
}
