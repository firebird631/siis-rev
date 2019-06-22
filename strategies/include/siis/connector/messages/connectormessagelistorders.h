/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-23
 */

#ifndef SIIS_CONNECTOR_MESSAGE_LISTORDERS_H
#define SIIS_CONNECTOR_MESSAGE_LISTORDERS_H

#include <list>

#include "connectormessagecore.h"
#include "../../order.h"

namespace siis {


class SIIS_API ConnectorMessageListOrders : public ConnectorMessageCore
{
public:

	ConnectorMessageListOrders();

    void write();
    void read(zmq::message_t *);

	std::list<Order*> listOrders() { return m_listOrders; }

protected:

	void initSize();
	void initSizeReturn();

protected:

	std::list<Order*> m_listOrders;
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_LISTORDERS_H
