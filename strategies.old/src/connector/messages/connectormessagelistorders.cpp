/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
  * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagelistorders.h"

using namespace siis;

ConnectorMessageListOrders::ConnectorMessageListOrders() :
    ConnectorMessageCore(FUNC_ID::LISTORDERS, FUNC_ID::LISTORDERS),
	m_listOrders()
{

}

void ConnectorMessageListOrders::initSize()
{
	ConnectorMessageCore::initSize();
    // @TODO
}

void ConnectorMessageListOrders::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

    // @TODO
}

void ConnectorMessageListOrders::write()
{
	ConnectorMessageCore::write();

    // TODO
}

void ConnectorMessageListOrders::read(zmq::message_t *message)
{
    ConnectorMessageCore::read(message);

    // @TODO
}
