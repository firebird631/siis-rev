/**
 * @brief SiiS strategy connector ZeroMQ implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/connector/zmqconnector.h"
#include "siis/connector/traderproxy.h"

#include "siis/connector/messages/connectormessagesubscribe.h"
#include "siis/connector/messages/connectormessageunsubscribe.h"

#include "siis/connector/messages/connectormessagereceiveohlc.h"
#include "siis/connector/messages/connectormessagereceiveohlcarray.h"
#include "siis/connector/messages/connectormessagereceivetick.h"
#include "siis/connector/messages/connectormessagereceivetickaggreged.h"
#include "siis/connector/messages/connectormessagereceivetickarray.h"

#include "siis/connector/messages/connectormessagesignals.h"

#include "siis/connector/messages/connectormessagelistorders.h"
#include "siis/connector/messages/connectormessagelistpositions.h"
#include "siis/connector/messages/connectormessagecreateorder.h"
#include "siis/connector/messages/connectormessagecancelorder.h"
#include "siis/connector/messages/connectormessagecloseposition.h"
#include "siis/connector/messages/connectormessagemodifyposition.h"

#include "siis/handler.h"

#include <o3d/core/debug.h>

using namespace siis;
using o3d::Logger;
using o3d::Debug;


ZmqConnector::ZmqConnector(Handler *handler, const o3d::String &host, o3d::UInt32 port, Protocol protocol) :
    m_handler(handler),
    m_host(host),
    m_port(port),
    m_protocol(protocol),
    m_running(false),
    m_connected(false),
    m_thread(this),
    m_traderProxy(nullptr),
    m_subscriptions(),
    m_context(nullptr),
    m_socket(nullptr),
    m_monitor(nullptr)
{

}

ZmqConnector::~ZmqConnector()
{
    stop();
}

void ZmqConnector::start()
{
    if (!m_running) {
        m_running = true;

        m_context = new zmq::context_t(1);
        m_thread.start();
        m_thread.setName("siis::zmq");
    }
}

void ZmqConnector::stop()
{
    if (m_running) {
        m_running = false;
        m_thread.waitFinish();

        o3d::deletePtr(m_context);
    }
}

void ZmqConnector::connect()
{
    if (!m_connected) {
        o3d::String protocol("tcp");
        if (m_protocol == PROTOCOL_ICP) {
            protocol = "icp";
        }

        o3d::CString addr = o3d::String("{0}://{1}:{2}").arg(protocol).arg(m_host).arg(m_port).toUtf8();

        // zmq req connect (we then receive the pub socket after)
        m_socket = new zmq::socket_t(*m_context, ZMQ_DEALER);
        m_socket->setsockopt(ZMQ_IDENTITY, addr.getData(), static_cast<o3d::UInt32>(addr.length()));
        m_socket->connect(addr.getData());

        m_monitor = new ZmqMonitor(this, 0);
        m_monitor->monitor(*m_socket, addr.getData(), ZMQ_EVENT_ALL);

        m_connected = true;

        O3D_WARNING("connected");
    }
}

void ZmqConnector::disconnect()
{
    if (m_connected) {
        o3d::String protocol("tcp");
        if (m_protocol == PROTOCOL_ICP) {
            protocol = "icp";
        }

        if (m_socket) {
            m_socket->disconnect(o3d::String("{0}://{1}:{2}").arg(protocol).arg(m_host).arg(m_port).toUtf8().getData());
            o3d::deletePtr(m_socket);
            o3d::deletePtr(m_monitor);
        }

        m_connected = false;
    }
}

void ZmqConnector::subscribe(Subscription &sub)
{
    if (!m_connected) {
        // @todo update
		ConnectorMessageSubscribe request(sub);

		request.write();

        m_sendMutex.lock();
        m_sendQueue.push(&request.message());
        m_sendMutex.unlock();

        // @todo on recv sub result
        // m_subscriptions[sub.id] = new Subscription(sub);
    }
}

void ZmqConnector::unsubscribe(Subscription &sub)
{
    auto it = m_subscriptions.find(sub.id);
    if (it != m_subscriptions.end()) {
        if (m_connected) {
            // @todo update
			ConnectorMessageUnsubscribe request(sub);

			request.write();

            m_sendMutex.lock();
            m_sendQueue.push(&request.message());
            m_sendMutex.unlock();
        }
        m_subscriptions.erase(it);
    }
}

void ZmqConnector::unsubscribeAll()
{
    for (auto pair : m_subscriptions) {
        if (m_connected) {
            unsubscribe(*pair.second);
        }
        o3d::deletePtr(pair.second);
    }

    m_subscriptions.clear();
}

o3d::Int32 ZmqConnector::run(void *)
{
    //  Initialize poll set
    zmq::pollitem_t items[] = {
        { m_socket, 0, ZMQ_POLLIN, 0 }
    };

    while (m_running) {
        if (!m_connected) {
            // connect or reconnect
            try {
                connect();
            } catch(zmq::error_t &e) {
                O3D_WARNING(e.what());
                o3d::System::waitMs(1000);  // wait 1sec before retry
            }
        }

        zmq::message_t message;
        zmq::poll(&items[0], 2, -1);

        processSendQueue();

        // if receive a message
        if (items[0].revents & ZMQ_POLLIN) {
            m_socket->recv(&message, ZMQ_DONTWAIT);
            // processRecvAndDispatch();

			// read functionId to dispatch message
            ConnectorMessageCore::FUNC_ID funcId = ConnectorMessageCore::readFuncId(message.data());

			// may be filtering the message !!!!
            switch (funcId) {
				// RECEIVE_TICK
                case ConnectorMessageCore::FUNC_ID::RECEIVE_TICK: {
					ConnectorMessageReceivetick msg;
					msg.read(&message);
					m_handler->onTick(msg.marketId(), msg.tick());
                } break;
				// REVEIVE_OHLC
                case ConnectorMessageCore::FUNC_ID::RECEIVE_OHLC: {
                    ConnectorMessageReceiveOhlc msg;
					msg.read(&message);
                    Ohlc::Type ohlcType = Ohlc::TYPE_MID;  // @todo need type on the message or 3 messages
                    m_handler->onOhlc(msg.marketId(), ohlcType, msg.ohlc());
                } break;
				
                // RECEIVE_TICK_ARRAY
                case ConnectorMessageCore::FUNC_ID::RECEIVE_TICK_ARRAY: {
                    ConnectorMessageReceiveTickArray msg;
					msg.read(&message);
					TickArray &tickArray = msg.tickArray();
                    for (o3d::Int32 i = 0; i < tickArray.getSize(); ++i) {
						m_handler->onTick(msg.marketId(), tickArray[i]);
					}
                } break;

                // RECEIVE_OHLC_ARRAY
                case ConnectorMessageCore::FUNC_ID::RECEIVE_OHLC_ARRAY: {
                    ConnectorMessageReceiveOhlcArray msg;
					msg.read(&message);
					OhlcArray &ohlcArray = msg.ohlcArray();
                    for (o3d::Int32 i = 0; i < ohlcArray.getSize(); ++i) {
                        Ohlc::Type ohlcType = Ohlc::TYPE_MID;  // @todo need type on the message or 3 messages
                        m_handler->onOhlc(msg.marketId(), ohlcType, ohlcArray[i]);
					}
                } break;
				
                // RECEIVE_TICK_AGGREGED
                case ConnectorMessageCore::FUNC_ID::RECEIVE_TICK_AGGREGED: {
                    ConnectorMessageReceiveTickAggreged msg;
					msg.read(&message);
					std::vector<o3d::String> &listMarketId = msg.listMarketId();
					TickArray &tickArray = msg.tickArray();
                    for (o3d::Int32 i = 0; i < tickArray.getSize(); ++i) {
                        m_handler->onTick(listMarketId[static_cast<size_t>(i)], tickArray[i]);
					}
                } break;
				
				
                // ACCOUNT_SIGNAL
                case ConnectorMessageCore::FUNC_ID::ACCOUNT_SIGNAL: {
					ConnectorMessageAccountSignal msg;
					msg.read(&message);
					m_traderProxy->onAccountSignal(msg.signal());
                } break;
				
                // ASSET_SIGNAL
                case ConnectorMessageCore::FUNC_ID::ASSET_SIGNAL: {
					ConnectorMessageAssetSignal msg;
					msg.read(&message);
					m_traderProxy->onAssetSignal(msg.signal());
                } break;
				
                // MARKET_SIGNAL
                case ConnectorMessageCore::FUNC_ID::MARKET_SIGNAL: {
					ConnectorMessageMarketSignal msg;
					msg.read(&message);
					m_traderProxy->onMarketSignal(msg.signal());
                } break;
				
                // ORDER_SIGNAL
                case ConnectorMessageCore::FUNC_ID::ORDER_SIGNAL: {
					ConnectorMessageOrderSignal msg;
					msg.read(&message);
					m_traderProxy->onOrderSignal(msg.signal());
                } break;
				
                // POSITION_SIGNAL
                case ConnectorMessageCore::FUNC_ID::POSITION_SIGNAL: {
					ConnectorMessagePositionSignal msg;
					msg.read(&message);
					m_traderProxy->onPositionSignal(msg.signal());
                } break;
				
                // STATUSSIGNAL
                case ConnectorMessageCore::FUNC_ID::STATUS_SIGNAL: {
					ConnectorMessageStatusSignal msg;
					msg.read(&message);
					m_traderProxy->onStatusSignal(msg.signal());
                } break;

                default:
                    break;
			}

        }

        // @todo on disconnect m_connected = false, loss all subscriptions... have to reinitiate ??
    }

    if (m_connected) {
        unsubscribeAll();

        // process last remaining send queue
        processSendQueue();

        disconnect();
    }

    return 0;
}

void ZmqConnector::setTraderProxy(TraderProxy *traderProxy)
{
    m_traderProxy = traderProxy;
}

TraderProxy *ZmqConnector::traderProxy()
{
    return m_traderProxy;
}

const TraderProxy *ZmqConnector::traderProxy() const
{
    return m_traderProxy;
}

Handler *ZmqConnector::handler()
{
    return m_handler;
}

const Handler *ZmqConnector::handler() const
{
    return m_handler;
}

void ZmqConnector::processSendQueue()
{
    // process the n current message no more to avoid an infinite loop of requests
    m_sendMutex.lock();
    size_t numMsg = m_sendQueue.size();
    m_sendMutex.unlock();

    while(numMsg > 0) {
        m_sendMutex.lock();
        zmq::message_t *msg = m_sendQueue.front();
        m_sendQueue.pop();
        m_sendMutex.unlock();

        m_socket->send(*msg, ZMQ_DONTWAIT);
        --numMsg;
    }
}

void ZmqConnector::processRecvAndDispatch()
{
    zmq::message_t msg;
    m_socket->recv(&msg, ZMQ_DONTWAIT);

    // decode & dispatch
    // @todo
}

void ZmqConnector::fetchAnyOrders()
{
    std::list<Order*> list;

    if (m_connected && m_traderProxy) {
//        ConnectorMessageListOrders msg;

//        msg.write();
//        m_req->send(msg.message());

//        m_req->recv(msg.messageReturn());
//        list = msg.listOrders();
    }

    //    return list;
}

void ZmqConnector::fetchOrder(const o3d::String &marketId)
{
    if (m_connected && m_traderProxy) {

    }
}

void ZmqConnector::fetchAnyPositions()
{
//   std::list<Position*> list;

    if (m_connected && m_traderProxy) {
//        ConnectorMessageListPositions msg;

//        msg.write();
//        m_req->send(msg.message());

//        m_req->recv(msg.messageReturn());
//        list = msg.listPositions();
    }

    //    return list;
}

void ZmqConnector::fetchPositions(const o3d::String &marketId)
{
    if (m_connected && m_traderProxy) {

    }
}

o3d::Int32 ZmqConnector::createOrder(Order *order)
{
    if (m_connected && m_traderProxy) {
        return 0;
//        ConnectorMessageCreateOrder msg(marketId, direction, orderType, orderPrice, quantity, stopLossPrice, takeProfitPrice, leverage);

//        msg.write();
//        m_req->send(msg.message());

//        m_req->recv(msg.messageReturn());

//        return msg.orderId();
    }
    else {
        return -1;
    }
}

o3d::Int32 ZmqConnector::cancelOrder(const o3d::String &orderId)
{
    if (m_connected && m_traderProxy) {
        // @todo replace by message static serializer
        ConnectorMessageCancelOrder msg(orderId);

        msg.write();

        m_sendMutex.lock();
        m_sendQueue.push(&msg.message());
        m_sendMutex.unlock();

        return 0;
    } else {
        return -1;
    }
}

o3d::Int32 ZmqConnector::closePosition(const o3d::String &positionId)
{
    if (m_connected && m_traderProxy) {
        // @todo replace by message static serializer
        ConnectorMessageClosePosition msg(positionId);

        msg.write();

        m_sendMutex.lock();
        m_sendQueue.push(&msg.message());
        m_sendMutex.unlock();

        return 0;
    } else {
        return -1;
    }
}

void ZmqConnector::fetchAccount()
{
    if (m_connected && m_traderProxy) {

    }
}

void ZmqConnector::fetchAnyAssets()
{
    if (m_connected && m_traderProxy) {

    }
}

void ZmqConnector::fetchAssets(const o3d::String &assetId)
{
    if (m_connected && m_traderProxy) {

    }
}

o3d::Int32 ZmqConnector::modifyPosition(
        const o3d::String &positionId,
        o3d::Double stopLossPrice,
        o3d::Double takeProfitPrice)
{
    if (m_connected && m_traderProxy) {
        ConnectorMessageModifyPosition msg(positionId, stopLossPrice, takeProfitPrice);

        msg.write();
        m_socket->send(msg.message());
        // m_socket->recv(msg.messageReturn());

        return 0;
    } else {
        return -1;
    }
}

ZmqMonitor::ZmqMonitor(ZmqConnector *connector, o3d::Int32 id) :
    m_id(id)
{

}

void ZmqMonitor::on_event_connected(const zmq_event_t &event, const char *addr)
{

}

void ZmqMonitor::on_event_disconnected(const zmq_event_t &event, const char *addr)
{

}
