/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-15
 */

#ifndef SIIS_CONNECTOR_MESSAGE_RECEIVEOHLC_H
#define SIIS_CONNECTOR_MESSAGE_RECEIVEOHLC_H

#include "connectormessagecore.h"
#include "../../ohlc.h"

namespace siis {

class SIIS_API ConnectorMessageReceiveOhlc : public ConnectorMessageCore
{
public:

	ConnectorMessageReceiveOhlc() :
        ConnectorMessageCore(FUNC_ID::RECEIVE_OHLC, FUNC_ID::RECEIVE_OHLC),
		m_ohlc(nullptr)
	{
	}

	void write() {} // don't use
	void read(zmq::message_t *message); // read the message

	Ohlc &ohlc() { return *m_ohlc;  }
	o3d::String marketId() { return m_marketId; }

protected:

	void initSize() {} // don't use
	void initSizeReturn();

protected:

	o3d::String m_marketId;
	Ohlc *m_ohlc;
};


} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_RECEIVEOHLC_H
