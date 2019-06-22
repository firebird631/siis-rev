/*
 * @brief SiiS strategy machine learning supervisor base class.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-28
 */

#include "siis/learning/supervisor.h"

using namespace siis;

Supervisor::Supervisor(Handler *handler, const o3d::String &identifier) :
    Strategy(handler, identifier)
{

}

Supervisor::~Supervisor()
{

}
