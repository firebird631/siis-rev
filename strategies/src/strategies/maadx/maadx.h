/**
 * @brief SiiS strategy MAADX.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_MAADX_H
#define SIIS_MAADX_H

#include "siis/base.h"
#include "siis/strategy.h"

#include "siis/tick.h"
#include "siis/ohlc.h"

#include "siis/indicators/price/price.h"
#include "siis/indicators/volume/volume.h"

#include "siis/analysers/analyser.h"
#include "siis/trade/stdtrademanager.h"
#include "siis/trade/tradesignal.h"

#include "siis/trade/entry.h"
#include "siis/trade/takeprofit.h"
#include "siis/trade/stoploss.h"
#include "siis/trade/breakeven.h"
#include "siis/trade/dynamicstoploss.h"

namespace siis {

class MaAdxSessionAnalyser;
class MaAdxTrendAnalyser;
class MaAdxSigAnalyser;
class MaAdxConfAnalyser;

/**
 * @brief Strategy MAADX.
 * @author Frederic Scherma
 * @date 2023-04-24
 */
class SIIS_PLUGIN_API MaAdx : public Strategy
{
public:

    MaAdx (Handler *handler, const o3d::String &identifier);
    virtual ~MaAdx () override;

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

    static constexpr o3d::Double ADX_MAX = 75.0;

    std::vector<Analyser*> m_analysers;
    StdTradeManager *m_tradeManager;

    MaAdxSessionAnalyser *m_sessionAnalyser;
    MaAdxTrendAnalyser *m_trendAnalyser;
    MaAdxSigAnalyser *m_sigAnalyser;
    MaAdxConfAnalyser *m_confAnalyser;

    TradeSignal m_lastSignal;

    o3d::Double m_adxSig;
    o3d::Double m_targetScale;
    o3d::Double m_riskReward;
    o3d::Double m_minProfit;

    Entry m_entry;
    TakeProfit m_takeProfit;
    StopLoss m_stopLoss;
    Breakeven m_breakeven;
    DynamicStopLoss m_dynamicStopLoss;

    void orderEntry(o3d::Double timestamp,
                    o3d::Double timeframe,
                    o3d::Int32 direction,
                    Order::OrderType orderType,
                    o3d::Double price,
                    o3d::Double takeProfitPrice,
                    o3d::Double stopLossPrice);

    void orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price);

    TradeSignal computeSignal(o3d::Double timestamp);

    o3d::Bool checkVp(o3d::Int32 direction, o3d::Int32 vpUp, o3d::Int32 vpDn) const;
};

} // namespace siis

#endif // SIIS_MAADX_H
