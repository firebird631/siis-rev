/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
  * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagesubscribe.h"

using namespace siis;

ConnectorMessageSubscribe::ConnectorMessageSubscribe(Subscription &sub) :
    ConnectorMessageCore(FUNC_ID::SUBSCRIBE, FUNC_ID::SUBSCRIBE),
	m_sub(sub)
{

}

void ConnectorMessageSubscribe::initSize()
{
	ConnectorMessageCore::initSize();

    m_size += static_cast<o3d::Int32>(sizeof(o3d::Int8)); // size of type subscription
    m_size += getSizeString(m_sub.marketId); // size of market
    m_size += static_cast<o3d::Int32>(sizeof(o3d::Double)); // size of timeframe

    if (m_sub.type == Subscription::Type::TICK_ORDER_BOOK) {
        m_size += static_cast<o3d::Int32>(sizeof(o3d::Int32)); // size of timeframe
    }
}

void ConnectorMessageSubscribe::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
}

void ConnectorMessageSubscribe::write()
{
	ConnectorMessageCore::write();
	
    writeInt8(static_cast<o3d::Int8>(m_sub.type));
    writeString(m_sub.marketId);
    writeDouble(m_sub.timeframe);

    if (m_sub.type == Subscription::Type::TICK_ORDER_BOOK) {
        writeInt32(m_sub.orderBookdepth);
    }
}

void ConnectorMessageSubscribe::read(zmq::message_t *message)
{
    ConnectorMessageCore::read(message);

	// read the id
	m_sub.id = readInt32();
}
