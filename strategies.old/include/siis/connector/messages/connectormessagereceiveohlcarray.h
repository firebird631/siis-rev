/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-15
 */
#ifndef SIIS_CONNECTOR_MESSAGE_RECEIVEOHLCARRAY_H
#define SIIS_CONNECTOR_MESSAGE_RECEIVEOHLCARRAY_H

#include "connectormessagecore.h"
#include "../../ohlc.h"

namespace siis {

class SIIS_API ConnectorMessageReceiveOhlcArray : public ConnectorMessageCore
{
public:

	ConnectorMessageReceiveOhlcArray() :
        ConnectorMessageCore(FUNC_ID::RECEIVE_OHLC_ARRAY, FUNC_ID::RECEIVE_OHLC_ARRAY),
        m_marketId(),
        m_ohlcArray()
	{
	}

	void write() {} // don't use
	void read(zmq::message_t *message); // read the message

    OhlcArray& ohlcArray() { return m_ohlcArray; }
	o3d::String marketId() { return m_marketId; }

protected:

	void initSize() {} // don't use
	void initSizeReturn();

protected:

	o3d::String m_marketId;
    OhlcArray m_ohlcArray;
};


} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_RECEIVEOHLCARRAY_H
