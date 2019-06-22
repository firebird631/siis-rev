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
    m_mode(MODE_BUY_SELL),
    m_orderCaps(ORDER_ALL),
    m_lastTimestamp(0.0),
    m_tradeable(true),
    m_contractSize(1.0),
    m_lotSize(1.0),
    m_valuePerPip(1.0),
    m_onePipMean(1.0),
    m_baseExchangeRate(1.0),
    m_marginFactor(1.0),
    m_bid(0.0),
    m_ofr(0.0),
    m_hedging(false),
    m_qtyFilter{0.0, 0.0, 0.0},
    m_notionalFilter{0.0, 0.0, 0.0},
    m_ticks(512)
{
    m_base.symbol = baseSymbol;
    m_quote.symbol = quoteSymbol;
}

Market::~Market()
{

}

void Market::setModeAndOrders(Market::Mode mode, o3d::Int32 orders)
{
    m_mode = mode;
    m_orderCaps = orders;
}

void Market::setBaseInfo(const o3d::String &symbol, o3d::Int32 precision)
{
    m_base.symbol = symbol.isValid() ? symbol : m_base.symbol;
    m_base.precision = precision;
}

void Market::setQuoteInfo(const o3d::String &symbol, o3d::Int32 precision)
{
    m_quote.symbol = symbol.isValid() ? symbol : m_quote.symbol;
    m_quote.precision = precision;
}

void Market::setBaseVol24h(o3d::Double vol24h)
{
    m_base.vol24h = vol24h >= 0.0 ? vol24h : m_base.vol24h;
}

void Market::setQuoteVol24h(o3d::Double vol24h)
{
    m_quote.vol24h = vol24h >= 0.0 ? vol24h : m_quote.vol24h;
}

void Market::setType(Market::Type type, Market::Contract contract, Market::Unit unit)
{
    m_type = type;
    m_contract = contract;
    m_unit = unit;
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

void Market::setMakerFee(o3d::Double rate, o3d::Double commission, const o3d::Double limits[])
{
    m_fees[0].rate = rate;
    m_fees[0].commission = commission;
    m_fees[0].limits[0] = limits[0];
    m_fees[0].limits[1] = limits[1];
}

void Market::setTakerFee(o3d::Double rate, o3d::Double commission, const o3d::Double limits[])
{
    m_fees[1].rate = rate;
    m_fees[1].commission = commission;
    m_fees[1].limits[0] = limits[0];
    m_fees[1].limits[1] = limits[1];
}

void Market::setState(o3d::Double baseExchangeRate, o3d::Bool tradeable)
{
    m_baseExchangeRate = baseExchangeRate >= 0.0 ? baseExchangeRate : m_baseExchangeRate;
    m_tradeable = tradeable;
}

void Market::setPrice(o3d::Double bid, o3d::Double ofr, o3d::Double timestamp)
{
    m_bid = bid >= 0.0 ? bid : m_bid;
    m_ofr = ofr >= 0.0 ? ofr : m_ofr;
    m_lastTimestamp = timestamp >= 0.0 ? timestamp : m_lastTimestamp;
}

void Market::setPriceFilter(const o3d::Double filter[])
{
    m_priceFilter[0] = filter[0] >= 0.0 ? filter[0] : m_priceFilter[0];
    m_priceFilter[1] = filter[1] >= 0.0 ? filter[1] : m_priceFilter[1];
    m_priceFilter[2] = filter[2] >= 0.0 ? filter[2]: m_priceFilter[2];
}

void Market::setQtyFilter(const o3d::Double filter[])
{
    m_qtyFilter[0] = filter[0] >= 0.0 ? filter[0] : m_qtyFilter[0];
    m_qtyFilter[1] = filter[1] >= 0.0 ? filter[1] : m_qtyFilter[1];
    m_qtyFilter[2] = filter[2] >= 0.0 ? filter[2]: m_qtyFilter[2];
}

void Market::setNotionalFilter(const o3d::Double filter[])
{
    m_notionalFilter[0] = filter[0] >= 0.0 ? filter[0] : m_notionalFilter[0];
    m_notionalFilter[1] = filter[1] >= 0.0 ? filter[1] : m_notionalFilter[1];
    m_notionalFilter[2] = filter[2] >= 0.0 ? filter[2] : m_notionalFilter[2];
}

o3d::String Market::formatPrice(o3d::Double price, o3d::Bool useQuote, o3d::Bool displaySymbol) const
{
    o3d::Int32 precision = 0;

    if (useQuote) {
        precision = m_quote.precision;
    } else {
        precision = m_base.precision;
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
        return formattedPrice + m_quote.symbol;
    } else {
        return formattedPrice + m_base.symbol;
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
