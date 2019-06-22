/**
 * @brief SiiS strategy connector.
 * @copyright Copyright (C) 2019 SiiS
  * @author Romain Locci
 * @date 2019-03-16
 */

#include "siis/connector/messages/connectormessagecore.h"

using namespace siis;

ConnectorMessageCore::ConnectorMessageCore(FUNC_ID funcId, FUNC_ID funcIdReturn) :
    m_message(nullptr),
    m_messageReturn(nullptr),
    m_size(0),
	m_size_check(0),
	m_size_return(0),
	m_size_check_return(0),
	m_overflow(false),
    m_ptr_msg(nullptr),
	m_ptr_msgReturn(nullptr),
    m_funcId(funcId),
    m_funcIdReturn(funcIdReturn),
    m_badfuncId(false)
{
}

ConnectorMessageCore::~ConnectorMessageCore()
{
	o3d::deletePtr(m_message);
	o3d::deletePtr(m_messageReturn);
}

zmq::message_t& ConnectorMessageCore::message()
{
    return *m_message;
}

zmq::message_t *ConnectorMessageCore::messageReturn()
{
	// init the return message
    if (m_messageReturn == nullptr) {
		m_messageReturn = new zmq::message_t();
    }

    return m_messageReturn;
}

//////////////////////////////////////////////////////
/// SIZE MANAGE
//////////////////////////////////////////////////////

o3d::Int32 ConnectorMessageCore::getSizeString(const o3d::String &str) const
{
    o3d::Int32 size = 0;

#ifdef SIIS_NBCARSTRINGFORMESSAGE
    size += static_cast<o3d::Int32>(sizeof(o3d::Int32));  // size for indicate the string size
#endif
    size += str.length();  // !! see if necessary to put the 0 (then +1)

	return size;
}

void ConnectorMessageCore::initSize()
{
    m_size = static_cast<o3d::Int32>(sizeof(o3d::Int32));
	m_size_check = 0;
}

void ConnectorMessageCore::initSizeReturn()
{
    m_size_return = (static_cast<o3d::Int32>(sizeof(o3d::Int32)));
	m_size_check_return = 0;
}

//////////////////////////////////////////////////////
/// ! SIZE MANAGE
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
/// WRITE MANAGE
//////////////////////////////////////////////////////

void ConnectorMessageCore::writeInt8(o3d::Int8 value)
{
    if (m_ptr_msg != nullptr && (m_size_check + static_cast<o3d::Int32>(sizeof(o3d::Int8))) <= m_size) {

        *reinterpret_cast<o3d::Int8*>(m_ptr_msg) = value;
		m_ptr_msg = reinterpret_cast<o3d::Int8 *>(m_ptr_msg) + 1;

        m_size_check += static_cast<o3d::Int32>(sizeof(o3d::Int8));
    } else {
		m_overflow = true;
    }
}

void ConnectorMessageCore::writeInt32(o3d::Int32 value)
{
    if (m_ptr_msg != nullptr && (m_size_check + static_cast<o3d::Int32>(sizeof(o3d::Int32))) <= m_size) {

        *reinterpret_cast<o3d::Int32*>(m_ptr_msg) = value;
		m_ptr_msg = reinterpret_cast<o3d::Int32 *>(m_ptr_msg) + 1;

        m_size_check += static_cast<o3d::Int32>(sizeof(o3d::Int32));
    } else {
		m_overflow = true;
    }
}

void ConnectorMessageCore::writeDouble(o3d::Double value)
{
    if (m_ptr_msg != nullptr && (m_size_check + static_cast<o3d::Int32>(sizeof(o3d::Double))) <= m_size) {

        *reinterpret_cast<o3d::Double*>(m_ptr_msg) = value;
		m_ptr_msg = reinterpret_cast<o3d::Double *>(m_ptr_msg) + 1;

        m_size_check += static_cast<o3d::Int32>(sizeof(o3d::Double));
    } else {
		m_overflow = true;
    }
}

void ConnectorMessageCore::writeString(const o3d::String &str)
{
	o3d::CString str_2 = str.toAscii(); // to have string with char type

    if (m_ptr_msg != nullptr && (m_size_check + static_cast<o3d::Int32>(sizeof(o3d::Int32)) + str_2.length()) <= m_size) {

        *reinterpret_cast<o3d::Int32*>(m_ptr_msg) = str_2.length();
        m_ptr_msg = reinterpret_cast<o3d::Int32*>(m_ptr_msg) + 1;

        memcpy(m_ptr_msg, str_2.getData(), static_cast<size_t>(str_2.length()));

        m_ptr_msg = reinterpret_cast<o3d::Char*>(m_ptr_msg) + str_2.length();

        m_size_check += static_cast<o3d::Int32>(sizeof(o3d::Int32)) + str_2.length();
    } else {
		m_overflow = true;
    }

}

