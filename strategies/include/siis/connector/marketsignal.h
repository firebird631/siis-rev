/**
 * @brief SiiS strategy market info signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-21
 */

#ifndef SIIS_MARKETSIGNAL_H
#define SIIS_MARKETSIGNAL_H

#include "basesignal.h"
#include "../constants.h"

#include <o3d/core/string.h>

namespace siis {

/**
 * @brief Strategy market info signal proxy.
 * @author Frederic Scherma
 * @date 2019-03-21
 */
class SIIS_API MarketSignal : public BaseSignal<o3d::NullClass>
{
public:

    enum Event{
        UPDATED = 0
    };

    enum MarketType {
        TYPE_UNDEFINED = -1,
        TYPE_UNKNOWN = 0,
        TYPE_CURRENCY = 1,
        TYPE_COMMODITY = 2,
        TYPE_INDICE = 3,
        TYPE_STOCK = 4,
        TYPE_RATE = 5,
        TYPE_SECTOR = 6,
        TYPE_CRYPTO = 7
    };

    enum MarketContract
    {
        CONTRACT_UNDEFINED = -1,
        CONTRACT_SPOT = 0,
        CONTRACT_CFD = 1,
        CONTRACT_FUTURE = 2,
        CONTRACT_OPTION = 3,
        CONTRACT_WARRANT = 4,
        CONTRACT_TURBO = 5
    };

    enum MarketUnit {
        UNIT_UNDEFINED = -1,
        UNIT_AMOUNT = 0,
        UNIT_CONTRACTS = 1,
        UNIT_SHARES = 2
    };

    static const o3d::Int32 TRADE_UNDEFINED = -1;
    static const o3d::Int32 TRADE_BUY_SELL = 1;     //!< no margin no short, only buy (hold) and sell
    static const o3d::Int32 TRADE_ASSET = 1;        //!< synonym for buy-sell/spot
    static const o3d::Int32 TRADE_SPOT = 1;         //!< synonym for buy-sell/spot
    static const o3d::Int32 TRADE_MARGIN = 2;       //!< margin, long and short
    static const o3d::Int32 TRADE_IND_MARGIN = 4;   //!< indivisible position, margin, long and short
    static const o3d::Int32 TRADE_FIFO = 8;         //!< position are closed in FIFO order
    static const o3d::Int32 TRADE_POSITION = 16;    //!< individual position on the broker side

    static const o3d::Int32 ORDER_UNDEFINED = -1;
    static const o3d::Int32 ORDER_MARKET = 0;
    static const o3d::Int32 ORDER_LIMIT = 1;
    static const o3d::Int32 ORDER_STOP_MARKET = 2;
    static const o3d::Int32 ORDER_STOP_LIMIT = 4;
    static const o3d::Int32 ORDER_TAKE_PROFIT_MARKET = 8;
    static const o3d::Int32 ORDER_TAKE_PROFIT_LIMIT = 16;
    static const o3d::Int32 ORDER_ALL = 32-1;

    static constexpr o3d::Double EXPIRY_NEVER = 0.0;

    static constexpr o3d::Int32 MIN = 0;   //!< for price and notional filter
    static constexpr o3d::Int32 MAX = 1;   //!< for price and notional filter
    static constexpr o3d::Int32 STEP = 2;  //!< for price and notional filter

    MarketSignal(Event _event) :
        BaseSignal(MARKET),
        event(_event),
        timestamp(TIMESTAMP_UNDEFINED),
        open(FLAG_UNDEFINED),
        marketType(TYPE_UNDEFINED),
        marketUnit(UNIT_UNDEFINED),
        tradeCaps(TRADE_UNDEFINED),
        hedging(FLAG_UNDEFINED),
        expiry(EXPIRY_NEVER),
        contractSize(QUANTITY_UNDEFINED),
        lotSize(QUANTITY_UNDEFINED),
        baseExchangeRate(RATE_UNDEFINED),
        valuePerPip(PRICE_UNDEFINED),
        onePipMeans(PRICE_UNDEFINED),
        marginFactor(RATE_UNDEFINED),
        priceFilter{PRICE_UNDEFINED, PRICE_UNDEFINED, PRICE_UNDEFINED},
        qtyFilter{QUANTITY_UNDEFINED, QUANTITY_UNDEFINED, QUANTITY_UNDEFINED},
        notionalFilter{PRICE_UNDEFINED, PRICE_UNDEFINED, PRICE_UNDEFINED},
        bid(PRICE_UNDEFINED),
        ask(PRICE_UNDEFINED)
    {
    }

    struct Symbol
    {
        o3d::String symbol;    //!< symbol
        o3d::Int32 precision;  //!< precision of the decimal
        o3d::Double vol24h;    //!< UTC 24h volume change

        Symbol() :
            precision(VALUE_UNDEFINED),
            vol24h(QUANTITY_UNDEFINED)
        {
        }
    };

    struct Fee
    {
        o3d::Double rate;         //!< rate part (for example 0.1% of fee is 0.001 rate), can be negative.
        o3d::Double commission;   //!< for a fixed additionnal commission part (for example 1$ of fixed fee is 1.0).

        o3d::Double limits[2];    //! 0 min, 1 max

        Fee() :
            rate(RATE_UNDEFINED),
            commission(PRICE_UNDEFINED),
            limits{RATE_UNDEFINED, RATE_UNDEFINED}
        {
        }
    };

    Event event;
    o3d::Double timestamp;

    o3d::Int32 id;    //!< internal unique integer id

    o3d::String marketId;
    o3d::String pair;
    o3d::String alias;

    o3d::Int8 open;   //!< 1 if the market is tradable/open, 0 mean currently closed/delayed.

    MarketType marketType;
    MarketUnit marketUnit;
    MarketContract marketContract;

    o3d::Int32 tradeCaps;
    o3d::Int8 hedging;

    o3d::Int32 orderCaps;

    o3d::Double expiry;   //!< expiry UTC futur timestamp (0 for never, -1 if not defined)

    Symbol base;        //!< details on the base symbol
    Symbol quote;       //!< details on the quote symbol
    Symbol settlement;  //!< details on the settlement symbol

    o3d::Double contractSize;
    o3d::Double lotSize;
    o3d::Double baseExchangeRate;  //!< is it usefull on strategy side ?

    o3d::Double valuePerPip;
    o3d::Double onePipMeans;

    o3d::Double marginFactor;   //!< 1.0 / leverage

    o3d::Double priceFilter[3];     //!< min/max/step for price filter
    o3d::Double qtyFilter[3];       //!< min/max/step for base quantity filter
    o3d::Double notionalFilter[3];  //!< min/max/step for notional filter

    o3d::Double bid;
    o3d::Double ask;

    Fee takerFees;
    Fee makerFees;
};

} // namespace siis

#endif // SIIS_MARKETSIGNAL_H
