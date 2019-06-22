/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-15
 */
#ifndef SIIS_CONNECTOR_MESSAGE_RECEIVETICKAGGREGED_H
#define SIIS_CONNECTOR_MESSAGE_RECEIVETICKAGGREGED_H

#include <vector>
#include "connectormessagecore.h"
#include "../../tick.h"

namespace siis {

class SIIS_API ConnectorMessageReceiveTickAggreged : public ConnectorMessageCore
{
public:

	ConnectorMessageReceiveTickAggreged() :
        ConnectorMessageCore(FUNC_ID::RECEIVE_TICK_AGGREGED, FUNC_ID::RECEIVE_TICK_AGGREGED),
        m_listMarketId(),
        m_tickArray()
	{
	}

	void write() {} // don't use
	void read(zmq::message_t *message); // read the message

    TickArray& tickArray() { return m_tickArray; }
    std::vector<o3d::String>& listMarketId() { return m_listMarketId; }

protected:

	void initSize() {} // don't use
	void initSizeReturn();

protected:

    std::vector<o3d::String> m_listMarketId;
    TickArray m_tickArray;
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_RECEIVETICKAGGREGED_H
