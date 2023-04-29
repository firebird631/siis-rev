/**
 * @brief SiiS strategy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_MARKET_H
#define SIIS_MARKET_H

#include "base.h"
#include "constants.h"

#include <cmath>

#include <o3d/core/string.h>
#include "tick.h"
#include "ohlc.h"

namespace siis {

/**
 * @brief Strategy market model.
 * @author Frederic Scherma
 * @date 2019-03-07
 * @todo add sessions
 */
class SIIS_API Market
{
public:

    enum MonthCode
    {
        MC_F = 1,  //!< january...
        MC_G = 2,
        MC_H = 3,
        MC_J = 4,
        MC_K = 5,
        MC_M = 6,
        MC_N = 7,
        MC_Q = 8,
        MC_U = 9,
        MC_V = 10,
        MC_X = 11,
        MC_Z = 12
    };

    static const o3d::Int32 TRADE_BUY_SELL = 1;     //!< no margin no short, only buy (hold) and sell
    static const o3d::Int32 TRADE_ASSET = 1;        //!< synonym for buy-sell/spot
    static const o3d::Int32 TRADE_SPOT = 1;         //!< synonym for buy-sell/spot
    static const o3d::Int32 TRADE_MARGIN = 2;       //!< margin, long and short
    static const o3d::Int32 TRADE_IND_MARGIN = 4;   //!< indivisible position, margin, long and short
    static const o3d::Int32 TRADE_FIFO = 8;         //!< position are closed in FIFO order
    static const o3d::Int32 TRADE_POSITION = 16;    //!< individual position on the broker side

    static const o3d::Int32 ORDER_MARKET = 0;
    static const o3d::Int32 ORDER_LIMIT = 1;
    static const o3d::Int32 ORDER_STOP_MARKET = 2;
    static const o3d::Int32 ORDER_STOP_LIMIT = 4;
    static const o3d::Int32 ORDER_TAKE_PROFIT_MARKET = 8;
    static const o3d::Int32 ORDER_TAKE_PROFIT_LIMIT = 16;
    static const o3d::Int32 ORDER_ALL = 32-1;

    enum Contract
    {
        CONTRACT_SPOT = 0,
        CONTRACT_CFD = 1,
        CONTRACT_FUTURE = 2,
        CONTRACT_OPTION = 3,
        CONTRACT_WARRANT = 4,
        CONTRACT_TURBO = 5
    };

    enum Type
    {
        TYPE_UNDEFINED = -1,
        TYPE_UNKNOWN = 0,
        TYPE_CURRENCY = 1,   //!< FOREX
        TYPE_COMMODITY = 2,
        TYPE_INDICE = 3,
        TYPE_STOCK = 4,
        TYPE_RATE = 5,
        TYPE_SECTOR = 6,
        TYPE_CRYPTO = 7
    };

    enum Unit
    {
        UNIT_UNDEFINED = -1,
        UNIT_AMOUNT = 0,      //!< amount is in number of token
        UNIT_CONTRACTS = 1,   //!< amount is in contract size
        UNIT_SHARES = 2       //!< amount is in currency (quote)
    };

    struct Symbol
    {
        o3d::String symbol;    //!< symbol
        o3d::Int32 precision;  //!< precision of the decimal
        o3d::Double vol24h;    //!< UTC 24h volume change

        Symbol() :
            precision(8),
            vol24h(0.0)
        {
        }
    };

    struct Filter
    {
        o3d::Double min;
        o3d::Double max;
        o3d::Double step;
        o3d::Int32 precision;

        Filter() :
            min(0.0),
            max(0.0),
            step(0.0),
            precision(0)
        {
        }
    };

    struct Fee
    {
        o3d::Double rate;         //!< rate part (for example 0.1% of fee is 0.001 rate), can be negative.
        o3d::Double commission;   //!< for a fixed additionnal commission part (for example 1$ of fixed fee is 1.0).

        o3d::Double limits[2];    //! 0 min, 1 max

        Fee() :
            rate(0.0),
            commission(0.0),
            limits{0.0, 0.0}
        {
        }
    };

    Market(const o3d::String &marketId,
           const o3d::String &pair,
           const o3d::String &baseSymbol,
           const o3d::String &quoteSymbol);

    ~Market();

    const o3d::String& marketId() const { return m_marketId; }
    const o3d::String& pair() const { return m_pair; }
    const o3d::String& alias() const { return m_alias; }

    const Symbol& base() const { return m_base; }
    const Symbol& quote() const { return m_quote; }
    const Symbol& settlement() const { return m_settlement; }

    o3d::Bool tradeable() const { return m_tradeable; }

    o3d::Double lastTimestamp() const { return m_lastTimestamp; }

    o3d::Double contractSize() const { return m_contractSize; }
    o3d::Double lotSize() const { return m_lotSize; }
    o3d::Double valuePerPip() const { return m_valuePerPip; }
    o3d::Double onePipMean() const { return m_onePipMean; }

    o3d::Double baseExchangeRate() const { return m_baseExchangeRate; }
    o3d::Double marginFactor() const { return m_marginFactor; }

