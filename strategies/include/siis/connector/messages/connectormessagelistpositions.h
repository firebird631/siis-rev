/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-23
 */

#ifndef SIIS_CONNECTOR_MESSAGE_LISTPOSITIONS_H
#define SIIS_CONNECTOR_MESSAGE_LISTPOSITIONS_H

#include <list>

#include "connectormessagecore.h"
#include "../../position.h"

namespace siis {


class SIIS_API_TEMPLATE ConnectorMessageListPositions : public ConnectorMessageCore
{
public:

	ConnectorMessageListPositions();

    void write();
    void read(zmq::message_t *);

	std::list<Position*> listPositions() { return m_lisPositions; }

protected:

	void initSize();
	void initSizeReturn();

protected:

	std::list<Position*> m_lisPositions;
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_LISTPOSITIONS_H
