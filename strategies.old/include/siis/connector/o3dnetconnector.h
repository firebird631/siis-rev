/**
 * @brief SiiS strategy connector o3dnet implementation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_O3DNETCONNECTOR_H
#define SIIS_O3DNETCONNECTOR_H

#include "connector.h"

#include <o3d/core/hashmap.h>
#include <o3d/core/thread.h>
#include <o3d/core/idmanager.h>

#include <zmq.hpp>

namespace siis {

/**
 * @brief SiiS strategy connector o3dnet implementation.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
class SIIS_API O3DNetConnector : public Connector, public o3d::Runnable
{
public:

    O3DNetConnector(const o3d::String &host, o3d::UInt32 port);

    virtual ~O3DNetConnector() override;

    virtual void start() override;
    virtual void stop() override;

    virtual void connect() override;
    virtual void disconnect() override;

    virtual void subscribe(const Subscription &sub) override;
    virtual void unsubscribe(const Subscription &sub) override;
    virtual void unsubscribeAll() override;

    virtual o3d::Int32 run(void *) override;

    virtual TraderProxy* traderProxy() override;
    virtual const TraderProxy* traderProxy() const override;

protected:

    o3d::String m_host;
    o3d::UInt32 m_port;

    o3d::UInt32 m_pubPort;

    o3d::Bool m_running;
    o3d::Bool m_connected;

    o3d::Thread m_thread;

    TraderProxy *m_traderProxy;
    stdext::hash_map<o3d::Int32, Subscription*> m_subscriptions;

    o3d::IDManager m_subIds;

protected:  // request

    void reqSubscribe(Subscription *sub);
    void reqUnsubscribe(o3d::Int32 subId);

protected:  // request response

    void onReqSubscribe();

protected:  // messages

};

} // namespace siis

#endif // SIIS_O3DNETCONNECTOR_H
