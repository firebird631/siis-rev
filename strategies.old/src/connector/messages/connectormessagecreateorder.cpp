/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
  * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagecreateorder.h"

using namespace siis;

ConnectorMessageCreateOrder::ConnectorMessageCreateOrder(const o3d::String marketId, 
                                                         o3d::Int32 direction, 
                                                         Order::OrderType orderType, 
                                                         o3d::Double orderPrice, 
                                                         o3d::Double quantity, 
                                                         o3d::Double stopLossPrice,
                                                         o3d::Double takeProfitPrice,
                                                         o3d::Double leverage) :
    ConnectorMessageCore(FUNC_ID::CREATEORDER, FUNC_ID::CREATEORDER),
	m_marketId(marketId),
    m_direction(direction),
    m_orderType(orderType),
    m_quantity(quantity),
    m_stopLossPrice(stopLossPrice),
    m_takeProfitPrice(takeProfitPrice),
    m_leverage(leverage),
    m_orderId(0)
{

}

void ConnectorMessageCreateOrder::initSize()
{
	ConnectorMessageCore::initSize();

    m_size += static_cast<o3d::Int32>(sizeof(o3d::Int8));
    m_size += getSizeString(m_marketId); // size of market
    m_size += static_cast<o3d::Int32>(sizeof(o3d::Int32)); // direction
    m_size += static_cast<o3d::Int32>(sizeof(o3d::Int8)); // orderType
    m_size += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 5;
}

void ConnectorMessageCreateOrder::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32)); // size of orderId
}

void ConnectorMessageCreateOrder::write()
{
	ConnectorMessageCore::write();

    writeString(m_marketId);
    writeInt32(m_direction);
    writeInt8(static_cast<o3d::Int8>(m_orderType));
    writeDouble(m_quantity);
    writeDouble(m_stopLossPrice);
    writeDouble(m_takeProfitPrice);
    writeDouble(m_leverage);
    writeDouble(m_leverage);
}

void ConnectorMessageCreateOrder::read(zmq::message_t *message)
{
    ConnectorMessageCore::read(message);

	// read the orderId
	m_orderId = readInt32();
}
