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
 */
class SIIS_API Market
{
public:

    enum Mode
    {
        MODE_BUY_SELL = 0,    //!< Asset buy/sell market.
        MODE_MARGIN = 1,      //!< Multiple position possible per market (hedging is another detail).
        MODE_IND_MARGIN = 2   //!< Indivisible margin position.
    };

    enum OrderCapacity
    {
        ORDER_MARKET = 0,
        ORDER_LIMIT = 1,
        ORDER_STOP_MARKET = 2,
        ORDER_STOP_LIMIT = 4,
        ORDER_TAKE_PROFIT_MARKET = 8,
        ORDER_TAKE_PROFIT_LIMIT = 16,
        ORDER_ALL = 32-1
    };

    enum Contract
    {
        CONTRACT_SPOT = 0,
        CONTRACT_CFD = 1,
        CONTRACT_FUTUR = 2,
        CONTRACT_OPTION = 3,
        CONTRACT_WARRANT = 4,
        CONTRACT_TURBO = 5
    };

    enum Type
    {
        TYPE_UNKNOWN = 0,
        TYPE_CURRENCY = 1,   //!< FOREX
        TYPE_CRYPTO = 2,
        TYPE_STOCK = 3,
        TYPE_COMMODITY = 4,
        TYPE_INDICE = 5
    };

    enum Unit
    {
        UNIT_UNKNOWN = 0,
        UNIT_CONTRACT = 1,    //!< amount is in contract size
        UNIT_CURRENCY = 2,    //!< amount is in currency (quote)
        UNIT_LOT = 3,         //!< unit of one lot, with 1 lot = 100000 of related currency
    };

    struct Part
    {
        o3d::String symbol;    //!< symbol
        o3d::Int32 precision;  //!< precision of the decimal
        o3d::Double vol24h;    //!< UTC 24h volume change

        Part() :
            precision(8),
            vol24h(0.0)
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
           const o3d::String &symbol,
           const o3d::String &baseSymbol,
           const o3d::String &quoteSymbol);

    ~Market();

    const o3d::String& marketId() const { return m_marketId; }
    const o3d::String& symbol() const { return m_symbol; }

    const Part& base() const { return m_base; }
    const Part& quote() const { return m_quote; }

    o3d::Bool tradeable() const { return m_tradeable; }

    o3d::Double lastTimestamp() const { return m_lastTimestamp; }

    o3d::Double contractSize() const { return m_contractSize; }
    o3d::Double lotSize() const { return m_lotSize; }
    o3d::Double valuePerPip() const { return m_valuePerPip; }
    o3d::Double onePipMean() const { return m_onePipMean; }

    o3d::Double baseExchangeRate() const { return m_baseExchangeRate; }
    o3d::Double marginFactor() const { return m_marginFactor; }

    o3d::Double bid() const { return m_bid; }
    o3d::Double ofr() const { return m_ofr; }
    o3d::Double mid() const { return (m_bid + m_ofr) * 0.5; }
    o3d::Double spread() const { return m_ofr - m_bid; }

    o3d::Bool hedging() const { return m_hedging; }

    const Fee& makerFee() const { return m_fees[0]; }
    const Fee& takerFee() const { return m_fees[1]; }

    o3d::Double minQty() const { return m_qtyFilter[0]; }
    o3d::Double maxQty() const { return m_qtyFilter[1]; }
    o3d::Double stepQty() const { return m_qtyFilter[2]; }

    o3d::Double minNotional() const { return m_notionalFilter[0]; }
    o3d::Double maxNotional() const { return m_notionalFilter[1]; }
    o3d::Double stepNotional() const { return m_notionalFilter[2]; }

    /**
     * @brief mode Buy/sell or margin market.
     */
    Mode mode() const { return m_mode; }

    /**
     * @brief orderCaps Order capacities (or'ed values).
     */
    o3d::Int32 orderCaps() const { return m_orderCaps; }

    //
    // setters/update
    //

    void acquire() const { m_mutex.lock(); }
    void release() const { m_mutex.unlock(); }

    void setModeAndOrders(Mode mode, o3d::Int32 orders);

