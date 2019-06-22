/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
  * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagecloseposition.h"

using namespace siis;

ConnectorMessageClosePosition::ConnectorMessageClosePosition(o3d::UInt32 positionId) :
    ConnectorMessageCore(FUNC_ID::CLOSEPOSITION, FUNC_ID::CLOSEPOSITION),
    m_positionId(positionId)
{

}

inline void ConnectorMessageClosePosition::initSize()
{
	ConnectorMessageCore::initSize();

    m_size += static_cast<o3d::Int32>(sizeof(o3d::Int32)); // size of positionId
}

inline void ConnectorMessageClosePosition::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
}

void ConnectorMessageClosePosition::write()
{
	ConnectorMessageCore::write();
	
    writeInt32(m_positionId);
}

void ConnectorMessageClosePosition::read(zmq::message_t *message)
{
    ConnectorMessageCore::read(message);

	// read the id
	m_positionId = readInt32();
}
