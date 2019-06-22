/**
 * @brief SiiS strategy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_STDSUPERVISOR_H
#define SIIS_STDSUPERVISOR_H

#include "supervisor.h"

#include "siis/tick.h"
#include "siis/ohlc.h"

#include "siis/trade/stdtrademanager.h"

namespace siis {

/**
 * @brief Standard implementation of a learning supervisor.
 * @author Frederic Scherma
 * @date 2019-03-05
 * @todo what do we need exactly, first extractor/aggregator of data, next learning itself from differents
 * cases of inputs.
 */
class SIIS_API StdSupervisor : public Supervisor
{
public:

    StdSupervisor(Handler *handler, const o3d::String &identifier);
    virtual ~StdSupervisor();

    virtual void init(Config *config);
    virtual void terminate(Connector *connector, Database *db);

    virtual void prepareMarketData(Connector *connector, Database *db);
    virtual void finalizeMarketData(Connector *connector, Database *db);

    virtual void onTickUpdate(o3d::Double timestamp, const TickArray &ticks);
    virtual void onOhlcUpdate(o3d::Double timestamp,
                              o3d::Double timeframe,
                              Ohlc::Type ohlcType,
                              const OhlcArray &ohlc);
    // virtual void onOrderBookUpdate(o3d::Double timestamp, o3d::Double timeframe, const OrderBook &orderBook);

    virtual void prepare(o3d::Double timestamp);
    virtual void compute(o3d::Double timestamp);
    virtual void finalize(o3d::Double timestamp);

private:

    o3d::Bool m_needUpdate;
    o3d::Double m_minTimeframe;

    o3d::Int32 m_numEpoch;
    o3d::Int32 m_batchSize;

    StdTradeManager *m_tradeManager;
};

} // namespace siis

#endif // SIIS_STDSUPERVISOR_H
