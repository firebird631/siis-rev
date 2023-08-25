/**
 * @brief SiiS strategy position signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_POSITION_H
#define SIIS_POSITION_H

#include "constants.h"

#include <o3d/core/string.h>

namespace siis {

class TraderProxy;
class Strategy;
class Market;

/**
 * @brief Strategy position model.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API Position
{
public:

    static constexpr o3d::Double TIMESTAMP_UNDEFINED = -1.0;
    static constexpr o3d::Double PRICE_UNDEFINED = -1.0;
    static constexpr o3d::Double QUANTITY_UNDEFINED = -1.0;
    static constexpr o3d::Double RATE_UNDEFINED = O3D_MIN_DOUBLE;
    static constexpr o3d::Int8 FLAG_UNDEFINED = -1;
    static constexpr o3d::Int8 VALUE_UNDEFINED = -1;

    struct Local
    {
        o3d::Double entryPrice;
        o3d::Double entryQty;

        o3d::Double exitPrice;
        o3d::Double exitQty;

        o3d::Double profitLoss;
        o3d::Double rawProfitLoss;
        o3d::Double rawProfitLossRate;
        o3d::Double profitLossRate;
        o3d::Double profitLossMarket;
        o3d::Double profitLossMarketRate;

        Local() :
            entryPrice(0.0),
            entryQty(0.0),
            exitPrice(0.0),
            exitQty(0.0),
            profitLoss(0.0),
            rawProfitLoss(0.0),
            rawProfitLossRate(0.0),
            profitLossRate(0.0),
            profitLossMarket(0.0),
            profitLossMarketRate(0.0)
        {

        }

        void reset() {
            entryPrice = 0.0;
            entryQty = 0.0;

            exitPrice = 0.0;
            exitQty = 0.0;

            profitLoss = 0.0;
            rawProfitLoss = 0.0;
            rawProfitLossRate = 0.0;
            profitLossRate = 0.0;
            profitLossMarket = 0.0;
            profitLossMarketRate = 0.0;
        }
    };

    Position() :
        proxy(nullptr),
        strategy(nullptr),
        created(TIMESTAMP_UNDEFINED),
        updated(TIMESTAMP_UNDEFINED),
        id(-1),
        direction(UNDEFINED),
        quantity(QUANTITY_UNDEFINED),
        avgPrice(PRICE_UNDEFINED),
        execPrice(PRICE_UNDEFINED),
        stopPrice(PRICE_UNDEFINED),
        limitPrice(PRICE_UNDEFINED),
        profitLoss(RATE_UNDEFINED),
        filled(QUANTITY_UNDEFINED),
        cumulativeFilled(QUANTITY_UNDEFINED),
        liquidationPrice(PRICE_UNDEFINED),
        commission(RATE_UNDEFINED)
    {

    }

    void reset()
    {
        strategy = nullptr;
        created = TIMESTAMP_UNDEFINED;
        updated = TIMESTAMP_UNDEFINED;
        id = -1;
        direction = UNDEFINED;
        quantity = QUANTITY_UNDEFINED;
        avgPrice = PRICE_UNDEFINED;
        execPrice = PRICE_UNDEFINED;
        stopPrice = PRICE_UNDEFINED;
        limitPrice = PRICE_UNDEFINED;
        profitLoss = RATE_UNDEFINED;
        filled = QUANTITY_UNDEFINED;
        cumulativeFilled = QUANTITY_UNDEFINED;
        liquidationPrice = PRICE_UNDEFINED;
        commission = RATE_UNDEFINED;
    }

    //
    // helpers
    //

    /**
     * @brief updatePnl Compute profit/loss and profit/loss rate for maker and taker.
     * @param market A valid market object related to the symbol of the position.
     * @todo partial reduce make a RPNL
     * @param market
     */
    void updatePnl(const Market* market);

    TraderProxy *proxy;
    Strategy *strategy;

    o3d::Double created;   //!< creation timestamp
    o3d::Double updated;   //!< last operation timestamp

    o3d::Int32 id;           //!< internal integer unique id

    o3d::CString positionId;  //!< must always be valid
    o3d::CString refOrderId;  //!< reference id of the order that as initied this position

    o3d::CString marketId;    //!< empty means not defined

    o3d::Int32 direction;    //!< -1 or 1, 0 means not defined
    o3d::Double quantity;    //!< always positive or 0

    o3d::Double avgPrice;    //!< average entry price or -1 if not defined
    o3d::Double execPrice;   //!< executed price or -1 if not defined

    o3d::Double stopPrice;       //! -1 if not defined
    o3d::Double limitPrice;      //! -1 if not defined

    o3d::CString profitCurrency;  //!< currency symbol
    o3d::Double profitLoss;       //!< unrealized profit/loss in profit currency

    o3d::Double filled;             //!< filled qty or neg if not defined
    o3d::Double cumulativeFilled;   //!< cumulative filled qty or neg if not defined

    o3d::Double liquidationPrice;   //!< liquidation price, neg if not defined
    o3d::Double commission;         //!< commission rate realized on the position

    Local local;             //!< locally computed value for measures/stats...
};

} // namespace siis

#endif // SIIS_POSITION_H
