/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-23
 */

#ifndef SIIS_CONNECTOR_MESSAGE_SIGNALS_H
#define SIIS_CONNECTOR_MESSAGE_SIGNALS_H

#include "connectormessagecore.h"
#include "../accountsignal.h"
#include "../assetsignal.h"
#include "../marketsignal.h"
#include "../ordersignal.h"
#include "../positionsignal.h"
#include "../statussignal.h"

namespace siis {

template <class S, ConnectorMessageCore::FUNC_ID funcId>
class SIIS_API_TEMPLATE ConnectorMessageSignal : public ConnectorMessageCore
{
public:

	ConnectorMessageSignal() : ConnectorMessageCore(funcId, funcId) {}

    void write() {} // don't use
    void read(zmq::message_t *) {} // read the message

	S &signal() { return *m_signal; }

protected:

	void initSize() {} // don't use
	void initSizeReturn();

protected:

	S* m_signal;
};

// AccountSignal
class SIIS_API ConnectorMessageAccountSignal : public ConnectorMessageSignal<AccountSignal, ConnectorMessageCore::FUNC_ID::ACCOUNT_SIGNAL>
{
public:
    void read(zmq::message_t *message);
protected:
    void initSizeReturn();
};

// AssetSignal
class SIIS_API ConnectorMessageAssetSignal : public ConnectorMessageSignal<AssetSignal, ConnectorMessageCore::FUNC_ID::ASSET_SIGNAL>
{
public:
    void read(zmq::message_t *message);
protected:
    void initSizeReturn();
};

// MarketSignal
class SIIS_API ConnectorMessageMarketSignal : public ConnectorMessageSignal<MarketSignal, ConnectorMessageCore::FUNC_ID::MARKET_SIGNAL>
{
public:
    void read(zmq::message_t *message);
protected:
    void initSizeReturn();
private:
    void readPart(MarketSignal::Part &part);
    void readFee(MarketSignal::Fee &fee);
};

// OrderSignal
class SIIS_API ConnectorMessageOrderSignal : public ConnectorMessageSignal<OrderSignal, ConnectorMessageCore::FUNC_ID::ORDER_SIGNAL>
{
public:
    void read(zmq::message_t *message);
protected:
    void initSizeReturn();
};

// PositionSignal
class SIIS_API ConnectorMessagePositionSignal : public ConnectorMessageSignal<PositionSignal, ConnectorMessageCore::FUNC_ID::POSITION_SIGNAL>
{
public:
    void read(zmq::message_t *message);
protected:
    void initSizeReturn();
};

// StatusSignal
class SIIS_API ConnectorMessageStatusSignal : public ConnectorMessageSignal<StatusSignal, ConnectorMessageCore::FUNC_ID::STATUS_SIGNAL>
{
public:
    void read(zmq::message_t *message);
protected:
    void initSizeReturn();
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_ACCOUNT_H
