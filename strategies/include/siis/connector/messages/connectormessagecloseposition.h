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

    ConnectorMessageClosePosition(const o3d::String &positionId);

    void write();
    void read(zmq::message_t *);

protected:

	void initSize();
	void initSizeReturn();

protected:

    o3d::String m_positionId;
    o3d::Int32 m_result;
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_CLOSEPOSITION_H
