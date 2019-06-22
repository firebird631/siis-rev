/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-23
 */

#ifndef SIIS_CONNECTOR_MESSAGE_CLOSEPOSITION_H
#define SIIS_CONNECTOR_MESSAGE_CLOSEPOSITION_H

#include "connectormessagecore.h"

namespace siis {


class SIIS_API_TEMPLATE ConnectorMessageClosePosition : public ConnectorMessageCore
{
public:

	ConnectorMessageClosePosition(o3d::UInt32 positionId);

    void write();
    void read(zmq::message_t *);

protected:

	void initSize();
	void initSizeReturn();

protected:

	o3d::UInt32 m_positionId;
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_CLOSEPOSITION_H
