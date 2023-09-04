/**
 * @brief SiiS strategy Kahlman + Fibonnaci + Donchian.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#ifndef SIIS_KAHLMANFIBO_H
#define SIIS_KAHLMANFIBO_H

#include "siis/base.h"
#include "siis/strategy.h"

#include "siis/tick.h"
#include "siis/ohlc.h"

#include "siis/indicators/price/price.h"
#include "siis/indicators/volume/volume.h"

#include "siis/analysers/analyser.h"
#include "siis/trade/stdtrademanager.h"

#include "siis/trade/entry.h"
#include "siis/trade/stoploss.h"
#include "siis/trade/breakeven.h"
#include "siis/trade/dynamicstoploss.h"

namespace siis {

class KahlmanFiboSigAnalyser;
class KahlmanFiboConfAnalyser;

/**
 * @brief Strategy Kahlman + Fibonnaci + Donchian..
 * @author Frederic Scherma
 * @date 2023-08-31
 */
class SIIS_PLUGIN_API KahlmanFibo : public Strategy
{
public:

    KahlmanFibo (Handler *handler, const o3d::String &identifier);
    virtual ~KahlmanFibo () override;

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

    KahlmanFiboSigAnalyser *m_sigAnalyser;
    KahlmanFiboConfAnalyser *m_confAnalyser;

    o3d::Double m_lastSig;
    o3d::Double m_lastSigTimestamp;
    o3d::Double m_lastTrend;
    o3d::Double m_lastTrendTimestamp;

    TradeSignal m_lastSignal;

    o3d::Double m_profitScale;
    o3d::Double m_riskScale;
    o3d::Double m_minProfit;     //!< should be per context
    o3d::Bool m_useKahlman;

    o3d::Int32 m_maxWide;   //!< should be per context
    o3d::Bool m_oneWay;     //!< should be per context

    Entry m_entry;
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
};

} // namespace siis

#endif // SIIS_KAHLMANFIBO_H
