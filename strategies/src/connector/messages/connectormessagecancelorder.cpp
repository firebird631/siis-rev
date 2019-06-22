/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagecancelorder.h"

using namespace siis;

ConnectorMessageCancelOrder::ConnectorMessageCancelOrder(const o3d::String &orderId) :
    ConnectorMessageCore(FUNC_ID::CANCELORDER, FUNC_ID::CANCELORDER),
    m_orderId(orderId),
    m_result(0)
{

}

inline void ConnectorMessageCancelOrder::initSize()
{
	ConnectorMessageCore::initSize();

    m_size += getSizeString(m_orderId);
}

inline void ConnectorMessageCancelOrder::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
}

void ConnectorMessageCancelOrder::write()
{
	ConnectorMessageCore::write();
	
    writeString(m_orderId);
}

void ConnectorMessageCancelOrder::read(zmq::message_t *message)
{
    ConnectorMessageCore::read(message);

	// read the id
    m_result = readInt32();
}
