/**
 * @brief SiiS connector client.
 * @copyright Copyright (C) 2019 SiiS
 * @author Romain Locci
 * @date 2019-03-23
 */

#ifndef SIIS_CONNECTOR_MESSAGE_CREATEORDER_H
#define SIIS_CONNECTOR_MESSAGE_CREATEORDER_H

#include "connectormessagecore.h"
#include "../../order.h"

namespace siis {


class SIIS_API_TEMPLATE ConnectorMessageCreateOrder : public ConnectorMessageCore
{
public:

	ConnectorMessageCreateOrder(const o3d::String marketId, 
                                      o3d::Int32 direction, 
                                      Order::OrderType orderType, 
                                      o3d::Double orderPrice, 
                                      o3d::Double quantity, 
                                      o3d::Double stopLossPrice,
                                      o3d::Double takeProfitPrice,
                                      o3d::Double leverage = 1.0);

    void write();
    void read(zmq::message_t *);

    o3d::Int32 orderId() { return m_orderId; }

protected:

	void initSize();
	void initSizeReturn();

protected:

	o3d::Int32 m_orderId;

	o3d::String m_marketId;
    o3d::Int32 m_direction;
    Order::OrderType m_orderType;
    o3d::Double m_orderPrice; 
    o3d::Double m_quantity;
    o3d::Double m_stopLossPrice;
    o3d::Double m_takeProfitPrice;
    o3d::Double m_leverage;
};

} // namespace siis

#endif // SIIS_CONNECTOR_MESSAGE_CREATEORDER_H