    void setBaseInfo(const o3d::String &symbol, o3d::Int32 precision);
    void setQuoteInfo(const o3d::String &symbol, o3d::Int32 precision);

    void setBaseVol24h(o3d::Double vol24h);
    void setQuoteVol24h(o3d::Double vol24h);

    void setType(Type type, Contract contract, Unit unit);
    void setDetails(o3d::Double contractSize, o3d::Double lotSize, o3d::Double valuePerPip, o3d::Double onePipMean, o3d::Bool hedging);

    void setMakerFee(o3d::Double rate, o3d::Double commission, const o3d::Double limits[2]);
    void setTakerFee(o3d::Double rate, o3d::Double commission, const o3d::Double limits[2]);

    void setState(o3d::Double baseExchangeRate, o3d::Bool tradeable);
    void setPrice(o3d::Double bid, o3d::Double ofr, o3d::Double timestamp);

    void setPriceFilter(const o3d::Double filter[3]);  //!< min,max,step
    void setQtyFilter(const o3d::Double filter[3]);  //!< min,max,step
    void setNotionalFilter(const o3d::Double filter[3]);  //!< min,max,step

    //
    // helpers
    //

    /**
     * @brief openExecPrice Return the execution price if an order open a position.
     * @param direction
     * @return Execution price if an order open a position.
     * It depend of the direction of the order and the market bid/ofr prices.
     * If position is long, then returns the market ofr price.
     * If position is short, then returns the market bid price.
     */
    o3d::Double openExecPrice(o3d::Int32 direction) const
    {
        if (direction > 0) {
            return m_ofr;
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
     * It depend of the direction of the order and the market bid/ofr prices.
     * If position is long, then returns the market bid price.
     * If position is short, then returns the market ofr price.
     */
    o3d::Double closeExecPrice(o3d::Int32 direction) const
    {
        if (direction > 0) {
            return m_bid;
        } else if (direction < 0) {
            return m_ofr;
        } else {
            return 0.0;
        }
    }

    /**
     * @brief formatPrice Format the price according to the precision.
     * @param price
     * @param useQuote True use quote display or quote, False base, None no symbol only price.
     * @param displaySymbol Append the unit symbol.
     * @return
     */
    o3d::String formatPrice(o3d::Double price, o3d::Bool useQuote=true, o3d::Bool displaySymbol=false) const;

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
    o3d::Double marginCost(o3d::Double qty) const;

    //
    // processing
    //

    /**
     * @brief lastTicks Return the last received array of ticks for reads.
     */
    const TickArray& lastTicks() { return m_ticks; }

    /**
     * @brief lastOhlc Return the last received array of OHLC for a specified type (mid, bid or ofr) for reads.
     * @param ohlcType Mid, bid or ofr.
     */
    const OhlcArray& lastOhlc(Ohlc::Type ohlcType) { return m_ohlcs[ohlcType]; }

    /**
     * @brief getTicks Get the tick buffer for writting from the handler/connector or any other original source.
     */
    TickArray& getTickBuffer() { return m_ticks; }

    /**
     * @brief swapOhlc Get the OHLC buffer fro writting from the handler/connector or any other original source.
     * @param ohlcType Mid, bid or ofr.
     */
    OhlcArray& getOhlcBuffer(Ohlc::Type ohlcType) { return m_ohlcs[ohlcType]; }

private:

    o3d::FastMutex m_mutex;

    o3d::String m_marketId;
    o3d::String m_symbol;

    Type m_type;
    Contract m_contract;
    Unit m_unit;

    Mode m_mode;
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
    o3d::Double m_ofr;

    o3d::Bool m_hedging;

    Part m_base;
    Part m_quote;

    Fee m_fees[2];

    o3d::Double m_priceFilter[3];     //!< min/max/step on base price
    o3d::Double m_qtyFilter[3];       //!< min/max/step on base quantity
    o3d::Double m_notionalFilter[3];  //!< min/max/step on notional price

    TickArray m_ticks;
    OhlcArray m_ohlcs[Ohlc::NUM_TYPE];  //!< last ohlcs buffers (one per type but single source timeframe)
};

} // namespace siis

#endif // SIIS_MARKET_H
