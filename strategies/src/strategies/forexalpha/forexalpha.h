/**
 * @brief SiiS strategy forexalpha.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_FOREXALPHA_H
#define SIIS_FOREXALPHA_H

#include "siis/base.h"
#include "siis/strategy.h"

#include "siis/tick.h"
#include "siis/ohlc.h"

#include "siis/indicators/price/price.h"
#include "siis/indicators/volume/volume.h"

#include "siis/trade/stdtrademanager.h"

#include "faanalyser.h"

namespace siis {

/**
 * @brief Strategy forexalpha.
 * @author Frederic Scherma
 * @date 2019-03-05
 */
class SIIS_PLUGIN_API ForexAlpha : public Strategy
{
public:

    ForexAlpha(Handler *handler, const o3d::String &identifier);
    virtual ~ForexAlpha() override;

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

    o3d::Double m_minVol24h;
    o3d::Double m_minPrice;

    o3d::Double m_minTradedTimeframe;
    o3d::Double m_maxTradedTimeframe;

    std::vector<FaAnalyser*> m_analysers;
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

#endif // SIIS_FOREXALPHA_H
