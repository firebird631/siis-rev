/**
 * @brief SiiS strategy SuperTrend range-bar.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#ifndef SIIS_SUPERTRENDRB_STRAT_H
#define SIIS_SUPERTRENDRB_STRAT_H

#include "siis/base.h"
#include "siis/strategy.h"

#include "siis/tick.h"
#include "siis/ohlc.h"

#include "siis/indicators/price/price.h"
#include "siis/indicators/volume/volume.h"

#include "siis/analysers/analyser.h"
#include "siis/trade/stdtrademanager.h"

#include "siis/trade/entry.h"
#include "siis/trade/takeprofit.h"
#include "siis/trade/stoploss.h"
#include "siis/trade/breakeven.h"
#include "siis/trade/dynamicstoploss.h"

namespace siis {

class SuperTrendRbTrendAnalyser;
class SuperTrendRbSigAnalyser;
class SuperTrendRbConfAnalyser;

/**
 * @brief Strategy SuperTrend range-bar.
 * @author Frederic Scherma
 * @date 2023-09-30
 */
class SIIS_PLUGIN_API SuperTrendRbStrat : public Strategy
{
public:

    SuperTrendRbStrat(Handler *handler, const o3d::String &identifier);
    virtual ~SuperTrendRbStrat() override;

    virtual void init(Config *config) override;
    virtual void terminate(Connector *connector, Database *db) override;

    virtual void prepareMarketData(Connector *connector, Database *db, o3d::Double fromTs, o3d::Double toTs) override;
    virtual void finalizeMarketData(Connector *connector, Database *db) override;

    virtual void onTickUpdate(o3d::Double timestamp, const TickArray &ticks) override;
    virtual void onOhlcUpdate(o3d::Double timestamp,
                              o3d::Double timeframe,
                              Ohlc::Type ohlcType,
                              const OhlcArray &ohlc) override;

    virtual void onOrderSignal(const OrderSignal &orderSignal) override;
    virtual void onPositionSignal(const PositionSignal &positionSignal) override;

    virtual void prepare(o3d::Double timestamp) override;
    virtual void compute(o3d::Double timestamp) override;
    virtual void finalize(o3d::Double timestamp) override;

    virtual void updateTrade(Trade *trade) override;

    virtual void updateStats() override;

private:

    std::vector<Analyser*> m_analysers;
    StdTradeManager *m_tradeManager;

    SuperTrendRbTrendAnalyser *m_trendAnalyser;
    SuperTrendRbSigAnalyser *m_sigAnalyser;
    SuperTrendRbConfAnalyser *m_confAnalyser;

    TradeSignal m_lastSignal;

    o3d::Double m_profitScale;
    o3d::Double m_riskScale;
    o3d::Double m_minProfit;

    Entry m_entry;
    TakeProfit m_takeProfit;
    StopLoss m_stopLoss;
    Breakeven m_breakeven;
    DynamicStopLoss m_dynamicStopLoss;

    void orderEntry(o3d::Double timestamp,
                    o3d::Int32 barSize,
                    o3d::Int32 direction,
                    Order::OrderType orderType,
                    o3d::Double price,
                    o3d::Double takeProfitPrice,
                    o3d::Double stopLossPrice);

    void orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price);

    TradeSignal computeSignal(o3d::Double timestamp);
};

} // namespace siis

#endif // SIIS_SUPERTRENDRB_STRAT_H
