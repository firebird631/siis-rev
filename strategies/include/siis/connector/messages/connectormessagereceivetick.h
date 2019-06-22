/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-15
 */
#ifndef SIIS_CONNECTOR_MESSAGE_RECEIVETICK_H
#define SIIS_CONNECTOR_MESSAGE_RECEIVETICK_H

#include "connectormessagecore.h"
#include "../../tick.h"

namespace siis {

class SIIS_API ConnectorMessageReceivetick : public ConnectorMessageCore
{
public:

	ConnectorMessageReceivetick() :
        ConnectorMessageCore(FUNC_ID::RECEIVE_TICK, FUNC_ID::RECEIVE_TICK),
        m_marketId(),
        m_tick()
	{
	}

	void write() {} // don't use
	void read(zmq::message_t *message); // read the message

    Tick& tick() { return m_tick; }
	o3d::String marketId() { return m_marketId; }

protected:

	void initSize() {} // don't use
	void initSizeReturn();

protected:

	o3d::String m_marketId;
    Tick m_tick;
};


} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_RECEIVETICK_H
