/**
 * @brief SiiS strategy machine learning supervisor base class.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-28
 */

#ifndef SIIS_SUPERVISOR_H
#define SIIS_SUPERVISOR_H

#include "../strategy.h"

namespace siis {

/**
 * @brief SiiS strategy machine learning supervisor interface.
 * @author Frederic Scherma
 * @date 2019-03-28
 * Special case of a strategy used to manage the learning of a streamed data set.
 */
class SIIS_API Supervisor : public Strategy
{
public:

    Supervisor(Handler *handler, const o3d::String &identifier);

    virtual ~Supervisor() = 0;
};

} // namespace siis

#endif // SIIS_SUPERVISOR_H
