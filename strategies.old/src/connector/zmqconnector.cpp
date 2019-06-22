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
    m_pubPort(port+1),
    m_running(false),
    m_connected(false),
    m_thread(this),
    m_traderProxy(nullptr),
    m_subscriptions(),
    m_context(nullptr),
    m_req(nullptr),
    m_sub(nullptr)
{

}

ZmqConnector::~ZmqConnector()
{
    stop();
}

void ZmqConnector::start()
{
    if (!m_running) {
        m_context = new zmq::context_t(1);
        m_traderProxy = TraderProxy::build(TraderProxy::TYPE_STD, this);

        m_thread.start();
    }
}

void ZmqConnector::stop()
{
    if (m_running) {
        m_running = false;
        m_thread.waitFinish();

        o3d::deletePtr(m_context);
        o3d::deletePtr(m_traderProxy);
    }
}

void ZmqConnector::connect()
{
    if (!m_connected) {
        o3d::String protocol("tcp");
        if (m_protocol == PROTOCOL_ICP) {
            protocol = "icp";
        }

        // zmq req connect (we then receive the pub socket after)
        m_req = new zmq::socket_t(*m_context, ZMQ_REQ);
        m_req->connect(o3d::String("{0}://{1}:{2}").arg(protocol).arg(m_host).arg(m_port).toUtf8().getData());

        // but for now we just assume the next port is the pub socket
        m_sub = new zmq::socket_t(*m_context, ZMQ_SUB);
        m_sub->connect(o3d::String("{0}://{1}:{2}").arg(protocol).arg(m_host).arg(m_pubPort).toUtf8().getData());
        m_sub->setsockopt(ZMQ_SUBSCRIBE, "", 0);

        m_connected = true;
    }
}

void ZmqConnector::disconnect()
{
    if (m_connected) {
        o3d::String protocol("tcp");
        if (m_protocol == PROTOCOL_ICP) {
            protocol = "icp";
        }

        if (m_req) {
            m_req->disconnect(o3d::String("{0}://{1}:{2}").arg(protocol).arg(m_host).arg(m_port).toUtf8().getData());
            o3d::deletePtr(m_req);
        }

        if (m_sub) {
            m_sub->disconnect(o3d::String("{0}://{1}:{2}").arg(protocol).arg(m_host).arg(m_pubPort).toUtf8().getData());
            o3d::deletePtr(m_sub);
        }

        m_connected = false;
    }
}

void ZmqConnector::subscribe(Subscription &sub)
{
    if (!m_connected) {
		ConnectorMessageSubscribe request(sub);

		request.write();
        m_req->send(request.message());

		// receive the return message
        m_req->recv(request.messageReturn());
		m_subscriptions[sub.id] = new Subscription(sub);
    }
}

void ZmqConnector::unsubscribe(Subscription &sub)
{
    auto it = m_subscriptions.find(sub.id);
    if (it != m_subscriptions.end()) {
        if (m_connected) {
			ConnectorMessageUnsubscribe request(sub);

			request.write();
            m_req->send(request.message());

			// receive the return message
            m_req->recv(request.messageReturn());
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
        { m_req, 0, ZMQ_POLLIN, 0 },
        { m_sub, 0, ZMQ_POLLIN, 0 }
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

        // if receive a message of req server
        if (items[0].revents & ZMQ_POLLIN) {
            m_req->recv(&message, ZMQ_DONTWAIT);
            // @todo decode & dispatch

        }

        // if receive a message to the publisher
        if (items[1].revents & ZMQ_POLLIN) {
            m_sub->recv(&message, ZMQ_DONTWAIT);
            
			// read functionId to dispatch message
            ConnectorMessageCore::FUNC_ID funcId = ConnectorMessageCore::readFuncId(message.data());

			// may be filtering the message !!!!
			switch (funcId)
			{
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
        disconnect();
    }

    return 0;
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


std::list<Order*> ZmqConnector::listOrders()
{
    std::list<Order*> list;

    if (!m_connected) {
        ConnectorMessageListOrders msg;

        msg.write();
        m_req->send(msg.message());

        m_req->recv(msg.messageReturn());
        list = msg.listOrders();
    }

    return list;
}

std::list<Position*> ZmqConnector::listPositions()
{
   std::list<Position*> list;

    if (!m_connected) {
        ConnectorMessageListPositions msg;

        msg.write();
        m_req->send(msg.message());

        m_req->recv(msg.messageReturn());
        list = msg.listPositions();
    }

    return list;
}

o3d::UInt32 ZmqConnector::createOrder(const o3d::String marketId, 
                                      o3d::Int32 direction, 
                                      Order::OrderType orderType, 
                                      o3d::Double orderPrice, 
                                      o3d::Double quantity, 
                                      o3d::Double stopLossPrice,
                                      o3d::Double takeProfitPrice,
                                      o3d::Double leverage) 
{
    if (!m_connected) {
        ConnectorMessageCreateOrder msg(marketId, direction, orderType, orderPrice, quantity, stopLossPrice, takeProfitPrice, leverage);

        msg.write();
        m_req->send(msg.message());

        m_req->recv(msg.messageReturn());

        return msg.orderId();
    }
    else {
        return 0;
    }
}

void ZmqConnector::cancelOrder(o3d::UInt32 orderId)
{
    if (!m_connected) {
        ConnectorMessageCancelOrder msg(orderId);

        msg.write();
        m_req->send(msg.message());

        m_req->recv(msg.messageReturn());
    }
}

void ZmqConnector::closePosition(o3d::UInt32 positionId)
{
    if (!m_connected) {
        ConnectorMessageClosePosition msg(positionId);

        msg.write();
        m_req->send(msg.message());

        m_req->recv(msg.messageReturn());
    }
}

void ZmqConnector::modifyPosition(o3d::UInt32 positionId,
                                  o3d::Double stopLossPrice,
                                  o3d::Double takeProfitPrice)
{
    if (!m_connected) {
        ConnectorMessageModifyPosition msg(positionId, stopLossPrice, takeProfitPrice);

        msg.write();
        m_req->send(msg.message());

        m_req->recv(msg.messageReturn());
    }
}
