/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
  * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagemodifyposition.h"

using namespace siis;

ConnectorMessageModifyPosition::ConnectorMessageModifyPosition(o3d::UInt32 positionId,
                                                               o3d::Double stopLossPrice,
                                                               o3d::Double takeProfitPrice) :
    ConnectorMessageCore(FUNC_ID::MODIFYPOSITION, FUNC_ID::MODIFYPOSITION),
    m_positionId(positionId),
    m_stopLossPrice(stopLossPrice),
    m_takeProfitPrice(takeProfitPrice)
{

}

inline void ConnectorMessageModifyPosition::initSize()
{
	ConnectorMessageCore::initSize();

    m_size += static_cast<o3d::Int32>(sizeof(o3d::Int32)); // size of positionId
    m_size += static_cast<o3d::Int32>(sizeof(o3d::Double)) * 2;
}

inline void ConnectorMessageModifyPosition::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
}

void ConnectorMessageModifyPosition::write()
{
	ConnectorMessageCore::write();
	
    writeInt32(m_positionId);
    writeDouble(m_stopLossPrice);
    writeDouble(m_takeProfitPrice);
}

void ConnectorMessageModifyPosition::read(zmq::message_t *message)
{
    ConnectorMessageCore::read(message);

	// read the id
	m_positionId = readInt32();
}
