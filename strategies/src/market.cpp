/**
 * @brief SiiS strategy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/market.h"
#include "siis/utils/math.h"

using namespace siis;

Market::Market(const o3d::CString &marketId,
               const o3d::CString &pair,
               const o3d::CString &baseSymbol,
               const o3d::CString &quoteSymbol) :
    m_marketId(marketId),
    m_pair(pair),
    m_tradeCaps(TRADE_BUY_SELL),
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
    m_ask(0.0),
    m_last(0.0),
    m_hedging(false),
    m_priceFilter(),
    m_qtyFilter(),
    m_notionalFilter(),
    m_ticks(512)
{
    m_base.symbol = baseSymbol;
    m_quote.symbol = quoteSymbol;
}

Market::~Market()
{

}

void Market::setPair(const o3d::CString &pair)
{
    m_pair = pair;
}

void Market::setAlias(const o3d::CString &alias)
{
    m_alias = alias;
}

void Market::setCapacities(o3d::Int32 tradeCaps, o3d::Int32 orderCaps)
{
    if (tradeCaps > -1) {
        m_tradeCaps = tradeCaps;
    }

    if (orderCaps > -1) {
        m_orderCaps = orderCaps;
    }
}

void Market::setTradeCapacities(o3d::Int32 tradeCaps)
{
    if (tradeCaps > -1) {
        m_tradeCaps = tradeCaps;
    }
}

void Market::setOrderCapacities(o3d::Int32 orderCaps)
{
    if (orderCaps > -1) {
        m_orderCaps = orderCaps;
    }
}

void Market::setBaseInfo(const o3d::CString &symbol, o3d::Int32 precision)
{
    m_base.symbol = symbol.isValid() ? symbol : m_base.symbol;
    m_base.precision = precision;
}

void Market::setQuoteInfo(const o3d::CString &symbol, o3d::Int32 precision)
{
    m_quote.symbol = symbol.isValid() ? symbol : m_quote.symbol;
    m_quote.precision = precision;
}

void Market::setSettlementInfo(const o3d::CString &symbol, o3d::Int32 precision)
{
    m_settlement.symbol = symbol.isValid() ? symbol : m_settlement.symbol;
    m_settlement.precision = precision;
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

void Market::setMakerFee(o3d::Double rate, o3d::Double commission)
{
    m_makerFees.rate = rate;
    m_makerFees.commission = commission;
}

void Market::setMakerFee(o3d::Double rate, o3d::Double commission, const o3d::Double limits[])
{
    m_makerFees.rate = rate;
    m_makerFees.commission = commission;
    m_makerFees.limits[0] = limits[0];
    m_makerFees.limits[1] = limits[1];
}

void Market::setTakerFee(o3d::Double rate, o3d::Double commission)
{
    m_takerFees.rate = rate;
    m_takerFees.commission = commission;
}

void Market::setTakerFee(o3d::Double rate, o3d::Double commission, const o3d::Double limits[])
{
    m_takerFees.rate = rate;
    m_takerFees.commission = commission;
    m_takerFees.limits[0] = limits[0];
    m_takerFees.limits[1] = limits[1];
}

void Market::setState(o3d::Double baseExchangeRate, o3d::Bool tradeable)
{
    m_baseExchangeRate = baseExchangeRate > 0.0 ? baseExchangeRate : m_baseExchangeRate;
    m_tradeable = tradeable;
}

void Market::setPrice(o3d::Double bid, o3d::Double ask, o3d::Double timestamp)
{
    if (bid > 0.0) {
        m_bid = bid;
    }

    if (ask > 0.0) {
        m_ask = ask;
    }

    if (timestamp > 0.0) {
        m_lastTimestamp = timestamp;
    }
}

void Market::setLast(o3d::Double last)
{
    if (last > 0.0) {
        m_last = last;
    }
}

void Market::setLastTick(const Tick &tick)
{
    if (tick.bid() > 0.0) {
        m_bid = tick.bid();
    }

    if (tick.ask() > 0.0) {
        m_ask = tick.ask();
    }

    if (tick.last() > 0.0) {
        m_last = tick.last();
    }

    if (tick.timestamp() > 0.0) {
        m_lastTimestamp = tick.timestamp();
    }
}

void Market::setPriceFilter(const o3d::Double filter[])
{
    m_priceFilter.min = filter[0] >= 0.0 ? filter[0] : m_priceFilter.min;
    m_priceFilter.max = filter[1] >= 0.0 ? filter[1] : m_priceFilter.max;
    m_priceFilter.step = filter[2] >= 0.0 ? filter[2]: m_priceFilter.step;
    m_priceFilter.precision = m_priceFilter.step > 0 ? o3d::max(0, decimalPlace(m_priceFilter.step)) : 0;
}

void Market::setPriceFilter(o3d::Double min, o3d::Double max, o3d::Double step)
{
    m_priceFilter.min = min >= 0.0 ? min : m_priceFilter.min;
    m_priceFilter.max = max >= 0.0 ? max : m_priceFilter.max;
    m_priceFilter.step = step >= 0.0 ? step: m_priceFilter.step;
    m_priceFilter.precision = m_priceFilter.step > 0 ? o3d::max(0, decimalPlace(m_priceFilter.step)) : 0;
}

void Market::setQtyFilter(const o3d::Double filter[])
{
    m_qtyFilter.min = filter[0] >= 0.0 ? filter[0] : m_qtyFilter.min;
    m_qtyFilter.max = filter[1] >= 0.0 ? filter[1] : m_qtyFilter.max;
    m_qtyFilter.step = filter[2] >= 0.0 ? filter[2]: m_qtyFilter.step;
    m_qtyFilter.precision = m_qtyFilter.step > 0 ? o3d::max(0, decimalPlace(m_qtyFilter.step)) : 0;
}

void Market::setQtyFilter(o3d::Double min, o3d::Double max, o3d::Double step)
{
    m_qtyFilter.min = min >= 0.0 ? min : m_qtyFilter.min;
    m_qtyFilter.max = max >= 0.0 ? max : m_qtyFilter.max;
    m_qtyFilter.step = step >= 0.0 ? step: m_qtyFilter.step;
    m_qtyFilter.precision = m_qtyFilter.step > 0 ? o3d::max(0, decimalPlace(m_qtyFilter.step)) : 0;
}

void Market::setNotionalFilter(const o3d::Double filter[])
{
    m_notionalFilter.min = filter[0] >= 0.0 ? filter[0] : m_notionalFilter.min;
    m_notionalFilter.max = filter[1] >= 0.0 ? filter[1] : m_notionalFilter.max;
    m_notionalFilter.step = filter[2] >= 0.0 ? filter[2] : m_notionalFilter.step;
    m_notionalFilter.precision = m_notionalFilter.step > 0 ? o3d::max(0, decimalPlace(m_notionalFilter.step)) : 0;
}

void Market::setNotionalFilter(o3d::Double min, o3d::Double max, o3d::Double step)
{
    m_notionalFilter.min = min >= 0.0 ? min : m_notionalFilter.min;
    m_notionalFilter.max = max >= 0.0 ? max : m_notionalFilter.max;
    m_notionalFilter.step = step >= 0.0 ? step : m_notionalFilter.step;
    m_notionalFilter.precision = m_notionalFilter.step > 0 ? o3d::max(0, decimalPlace(m_notionalFilter.step)) : 0;
}

void Market::setMarginFactor(o3d::Double marginFactor)
{
    if (marginFactor > 0) {
        m_marginFactor = marginFactor;
    }
}

o3d::Double Market::adjustPrice(o3d::Double price) const
{
    if (std::isnan(price)) {
        price = 0.0;
    }

    o3d::Int32 precision = m_priceFilter.precision;
    if (m_priceFilter.step <= 0.0) {
        if (m_onePipMean != 0.0) {
            precision = -static_cast<o3d::Int32>(log10(m_onePipMean));
        } else {
            precision = 8;
        }
    }

    o3d::Double tickSize = m_priceFilter.step;
    if (tickSize <= 0.0) {
        tickSize = 0.00000001;
    }

    return truncate(::round(price / tickSize) * tickSize, precision);
}

o3d::String Market::formatPrice(o3d::Double price, o3d::Bool displaySymbol) const
{
    if (std::isnan(price)) {
        price = 0.0;
    }

    o3d::Int32 precision = m_priceFilter.precision;
    if (m_priceFilter.step <= 0.0) {
        if (m_onePipMean != 0.0) {
            precision = -static_cast<o3d::Int32>(log10(m_onePipMean));
        } else {
            precision = 8;
        }
    }

    o3d::Double tickSize = m_priceFilter.step;
    if (tickSize <= 0.0) {
        tickSize = 0.00000001;
    }

    o3d::String formattedPrice;
    formattedPrice.concat(truncate(::round(price / tickSize) * tickSize, precision), precision);

    if (formattedPrice.find('.') >= 0) {
        formattedPrice.trimRight('0', true);
        formattedPrice.trimRight('.');
    }

    if (!displaySymbol) {
        return formattedPrice;
    }

    return formattedPrice + m_base.symbol;
}

o3d::Double Market::adjustQuotePrice(o3d::Double price) const
{
    if (std::isnan(price)) {
        price = 0.0;
    }

    o3d::Int32 precision = m_notionalFilter.precision;
    if (m_notionalFilter.step <= 0.0) {
        precision = 2;
    }

    o3d::Double tickSize = m_notionalFilter.step;
    if (tickSize <= 0.0) {
        tickSize = 0.01;
    }

    return truncate(::round(price / tickSize) * tickSize, precision);
}

o3d::String Market::formatQuotePrice(o3d::Double price, o3d::Bool displaySymbol) const
{
    if (std::isnan(price)) {
        price = 0.0;
    }

    o3d::Int32 precision = m_notionalFilter.precision;
    if (m_notionalFilter.step <= 0.0) {
        precision = 2;
    }

    o3d::Double tickSize = m_notionalFilter.step;
    if (tickSize <= 0.0) {
        tickSize = 0.01;
    }

    o3d::String formattedPrice;
    formattedPrice.concat(truncate(::round(price / tickSize) * tickSize, precision), precision);

    if (formattedPrice.find('.') >= 0) {
        formattedPrice.trimRight('0', true);
        formattedPrice.trimRight('.');
    }

    if (!displaySymbol) {
        return formattedPrice;
    }

    return formattedPrice + m_quote.symbol;
}

o3d::Double Market::adjustSettlementPrice(o3d::Double price) const
{
    if (std::isnan(price)) {
        price = 0.0;
    }

    o3d::Int32 precision = m_settlement.precision;
    o3d::Double tickSize = pow(10, -precision);

    return truncate(::round(price / tickSize) * tickSize, precision);
}

o3d::String Market::formatSettlementPrice(o3d::Double price, o3d::Bool displaySymbol) const
{
    if (std::isnan(price)) {
        price = 0.0;
    }

    o3d::Int32 precision = m_settlement.precision;
    o3d::Double tickSize = pow(10, -precision);

    o3d::String formattedPrice;
    formattedPrice.concat(truncate(::round(price / tickSize) * tickSize, precision), precision);

    if (formattedPrice.find('.') >= 0) {
        formattedPrice.trimRight('0', true);
        formattedPrice.trimRight('.');
    }

    if (!displaySymbol) {
        return formattedPrice;
    }

    return formattedPrice + m_settlement.symbol;
}

o3d::Double Market::adjustQty(o3d::Double qty, o3d::Bool minIsZero) const
{
    if (std::isnan(qty)) {
        qty = 0.0;
    }

    if (m_qtyFilter.min > 0.0 && qty < m_qtyFilter.min) {
        if (minIsZero) {
            return 0.0;
        }

        return m_qtyFilter.min;
    }

    if (m_qtyFilter.max > 0.0 && qty > m_qtyFilter.max) {
        return m_qtyFilter.max;
    }

    if (m_qtyFilter.step > 0.0) {
        return o3d::max(round(::round(qty / m_qtyFilter.step) * m_qtyFilter.step, m_qtyFilter.precision), m_qtyFilter.min);
    }

    return qty;
}

o3d::String Market::formatQty(o3d::Double qty) const
{
    o3d::Int32 precision = m_qtyFilter.precision;
    if (m_qtyFilter.step <= 0.0) {
        precision = 2;
    }

    o3d::Double step = m_qtyFilter.step;
    if (step <= 0.0) {
        step = 0.01;
    }

    o3d::String formattedQty;
    formattedQty.concat(truncate(::round(qty / step) * step, precision), precision);

    if (formattedQty.find('.') >= 0) {
        formattedQty.trimRight('0', true);
        formattedQty.trimRight('.');
    }

    return formattedQty;
}

o3d::Double Market::marginCost(o3d::Double qty, o3d::Double price) const
{
    o3d::Double realizedPositionCost = 0.0;

    if (qty <= 0.0 || price <= 0.0) {
        return 0.0;
    }

    if (m_unit == UNIT_AMOUNT) {
        // in quote currency
        realizedPositionCost = qty * (m_lotSize * m_contractSize) * price;
    } else if (m_unit == UNIT_CONTRACTS) {
        realizedPositionCost = qty * (m_lotSize * m_contractSize / m_valuePerPip * price);
    } else if (m_unit == UNIT_SHARES) {
        // in quote currency
        realizedPositionCost = qty * price;
    } else if (m_unit == UNIT_INVERSE) {
        // in quote currency
        realizedPositionCost = qty * (m_lotSize * m_contractSize) / price;
    } else {
        // in quote currency
        realizedPositionCost = qty * (m_lotSize * m_contractSize) * price;
    }

    // in account currency
    return realizedPositionCost * m_marginFactor / m_baseExchangeRate;
}

o3d::Double Market::effectiveCost(o3d::Double qty, o3d::Double price) const
{
    if (qty <= 0.0 || price <= 0.0) {
        return 0.0;
    }

    if (m_unit == UNIT_AMOUNT) {
        // in quote currency
        return qty * (m_lotSize * m_contractSize) * price;
    } else if (m_unit == UNIT_CONTRACTS) {
        return qty * (m_lotSize * m_contractSize / m_valuePerPip * price);
    } else if (m_unit == UNIT_SHARES) {
        // in quote currency
        return qty * price;
    } else if (m_unit == UNIT_INVERSE) {
        // in quote currency
        return qty * (m_lotSize * m_contractSize) / price;
    } else {
        // in quote currency
        return qty * (m_lotSize * m_contractSize) * price;
    }
}

o3d::Double Market::computePnl(o3d::Double qty, o3d::Int32 direction,
                               o3d::Double initialPrice, o3d::Double lastPrice) const
{
    if (qty <= 0.0 || initialPrice <= 0.0 || lastPrice <= 0.0 || direction == 0) {
        return 0.0;
    }

    if (m_unit == UNIT_AMOUNT) {
        // in quote or settlement currency
        return qty * (m_lotSize * m_contractSize) * direction * (lastPrice - initialPrice);
    } else if (m_unit == UNIT_CONTRACTS) {
        return qty * (m_lotSize * m_contractSize / m_valuePerPip * direction * (lastPrice - initialPrice));
    } else if (m_unit == UNIT_SHARES) {
        // in quote or settlement currency
        return qty * (lastPrice - initialPrice) * direction;
    } else if (m_unit == UNIT_INVERSE) {
        // in quote or settlement currency
        return qty * (m_lotSize * m_contractSize) * direction * (1.0 / initialPrice - 1.0 / lastPrice);
    } else {
        // in quote or settlement currency
        return qty * (m_lotSize * m_contractSize) * direction * (lastPrice - initialPrice);
    }
}

