/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-15
 */

#ifndef SIIS_CONNECTOR_MESSAGE_CORE_H
#define SIIS_CONNECTOR_MESSAGE_CORE_H

#include "../subscription.h"

#include <zmq.hpp>

/**
 * @warning If it is necessary to indicate the number of characters for the strings of characters in the message
 * put the line in comment if not necessary
 */
#define SIIS_NBCARSTRINGFORMESSAGE

namespace siis {

/**
 * @brief Class core message for the connector
 */
class SIIS_API ConnectorMessageCore
{
public:

	enum FUNC_ID
	{
        SUBSCRIBE = 1,
        UNSUBSCRIBE = 2,
        RECEIVE_TICK = 3,
        RECEIVE_OHLC = 4,
        RECEIVE_TICK_ARRAY = 5,
        RECEIVE_TICK_AGGREGED = 6,
        RECEIVE_OHLC_ARRAY = 7,

        STATUS_SIGNAL = 20,
        ACCOUNT_SIGNAL = 21,
        ASSET_SIGNAL = 22,
        MARKET_SIGNAL = 23,
        POSITION_SIGNAL = 24,
        ORDER_SIGNAL = 25,

        LISTORDERS = 40,
        LISTPOSITIONS = 41,
        CREATEORDER = 42,
        CANCELORDER = 43,
        CLOSEPOSITION = 44,
        MODIFYPOSITION = 45
	};

    ConnectorMessageCore(FUNC_ID funcId, FUNC_ID funcIdReturn);

    ~ConnectorMessageCore();

    zmq::message_t &message();
    zmq::message_t* messageReturn();

    void write(); // write the message
    void read(zmq::message_t *message); // read the message

    static ConnectorMessageCore::FUNC_ID readFuncId(void *ptr_msg);

protected:

	// initialize special size
    o3d::Int32 getSizeString(const o3d::String &str) const;

    void initSize(); // initialize size of message
    void initSizeReturn(); // initialize size of return message

	// writer type
	void writeInt8(o3d::Int8 value);
	void writeInt32(o3d::Int32 value);
	void writeDouble(o3d::Double value);
	void writeString(const o3d::String &str);

	// read type
    o3d::Int8 readInt8();
    o3d::Int32 readInt32();
    o3d::Double readDouble();
    o3d::String readString();

protected:
	
	zmq::message_t *m_message; // message stream
	zmq::message_t *m_messageReturn; // return message stream

	o3d::Int32 m_size; // size of message stream
	o3d::Int32 m_size_check; // size check
	o3d::Int32 m_size_return; // size of return message stream
	o3d::Int32 m_size_check_return; // size check return
	o3d::Bool m_overflow; // if write overflow

	void *m_ptr_msg; // pointer to move in message
	void *m_ptr_msgReturn; // pointer to move in return message

	o3d::Int8 m_funcId; // the id to identify the message function
	o3d::Int8 m_funcIdReturn; // the id to identify the return message function
	o3d::Bool m_badfuncId; // if bad funcId returned
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_CORE_H
