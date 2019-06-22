/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagereceiveohlc.h"

using namespace siis;


inline void ConnectorMessageReceiveOhlc::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 8;
}

void ConnectorMessageReceiveOhlc::read(zmq::message_t *message)
{
    ConnectorMessageCore::read(message);

	// read the marketId
	m_marketId = readString();

	// read the ohlc
	m_ohlc = new Ohlc();
    m_ohlc->copy(reinterpret_cast<o3d::Double*>(m_ptr_msgReturn));
}
