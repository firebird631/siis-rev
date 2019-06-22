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
class SIIS_API MarketSignal : public BaseSignal
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

    enum MarketUnit {
        UNIT_UNDEFINED = -1,
        UNIT_AMOUNT = 0,
        UNIT_CONTRACTS = 1,
        UNIT_SHARES = 2
    };

    enum MarketMode {
        MODE_UNDEFINED = -1,
        MODE_BUY_SELL = 0,  //!< no marginn no short, only buy (hold) and sell
        MODE_MARGIN = 1     //!< margin, long and short
    };

    enum OrderCapacity {
        // or could have a OrderPolicy because not everywhere same concepts
        ORDER_UNDEFINED = -1,
        ORDER_MARKET = 0,
        ORDER_LIMIT = 1,
        ORDER_STOP_MARKET = 2,
        ORDER_STOP_LIMIT = 4,
        ORDER_TAKE_PROFIT_MARKET = 8,
        ORDER_TAKE_PROFIT_LIMIT = 16,
        ORDER_ALL = 32-1
    };

    static constexpr o3d::Double EXPIRY_NEVER = 0.0;

    static constexpr o3d::Int32 MIN = 0;   //!< for price and notional filter
    static constexpr o3d::Int32 MAX = 1;   //!< for price and notional filter
    static constexpr o3d::Int32 STEP = 2;  //!< for price and notional filter

    static constexpr o3d::Int32 MAKER = 0;   //!< for fees
    static constexpr o3d::Int32 TAKER = 1;   //!< for fees

    MarketSignal(Event _event) :
        BaseSignal(MARKET),
        event(_event),
        timestamp(TIMESTAMP_UNDEFINED),
        open(FLAG_UNDEFINED),
        marketType(TYPE_UNDEFINED),
        unit(UNIT_UNDEFINED),
        mode(MODE_UNDEFINED),
        hedging(FLAG_UNDEFINED),
        expiry(EXPIRY_NEVER),
        contractSize(QUANTITY_UNDEFINED),
        lotSize(QUANTITY_UNDEFINED),
        baseExchangeRate(RATE_UNDEFINED),
        valuePerPip(PRICE_UNDEFINED),
        onePipMeans(PRICE_UNDEFINED),
        marginFactor(RATE_UNDEFINED),
        priceFilter{PRICE_UNDEFINED, PRICE_UNDEFINED, PRICE_UNDEFINED},
        notionalFilter{PRICE_UNDEFINED, PRICE_UNDEFINED, PRICE_UNDEFINED},
        bid(PRICE_UNDEFINED),
        ofr(PRICE_UNDEFINED)
    {
    }

    struct Part
    {
        o3d::String symbol;    //!< symbol
        o3d::Int32 precision;  //!< precision of the decimal
        o3d::Double vol24h;    //!< UTC 24h volume change

        Part() :
            precision(VALUE_UNDEFINED),
            vol24h(QUANTITY_UNDEFINED)
        {
        }
    };

    struct Fee
    {
        o3d::Double rate;         //!< rate part (for example 0.1% of fee is 0.001 rate).
        o3d::Double commission;   //!< for a fixed additionnal commission part (for example 1$ of fixed fee is 1.0).

        o3d::Double limits[2];  //! 0 min, 1 max

        Fee() :
            rate(RATE_UNDEFINED),
            commission(PRICE_UNDEFINED)
        {
        }
    };

    Event event;
    o3d::Double timestamp;

    o3d::Int32 id;    //!< internal unique integer id

    o3d::String marketId;
    o3d::Int8 open;   //!< 1 if the market is tradable/open, 0 mean currently closed/delayed.

    MarketType marketType;
    MarketUnit unit;
    MarketMode mode;

    o3d::Int8 hedging;

    OrderCapacity orderCaps;

    o3d::Double expiry;   //!< expiry UTC futur timestamp (0 for never, -1 if not defined)

    Part base;   //!< details on the base symbol
    Part quote;  //!< details on the quote symbol

    o3d::Double contractSize;
    o3d::Double lotSize;
    o3d::Double baseExchangeRate;  //!< is it usefull on strategy side ?

    o3d::Double valuePerPip;
    o3d::Double onePipMeans;

    o3d::Double marginFactor;   //!< 1.0 / leverage

    o3d::Double priceFilter[3];     //!< min/max/step for price filter
    o3d::Double notionalFilter[3];  //!< min/max/step for notional filter

    o3d::Double bid;
    o3d::Double ofr;

    Fee fees[2];
};

} // namespace siis

#endif // SIIS_MARKETSIGNAL_H
