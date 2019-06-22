/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-23
 */

#ifndef SIIS_CONNECTOR_MESSAGE_CANCELORDER_H
#define SIIS_CONNECTOR_MESSAGE_CANCELORDER_H

#include "connectormessagecore.h"

namespace siis {


class SIIS_API_TEMPLATE ConnectorMessageCancelOrder : public ConnectorMessageCore
{
public:

	ConnectorMessageCancelOrder(o3d::UInt32 orderId);

    void write();
    void read(zmq::message_t *);

protected:

	void initSize();
	void initSizeReturn();

protected:

	o3d::UInt32 m_orderId;
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_CANCELORDER_H