void ConnectorMessageCore::write()
{
	// init size to have the total size for init the buffer message
	initSize();

	// init the message
    if (m_message != nullptr) {
		o3d::deletePtr(m_message);
    }

    m_message = new zmq::message_t(static_cast<size_t>(m_size));
    m_ptr_msg = reinterpret_cast<void*>(m_message->data());

	// write the funcId
	writeInt8(m_funcId);
}

//////////////////////////////////////////////////////
/// ! WRITE MANAGE
//////////////////////////////////////////////////////


//////////////////////////////////////////////////////
/// READ MANAGE
//////////////////////////////////////////////////////

ConnectorMessageCore::FUNC_ID ConnectorMessageCore::readFuncId(void *ptr_msg)
{
	o3d::Int8 value = 0;

    if (ptr_msg != nullptr) {
        value = *reinterpret_cast<o3d::Int8*>(ptr_msg);
    }

	return static_cast<FUNC_ID>(value);
}

o3d::Int8 ConnectorMessageCore::readInt8()
{
	o3d::Int8 value = 0;
	
    if (m_ptr_msgReturn != nullptr && (m_size_check_return + static_cast<o3d::Int32>(sizeof(o3d::Int8))) <= m_size_return) {
        value = *reinterpret_cast<o3d::Int8*>(m_ptr_msgReturn);
		m_ptr_msgReturn = reinterpret_cast<o3d::Int8*>(m_ptr_msgReturn) + 1;

        m_size_check_return += static_cast<o3d::Int32>(sizeof(o3d::Int8));
    } else {
		m_overflow = true;
    }

	return value;
}

o3d::Int32 ConnectorMessageCore::readInt32()
{
	o3d::Int32 value = 0;

    if (m_ptr_msgReturn != nullptr && (m_size_check_return + static_cast<o3d::Int32>(sizeof(o3d::Int32))) <= m_size_return) {
        value = *reinterpret_cast<o3d::Int32*>(m_ptr_msgReturn);
		m_ptr_msgReturn = reinterpret_cast<o3d::Int32*>(m_ptr_msgReturn) + 1;

        m_size_check_return += static_cast<o3d::Int32>(sizeof(o3d::Int32));
    } else {
		m_overflow = true;
    }

	return value;
}

o3d::Double ConnectorMessageCore::readDouble()
{
	o3d::Double value = 0;

    if (m_ptr_msgReturn != nullptr && (m_size_check_return + static_cast<o3d::Int32>(sizeof(o3d::Double))) <= m_size_return) {
        value = *reinterpret_cast<o3d::Double*>(m_ptr_msgReturn);
		m_ptr_msgReturn = reinterpret_cast<o3d::Double*>(m_ptr_msgReturn) + 1;

        m_size_check_return += static_cast<o3d::Int32>(sizeof(o3d::Double));
    } else {
		m_overflow = true;
    }

	return value;
}

o3d::String ConnectorMessageCore::readString()
{	
//#ifdef SIIS_NBCARSTRINGFORMESSAGE

	o3d::String str;

    if (m_ptr_msgReturn != nullptr && (m_size_check_return + static_cast<o3d::Int32>(sizeof(o3d::Int32))) < m_size_return) {
        o3d::Int32 size = *reinterpret_cast<o3d::Int32*>(m_ptr_msgReturn);
		m_ptr_msgReturn = reinterpret_cast<o3d::Int32 *>(m_ptr_msgReturn) + 1;

        o3d::Char *str_c = new o3d::Char[static_cast<size_t>(size)+1];
        memcpy(str_c, m_ptr_msgReturn, static_cast<size_t>(size));
		str_c[size] = 0;

		m_ptr_msgReturn = reinterpret_cast<o3d::Char*>(m_ptr_msgReturn) + size + static_cast<o3d::Int32>(sizeof(o3d::Int8));
		m_size_return += size;
        m_size_check_return += static_cast<o3d::Int32>(sizeof(o3d::Int32)) + size + static_cast<o3d::Int32>(sizeof(o3d::Int8));

        str.fromUtf8(str_c, 0);
        o3d::deleteArray(str_c);

    } else {
		m_overflow = true;
    }

    return str;
}

void ConnectorMessageCore::read(zmq::message_t *message)
{
	m_ptr_msgReturn = message;

	// init size to have the total size for init the buffer message
	initSizeReturn();

    m_ptr_msgReturn = reinterpret_cast<void*>(m_messageReturn->data());

	// read the funcId
    o3d::Int8 funcIdReturn = readInt8();

    if (funcIdReturn != m_funcIdReturn) {
		m_badfuncId = true;
    } else {
		m_badfuncId = false;
    }
}

//////////////////////////////////////////////////////
/// ! READ MANAGE
//////////////////////////////////////////////////////
