/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-15
 */

#ifndef SIIS_CONNECTOR_MESSAGE_UNSUBSCRIBE_H
#define SIIS_CONNECTOR_MESSAGE_UNSUBSCRIBE_H

#include "connectormessagecore.h"

namespace siis {

class SIIS_API ConnectorMessageUnsubscribe : public ConnectorMessageCore
{
public:

    ConnectorMessageUnsubscribe(Subscription &sub);

	void write(); // write the message
    void read(zmq::message_t *message); // read the message

protected:

    void initSize();
    void initSizeReturn();

protected:

    Subscription m_sub;
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_UNSUBSCRIBE_H
