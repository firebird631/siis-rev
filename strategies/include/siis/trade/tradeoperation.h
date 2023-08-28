/**
 * @brief SiiS strategy trade model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_TRADEOPERATION_H
#define SIIS_TRADEOPERATION_H

#include "../base.h"
#include "../utils/common.h"

#include <o3d/core/base.h>

namespace siis {

/**
 * @brief Strategy trade operation base class.
 * @author Frederic Scherma
 * @date 2019-08-19
 */
class SIIS_API TradeOperation
{
public:

    TradeOperation();
    virtual ~TradeOperation();

    void setId(o3d::Int32 id) { m_id = id; }
    o3d::Int32 id() const { return m_id; }

    // @todo step stop-loss operation

protected:

    o3d::Int32 m_id;
    o3d::Int32 m_type;
};

} // namespace siis

#endif // SIIS_TRADEOPERATION_H
