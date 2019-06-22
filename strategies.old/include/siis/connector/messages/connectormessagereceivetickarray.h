/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-15
 */

#ifndef SIIS_CONNECTOR_MESSAGE_RECEIVETICKARRAY_H
#define SIIS_CONNECTOR_MESSAGE_RECEIVETICKARRAY_H

#include "connectormessagecore.h"
#include "../../tick.h"

namespace siis {

class SIIS_API ConnectorMessageReceiveTickArray : public ConnectorMessageCore
{
public:

	ConnectorMessageReceiveTickArray() :
        ConnectorMessageCore(FUNC_ID::RECEIVE_TICK_ARRAY, FUNC_ID::RECEIVE_TICK_ARRAY),
        m_tickArray()
	{
	}

	void write() {} // don't use
	void read(zmq::message_t *message); // read the message

    TickArray& tickArray() { return m_tickArray; }
	o3d::String marketId() { return m_marketId; }

protected:

	void initSize() {} // don't use
	void initSizeReturn();

protected:

	o3d::String m_marketId;
    TickArray m_tickArray;
};


} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_RECEIVETICKARRAY_H
