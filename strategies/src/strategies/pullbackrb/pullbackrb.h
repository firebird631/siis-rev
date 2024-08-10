/**
 * @brief SiiS strategy pullback range-bar.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#ifndef SIIS_PULLBACKRB_H
#define SIIS_PULLBACKRB_H

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

class PullbackRbSessionAnalyser;
class PullbackRbSRAnalyser;
class PullbackRbBBAnalyser;
class PullbackRbConfAnalyser;

/**
 * @brief Strategy pullback range-bar.
 * @author Frederic Scherma
 * @date 2023-05-09
 */
class SIIS_PLUGIN_API PullbackRb : public Strategy
{
public:

    PullbackRb(Handler *handler, const o3d::String &identifier);
    virtual ~PullbackRb() override;

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

    PullbackRbSessionAnalyser *m_sessionAnalyser;
    PullbackRbSRAnalyser *m_srAnalyser;
    PullbackRbBBAnalyser *m_bbAnalyser;
    PullbackRbConfAnalyser *m_confAnalyser;

    TradeSignal m_lastSignal;

    o3d::Bool m_confirmAtClose;
    o3d::Double m_targetScale;
    o3d::Double m_riskReward;
    o3d::Double m_minProfit;

    Entry m_entry;
    StopLoss m_stopLoss;
    TakeProfit m_takeProfit;
    Breakeven m_breakeven;
    DynamicStopLoss m_dynamicStopLoss;

    o3d::Double m_breakoutTimestamp;
    o3d::Int32 m_breakoutDirection;
    o3d::Double m_breakoutPrice;
    o3d::Double m_integrateTimestamp;
    o3d::Int32 m_integrateDirection;

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

#endif // SIIS_PULLBACKRB_H
