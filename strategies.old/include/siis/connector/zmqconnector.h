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

#include <zmq.hpp>

namespace siis {

class Handler;

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

    virtual TraderProxy* traderProxy() override;
    virtual const TraderProxy* traderProxy() const override;

    virtual Handler* handler() override;
    virtual const Handler* handler() const override;

    //
    //
    // orders functions
    //

    virtual std::list<Order*> listOrders() override;
    virtual std::list<Position*> listPositions() override;
    virtual o3d::UInt32 createOrder(const o3d::String marketId, 
                                    o3d::Int32 direction, 
                                    Order::OrderType orderType, 
                                    o3d::Double orderPrice, 
                                    o3d::Double quantity, 
                                    o3d::Double stopLossPrice,
                                    o3d::Double takeProfitPrice,
                                    o3d::Double leverage) override;
    virtual void cancelOrder(o3d::UInt32 orderId) override;
    virtual void closePosition(o3d::UInt32 positionId) override;
    virtual void modifyPosition(o3d::UInt32 positionId,
                                o3d::Double stopLossPrice,
                                o3d::Double takeProfitPrice) override;

protected:

    Handler *m_handler;

    o3d::String m_host;
    o3d::UInt32 m_port;
    Protocol m_protocol;

    o3d::UInt32 m_pubPort;

    o3d::Bool m_running;
    o3d::Bool m_connected;

    o3d::Thread m_thread;

    TraderProxy *m_traderProxy;
    stdext::hash_map<o3d::Int32, Subscription*> m_subscriptions;

    zmq::context_t *m_context;
    zmq::socket_t *m_req;
    zmq::socket_t *m_sub;
};

} // namespace siis

#endif // SIIS_ZMQCONNECTOR_H
