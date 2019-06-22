/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagereceivetickaggreged.h"

using namespace siis;


inline void ConnectorMessageReceiveTickAggreged::initSizeReturn()
{
	ConnectorMessageCore::initSizeReturn();

	// timestamp
    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Double));
	// size string + nb
    m_size_return += static_cast<o3d::Int32>(sizeof(o3d::Int32)) * 2;
}


void ConnectorMessageReceiveTickAggreged::read(zmq::message_t *message)
{
	ConnectorMessageCore::read(message);

	// read the timestamp
	o3d::Double timestamp = readDouble();

	// read the number of tick
    o3d::Int32 numTicks = readInt32();
    m_size_return += (static_cast<o3d::Int32>(sizeof(o3d::Double)) * 3 + static_cast<o3d::Int32>(sizeof(o3d::Int32))) * numTicks;

    m_listMarketId.reserve(static_cast<size_t>(numTicks));

	// read the tick array
	o3d::Double bid, ofr, v;
    m_tickArray.resize(numTicks);
    for (o3d::Int32 i = 0; i < numTicks; i++) {
        m_listMarketId[static_cast<size_t>(i)] = readString();

        bid = readDouble();
		ofr = readDouble();
		v = readDouble();

        m_tickArray[i].set(timestamp, bid, ofr, v);
	}
}
