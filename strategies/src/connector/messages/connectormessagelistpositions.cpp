/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagelistpositions.h"

using namespace siis;

ConnectorMessageListPositions::ConnectorMessageListPositions() :
    ConnectorMessageCore(FUNC_ID::LISTPOSITIONS, FUNC_ID::LISTPOSITIONS),
	m_lisPositions()
{

}

void ConnectorMessageListPositions::initSize()
{
	ConnectorMessageCore::initSize();
    // @TODO
}

void ConnectorMessageListPositions::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    // @TODO
}

void ConnectorMessageListPositions::write()
{
	ConnectorMessageCore::write();

    // TODO
}

void ConnectorMessageListPositions::read(zmq::message_t *message)
{
    ConnectorMessageCore::read(message);

    // @TODO
}
