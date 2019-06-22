/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
  * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagereceivetickarray.h"

using namespace siis;


inline void ConnectorMessageReceiveTickArray::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

	// size string + nb
    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32)) * 2;
}


void ConnectorMessageReceiveTickArray::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

	// read the marketId
	m_marketId = readString();

	// read the number of tick
    o3d::Int32 numTicks = readInt32();
    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 4 * numTicks;

	// read the tick array
    m_tickArray.resize(numTicks);
    for (o3d::Int32 i = 0; i < numTicks; i++) {
		m_tickArray[i].copy(reinterpret_cast<o3d::Double*>(m_ptr_msgReturn));
        // memcpy(m_tickArray.getContent(i), m_ptr_msgReturn, 4*sizeof(o3d::Double));

		m_ptr_msg = reinterpret_cast<o3d::Double*>(m_ptr_msg) + 4;
		m_size_check_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 4;
	}
}
