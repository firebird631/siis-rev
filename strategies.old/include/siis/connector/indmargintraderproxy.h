/**
 * @brief SiiS strategy indivisible position, margin account trader proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_INDMARGINTRADERPROXY_H
#define SIIS_INDMARGINTRADERPROXY_H

#include "traderproxy.h"

namespace siis {

/**
 * @brief Strategy indivisible position, margin trader proxy.
 * @author Frederic Scherma
 * @date 2019-03-24
 */
class SIIS_API IndMarginTraderProxy : public TraderProxy
{
public:

    IndMarginTraderProxy(Connector *connector);

    virtual ~IndMarginTraderProxy() override;

    //
    // signals
    //

    virtual void onAccountSignal(const AccountSignal &signal) override;
    virtual void onAssetSignal(const AssetSignal &signal) override;
    virtual void onMarketSignal(const MarketSignal &signal) override;
    virtual void onOrderSignal(const OrderSignal &signal) override;
    virtual void onPositionSignal(const PositionSignal &signal) override;
    virtual void onStatusSignal(const StatusSignal &signal) override;

private:

    Connector *m_connector;
};

} // namespace siis

#endif // SIIS_INDMARGINTRADERPROXY_H
