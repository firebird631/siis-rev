/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
  * @author Romain Locci
 * @date 2019-03-23
 */

#include "siis/connector/messages/connectormessagesignals.h"

#include "siis/connector/marketsignal.h"
#include "siis/connector/ordersignal.h"
#include "siis/connector/positionsignal.h"
#include "siis/connector/statussignal.h"
#include "siis/connector/accountsignal.h"

using namespace siis;

///////////////////////////////////////////////////////////////////
// AccountSignal
///////////////////////////////////////////////////////////////////

void ConnectorMessageAccountSignal::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

	// event
    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int8));
	// timestamp, freeMargin, reservedMargin, marginFactor
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 4;
}

void ConnectorMessageAccountSignal::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

//	o3d::deletePtr(m_signal);
	AccountSignal::Event event = static_cast<AccountSignal::Event>(readInt8());
	m_signal = new AccountSignal(event);
	m_signal->timestamp = readDouble();
	m_signal->freeMargin = readDouble();
	m_signal->reservedMargin = readDouble();
	m_signal->marginFactor = readDouble();
}

///////////////////////////////////////////////////////////////////
// AssetSignal
///////////////////////////////////////////////////////////////////

void ConnectorMessageAssetSignal::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

	// event
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int8));
	// timestamp, freeQuantity, lockedQuantity
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 3;
	// string size
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
}

void ConnectorMessageAssetSignal::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

//	o3d::deletePtr(m_signal);
	AssetSignal::Event event = static_cast<AssetSignal::Event>(readInt8());
	m_signal = new AssetSignal(event);
	m_signal->timestamp = readDouble();
	m_signal->symbol = readString();
	m_signal->freeQuantity = readDouble();
	m_signal->lockedQuantity = readDouble();
}

///////////////////////////////////////////////////////////////////
// MarketSignal
///////////////////////////////////////////////////////////////////

void ConnectorMessageMarketSignal::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

	// event, ...
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int8)) * 7;
	// timestamp, ...
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 16;
	// string
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
	// Part
	m_size_return += static_cast<o3d::Int32>(sizeof(MarketSignal::Part)) * 2;
	// Fee
	m_size_return += static_cast<o3d::Int32>(sizeof(MarketSignal::Fee)) * 2;
}

void ConnectorMessageMarketSignal::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

//	o3d::deletePtr(m_signal);
	MarketSignal::Event event = static_cast<MarketSignal::Event>(readInt8());
	m_signal = new MarketSignal(event);

	m_signal->timestamp = readDouble();
	m_signal->marketId = readString();
	m_signal->open = readInt8();

	m_signal->marketType = static_cast<MarketSignal::MarketType>(readInt8());
	m_signal->unit = static_cast<MarketSignal::MarketUnit>(readInt8());
	m_signal->mode = static_cast<MarketSignal::MarketMode>(readInt8());

	m_signal->hedging = readInt8();

	m_signal->orderCaps = static_cast<MarketSignal::OrderCapacity>(readInt8());

	m_signal->expiry = readDouble();

    readPart(m_signal->base);
    readPart(m_signal->quote);

	m_signal->contractSize = readDouble();
	m_signal->lotSize = readDouble();
	m_signal->baseExchangeRate = readDouble();

	m_signal->valuePerPip = readDouble();
	m_signal->onePipMeans = readDouble();

	m_signal->marginFactor = readDouble();

	m_signal->priceFilter[0] = readDouble();
	m_signal->priceFilter[1] = readDouble();
	m_signal->priceFilter[2] = readDouble();
	
	m_signal->notionalFilter[0] = readDouble();
	m_signal->notionalFilter[1] = readDouble();
	m_signal->notionalFilter[2] = readDouble();

	m_signal->bid = readDouble();
	m_signal->ofr = readDouble();

    readFee(m_signal->fees[0]);
    readFee(m_signal->fees[1]);
}

void ConnectorMessageMarketSignal::readPart(MarketSignal::Part &part)
{
	part.symbol = readString();
	part.precision = readInt32();
	part.vol24h = readDouble();
}

void ConnectorMessageMarketSignal::readFee(MarketSignal::Fee &fee)
{
	fee.rate = readDouble();
	fee.commission = readDouble();

	fee.limits[0] = readDouble();
	fee.limits[1] = readDouble();
}

///////////////////////////////////////////////////////////////////
// OrderSignal
///////////////////////////////////////////////////////////////////

void ConnectorMessageOrderSignal::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

	// event, ...
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int8)) * 6;
	// timestamp, ..
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 11;
	// string, ...
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32)) * 6;
}

void ConnectorMessageOrderSignal::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

//	o3d::deletePtr(m_signal);
	OrderSignal::Event event = static_cast<OrderSignal::Event>(readInt8());
	m_signal = new OrderSignal(event);

	m_signal->timestamp = readDouble();

	m_signal->orderId = readString();
	m_signal->orderRefId = readString();

	m_signal->marketId = readString();

    m_signal->orderType = static_cast<OrderSignal::OrderType>(readInt8());
	m_signal->direction = readInt32();

	m_signal->orderQuantity = readDouble();
	m_signal->orderPrice = readDouble();

	m_signal->stopLossPrice = readDouble();
	m_signal->limitPrice = readDouble();

    m_signal->timeInForce = static_cast<OrderSignal::TimeInForce>(readInt8());
    m_signal->priceType = static_cast<OrderSignal::PriceType>(readInt8());

	m_signal->postOnly = readInt8();
	m_signal->closeOnly = readInt8();

	m_signal->tradeId = readString();

	m_signal->execPrice = readDouble();
	m_signal->avgPrice = readDouble();

	m_signal->filled = readDouble();
	m_signal->cumulativeFilled = readDouble();
	m_signal->quoteTransacted = readDouble();

	m_signal->commissionAmount = readDouble();
	m_signal->commissionAsset = readString();
}

///////////////////////////////////////////////////////////////////
// PositionSignal
///////////////////////////////////////////////////////////////////

void ConnectorMessagePositionSignal::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

	// event
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int8));
	// timestamp, ...
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 10;
	// string, ...
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32)) * 4;
}

void ConnectorMessagePositionSignal::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

//	o3d::deletePtr(m_signal);
	PositionSignal::Event event = static_cast<PositionSignal::Event>(readInt8());
	m_signal = new PositionSignal(event);
	m_signal->timestamp = readDouble();

	m_signal->positionId = readString();
	m_signal->marketId = readString();

	m_signal->direction = readInt32();
	m_signal->quantity = readDouble();

	m_signal->avgPrice = readDouble();
	m_signal->execPrice = readDouble();

	m_signal->stopLossPrice = readDouble();
	m_signal->limitPrice = readDouble();

	m_signal->profitCurrency = readString();
	m_signal->profitLoss = readDouble();

	m_signal->filled = readDouble();
	m_signal->cumulativeFilled = readDouble();

	m_signal->liquidationPrice = readDouble();
	m_signal->commission = readDouble();
}

///////////////////////////////////////////////////////////////////
// StatusSignal
///////////////////////////////////////////////////////////////////

void ConnectorMessageStatusSignal::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

	// event
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int8));
	// timestamp
	m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double));
}

void ConnectorMessageStatusSignal::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

//	o3d::deletePtr(m_signal);
	StatusSignal::Event event = static_cast<StatusSignal::Event>(readInt8());
	m_signal = new StatusSignal(event);
	m_signal->timestamp = readDouble();
}
