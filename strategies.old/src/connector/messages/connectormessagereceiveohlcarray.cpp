/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
  * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagereceiveohlcarray.h"

using namespace siis;


inline void ConnectorMessageReceiveOhlcArray::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

	// size string + nb
    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32)) * 2;
}

void ConnectorMessageReceiveOhlcArray::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

	// read the marketId
	m_marketId = readString();

	// read the number of ohlc
    o3d::Int32 numOhlc = readInt32();
    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 8 * numOhlc;

	// read the ohlc array
    m_ohlcArray.resize(numOhlc);
    for (o3d::Int32 i = 0; i < numOhlc; i++) {
        m_ohlcArray[i].copy(reinterpret_cast<o3d::Double*>(m_ptr_msgReturn));
        // or memcpy(m_ohlcArray.getContent(i), m_ptr_msgReturn, 8*sizeof(o3d::Double));

		m_ptr_msg = reinterpret_cast<o3d::Double*>(m_ptr_msg) + 8;
		m_size_check_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 8;
	}
}
