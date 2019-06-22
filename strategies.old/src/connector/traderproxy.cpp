/**
 * @brief SiiS strategy trader proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#include "siis/connector/traderproxy.h"

#include "siis/connector/stdtraderproxy.h"
#include "siis/connector/margintraderproxy.h"
#include "siis/connector/indmargintraderproxy.h"

using namespace siis;

TraderProxy* TraderProxy::build(TraderProxy::Type type, Connector *connector)
{
    switch (type) {
        case TYPE_STD:
            return new StdTraderProxy(connector);
    }
}

TraderProxy::~TraderProxy()
{

}