    o3d::Double bid() const { return m_bid; }
    o3d::Double ofr() const { return m_ask; }
    o3d::Double mid() const { return (m_bid + m_ask) * 0.5; }
    o3d::Double spread() const { return m_ask - m_bid; }

    o3d::Bool hedging() const { return m_hedging; }

    const Fee& makerFee() const { return m_makerFees; }
    const Fee& takerFee() const { return m_takerFees; }

    o3d::Double minQty() const { return m_qtyFilter.min; }
    o3d::Double maxQty() const { return m_qtyFilter.max; }
    o3d::Double stepQty() const { return m_qtyFilter.step; }
    o3d::Double precisionQty() const { return m_qtyFilter.precision; }

    o3d::Double minPrice() const { return m_priceFilter.min; }
    o3d::Double maxPrice() const { return m_priceFilter.max; }
    o3d::Double stepPrice() const { return m_priceFilter.step; }
    o3d::Double precisionPrice() const { return m_priceFilter.precision; }

    o3d::Double minNotional() const { return m_notionalFilter.min; }
    o3d::Double maxNotional() const { return m_notionalFilter.max; }
    o3d::Double stepNotional() const { return m_notionalFilter.step; }
    o3d::Double precisionNotional() const { return m_notionalFilter.precision; }

    /**
     * @brief trade Trade capacities (spot, margin...)
     */
    o3d::Int32 tradeCaps() const { return m_tradeCaps; }

    /**
     * @brief hasSpot Has spot market capacity.
     */
    o3d::Bool hasSpot() const { return m_tradeCaps & TRADE_BUY_SELL; }

    /**
     * @brief hasMargin Has margin market capacity.
     */
    o3d::Bool hasMargin() const { return m_tradeCaps & TRADE_MARGIN; }

    /**
     * @brief hasSpot Has spot market capacity.
     */
    o3d::Bool indivisiblePosition() const { return m_tradeCaps & TRADE_IND_MARGIN; }

    /**
     * @brief hasSpot Has spot market capacity.
     */
    o3d::Bool fifoPosition() const { return m_tradeCaps & TRADE_FIFO; }

    /**
     * @brief hasSpot Has spot market capacity.
     */
    o3d::Bool hasPosition() const { return m_tradeCaps & TRADE_POSITION; }

    /**
     * @brief orderCaps Order capacities (or'ed values).
     */
    o3d::Int32 orderCaps() const { return m_orderCaps; }

    //
    // setters/update
    //

    void acquire() const { m_mutex.lock(); }
    void release() const { m_mutex.unlock(); }

    void setPair(const o3d::String &pair);
    void setAlias(const o3d::String &alias);

    void setCapacities(o3d::Int32 tradeCaps, o3d::Int32 orderCaps);
    void setTradeCapacities(o3d::Int32 tradeCaps);
    void setOrderCapacities(o3d::Int32 orderCaps);

    void setBaseInfo(const o3d::String &symbol, o3d::Int32 precision);
    void setQuoteInfo(const o3d::String &symbol, o3d::Int32 precision);
    void setSettlementInfo(const o3d::String &symbol, o3d::Int32 precision);

    void setBaseVol24h(o3d::Double vol24h);
    void setQuoteVol24h(o3d::Double vol24h);

    void setType(Type type, Contract contract, Unit unit);
    void setDetails(o3d::Double contractSize, o3d::Double lotSize, o3d::Double valuePerPip, o3d::Double onePipMean, o3d::Bool hedging);

    void setMakerFee(o3d::Double rate, o3d::Double commission, const o3d::Double limits[2]);
    void setTakerFee(o3d::Double rate, o3d::Double commission, const o3d::Double limits[2]);

    void setState(o3d::Double baseExchangeRate, o3d::Bool tradeable);
    void setPrice(o3d::Double bid, o3d::Double ask, o3d::Double timestamp);

    void setPriceFilter(const o3d::Double filter[3]);     //!< min,max,step
    void setQtyFilter(const o3d::Double filter[3]);       //!< min,max,step
    void setNotionalFilter(const o3d::Double filter[3]);  //!< min,max,step

    //
    // helpers
    //

    /**
     * @brief openExecPrice Return the execution price if an order open a position.
     * @param direction
     * @return Execution price if an order open a position.
     * It depend of the direction of the order and the market bid/ask prices.
     * If position is long, then returns the market ask price.
     * If position is short, then returns the market bid price.
     */
    o3d::Double openExecPrice(o3d::Int32 direction) const
    {
        if (direction > 0) {
            return m_ask;
        } else if (direction < 0) {
            return m_bid;
        } else {
            return 0.0;
        }
    }

    /**
     * @brief closeExecPrice Return the execution price if an order/position is closing.
     * @param direction
     * @return Execution price if an order/position is closing.
     * It depend of the direction of the order and the market bid/ask prices.
     * If position is long, then returns the market bid price.
     * If position is short, then returns the market ask price.
     */
    o3d::Double closeExecPrice(o3d::Int32 direction) const
    {
        if (direction > 0) {
            return m_bid;
        } else if (direction < 0) {
            return m_ask;
        } else {
            return 0.0;
        }
    }

