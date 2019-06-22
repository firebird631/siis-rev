/**
 * @brief SiiS strategy machine learning strategy optimizer interface.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-28
 */

#ifndef SIIS_OPTIMIZER_H
#define SIIS_OPTIMIZER_H

#include "../base.h"

namespace siis {

/**
 * @brief SiiS strategy machine learning strategy optimizer interface.
 * @author Frederic Scherma
 * @date 2019-03-28
 */
class SIIS_API Optimizer
{
public:

    virtual ~Optimizer() = 0;

    // @todo
};

} // namespace siis

#endif // SIIS_OPTIMIZER_H
