/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagereceivetick.h"

using namespace siis;


inline void ConnectorMessageReceivetick::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 4;
}

void ConnectorMessageReceivetick::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

	// read the marketId
	m_marketId = readString();
	// read the tick
	m_tick.copy(reinterpret_cast<o3d::Double*>(m_ptr_msgReturn));
}