    /**
     * @brief adjustPrice Adjust the price according to the precision.
     * @param price
     * @param displaySymbol Append the unit symbol.
     * @return
     */
    o3d::Double adjustPrice(o3d::Double price) const;

    /**
     * @brief formatPrice Format the price according to the precision.
     * @param price
     * @param displaySymbol Append the unit symbol.
     * @return
     */
    o3d::String formatPrice(o3d::Double price, o3d::Bool displaySymbol=false) const;

    /**
     * @brief adjustQuotePrice Adjust the quote price according to the precision.
     * @param price
     * @param displaySymbol Append the unit symbol.
     * @return
     */
    o3d::Double adjustQuotePrice(o3d::Double price) const;

    /**
     * @brief formatQuotePrice Format the quote price according to the precision.
     * @param price
     * @param displaySymbol Append the unit symbol.
     * @return
     */
    o3d::String formatQuotePrice(o3d::Double price, o3d::Bool displaySymbol=false) const;

    /**
     * @brief adjustSettlementPrice Adjust the settlement price according to the precision.
     * @param price
     * @param useQuote True use quote display or quote, False base, None no symbol only price.
     * @param displaySymbol Append the unit symbol.
     * @return
     */
    o3d::Double adjustSettlementPrice(o3d::Double price) const;

    /**
     * @brief formatSettlementPrice Format the settlement price according to the precision.
     * @param price
     * @param useQuote True use quote display or quote, False base, None no symbol only price.
     * @param displaySymbol Append the unit symbol.
     * @return
     */
    o3d::String formatSettlementPrice(o3d::Double price, o3d::Bool displaySymbol=false) const;

    /**
     * @brief adjustQty From quantity return the floor tradable quantity according to min, max and rounded to step size.
     * @param qty Quantity to adjust
     * @param minIsZero Default true. If quantity is lesser than min returns 0 else return min size.
     * @return
     * To make a precise value for trade use format_value from this returned value.
     */
    o3d::Double adjustQty(o3d::Double qty, o3d::Bool minIsZero=true) const;

    /**
     * @brief formatQty Return a quantity as str according to the precision of the step size.
     */
    o3d::String formatQty(o3d::Double qty) const;

    /**
     * @brief marginCost Compute and return the margin cost according to the given quantity and market details.
     */
    o3d::Double marginCost(o3d::Double qty, o3d::Double price) const;

    /**
     * @brief effectiveCost Compute and return the cost without the margin factor,
     * according to the given quantity and market details.
     */
    o3d::Double effectiveCost(o3d::Double qty, o3d::Double price) const;

    //
    // processing
    //

    /**
     * @brief lastTicks Return the last received array of ticks for reads.
     */
    const TickArray& lastTicks() { return m_ticks; }

    /**
     * @brief lastOhlc Return the last received array of OHLC for a specified type (mid, bid or ask) for reads.
     * @param ohlcType Mid, bid or ask.
     */
    const OhlcArray& lastOhlc(Ohlc::Type ohlcType) { return m_ohlcs[ohlcType]; }

    /**
     * @brief getTicks Get the tick buffer for writting from the handler/connector or any other original source.
     */
    TickArray& getTickBuffer() { return m_ticks; }

    /**
     * @brief swapOhlc Get the OHLC buffer fro writting from the handler/connector or any other original source.
     * @param ohlcType Mid, bid or ask.
     */
    OhlcArray& getOhlcBuffer(Ohlc::Type ohlcType) { return m_ohlcs[ohlcType]; }

private:

    o3d::FastMutex m_mutex;

    o3d::String m_marketId;
    o3d::String m_pair;
    o3d::String m_alias;

    Type m_type;
    Contract m_contract;
    Unit m_unit;

    o3d::Int32 m_tradeCaps;
    o3d::Int32 m_orderCaps;

    o3d::Double m_lastTimestamp;

    o3d::Bool m_tradeable;

    o3d::Double m_contractSize;
    o3d::Double m_lotSize;
    o3d::Double m_valuePerPip;
    o3d::Double m_onePipMean;     //!< equiv to tick size

    o3d::Double m_baseExchangeRate;
    o3d::Double m_marginFactor;

    o3d::Double m_bid;
    o3d::Double m_ask;

    o3d::Bool m_hedging;

    Symbol m_base;
    Symbol m_quote;
    Symbol m_settlement;

    Fee m_makerFees;
    Fee m_takerFees;

    Filter m_priceFilter;     //!< min/max/step on base price
    Filter m_qtyFilter;       //!< min/max/step on base quantity
    Filter m_notionalFilter;  //!< min/max/step on notional price

    TickArray m_ticks;
    OhlcArray m_ohlcs[Ohlc::NUM_TYPE];  //!< last ohlcs buffers (one per type but single source timeframe)
};

} // namespace siis

#endif // SIIS_MARKET_H
