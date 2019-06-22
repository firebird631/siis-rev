/**
 * @brief SiiS strategy connector ZeroMQ implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_ZMQCONNECTOR_H
#define SIIS_ZMQCONNECTOR_H

#include "connector.h"

#include <o3d/core/hashmap.h>
#include <o3d/core/thread.h>
#include <o3d/core/mutex.h>

#include <zmq.hpp>
#include <queue>

namespace siis {

class Handler;

class ZmqMonitor : public zmq::monitor_t
{
public:

    ZmqMonitor(class ZmqConnector *connector, o3d::Int32 id);

private:

   o3d::Int32 m_id;
   class ZmqConnector *m_connector;

   virtual void on_event_connected(const zmq_event_t &event, const char* addr) override;
   virtual void on_event_disconnected(const zmq_event_t &event, const char* addr) override;
};

/**
 * @brief SiiS strategy connector ZeroMQ implementation.
 * @author Frederic Scherma
 * @date 2019-03-17
 * Given the request port at connector, the connection return the port the publisher.
 */
class SIIS_API ZmqConnector : public Connector, public o3d::Runnable
{
public:

    enum Protocol {
        PROTOCOL_TCP = 0,
        PROTOCOL_ICP = 1
    };

    ZmqConnector(Handler *handler, const o3d::String &host, o3d::UInt32 port, Protocol protocol=PROTOCOL_TCP);

    virtual ~ZmqConnector() override;

    virtual void start() override;
    virtual void stop() override;

    virtual void connect() override;
    virtual void disconnect() override;

    virtual void subscribe(Subscription &sub) override;
    virtual void unsubscribe(Subscription &sub) override;
    virtual void unsubscribeAll() override;

    virtual o3d::Int32 run(void *) override;

    virtual void setTraderProxy(TraderProxy *traderProxy) override;

    virtual TraderProxy* traderProxy() override;
    virtual const TraderProxy* traderProxy() const override;

    virtual Handler* handler() override;
    virtual const Handler* handler() const override;

    //
    // order
    //

    virtual void fetchAnyOrders() override;
    virtual void fetchOrder(const o3d::String& marketId) override;

    virtual o3d::Int32 createOrder(Order *order) override;

    virtual o3d::Int32 cancelOrder(const o3d::String &orderId) override;

    //
    // position
    //

    virtual void fetchAnyPositions() override;
    virtual void fetchPositions(const o3d::String& marketId) override;

    virtual o3d::Int32 closePosition(const o3d::String &positionId) override;

    virtual o3d::Int32 modifyPosition(
            const o3d::String &positionId,
            o3d::Double stopLossPrice,
            o3d::Double takeProfitPrice) override;

    //
    // account
    //

    virtual void fetchAccount() override;

    //
    // asset
    //

    virtual void fetchAnyAssets() override;

    virtual void fetchAssets(const o3d::String& assetId) override;

    //
    // processing
    //

    void processSendQueue();
    void processRecvAndDispatch();

protected:

    Handler *m_handler;

    o3d::String m_host;
    o3d::UInt32 m_port;
    Protocol m_protocol;

    o3d::Bool m_running;
    o3d::Bool m_connected;

    o3d::Thread m_thread;

    TraderProxy *m_traderProxy;
    stdext::hash_map<o3d::Int32, Subscription*> m_subscriptions;

    zmq::context_t *m_context;
    zmq::socket_t *m_socket;

    o3d::FastMutex m_sendMutex;
    o3d::FastMutex m_recvMutex;

    std::queue<zmq::message_t*> m_sendQueue;
    std::queue<zmq::message_t*> m_recvQueue;

    ZmqMonitor *m_monitor;
};

} // namespace siis

#endif // SIIS_ZMQCONNECTOR_H
