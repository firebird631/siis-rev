/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessageunsubscribe.h"

using namespace siis;

ConnectorMessageUnsubscribe::ConnectorMessageUnsubscribe(Subscription &sub) :
    ConnectorMessageCore(FUNC_ID::UNSUBSCRIBE, FUNC_ID::UNSUBSCRIBE),
    m_sub(sub)
{

}

inline void ConnectorMessageUnsubscribe::initSize()
{
	ConnectorMessageCore::initSize();

    m_size += static_cast<o3d::Int32>(sizeof(o3d::Int32)); // size of id subscription
    m_size += getSizeString(m_sub.marketId); // size of market
}

inline void ConnectorMessageUnsubscribe::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
}

void ConnectorMessageUnsubscribe::write()
{
	ConnectorMessageCore::write();
	
    writeInt32(m_sub.id);
    writeString(m_sub.marketId);
}

void ConnectorMessageUnsubscribe::read(zmq::message_t *message)
{
    ConnectorMessageCore::read(message);

	// read the id
	m_sub.id = readInt32();
}
