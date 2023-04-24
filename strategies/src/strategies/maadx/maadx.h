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

namespace siis {

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

    virtual void prepareMarketData(Connector *connector, Database *db) override;
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

private:

    o3d::Bool m_reversal;
    o3d::Bool m_pyramided;
    o3d::Bool m_hedging;
    o3d::Int32 m_maxTrades;
    o3d::Double m_tradeDelay;
    o3d::Bool m_needUpdate;

    o3d::Double m_baseTimeframe;

    std::vector<Analyser*> m_analysers;
    StdTradeManager *m_tradeManager;

    void orderEntry(o3d::Double timestamp,
                    o3d::Double timeframe,
                    o3d::Int32 direction,
                    o3d::Double price,
                    o3d::Double limitPrice,
                    o3d::Double stopPrice);

    void orderExit(o3d::Double timestamp, Trade *trade, o3d::Double price);
};

} // namespace siis

#endif // SIIS_MAADX_H
