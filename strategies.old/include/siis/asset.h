/**
 * @brief SiiS strategy asset model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_ASSET_H
#define SIIS_ASSET_H

#include "base.h"

#include <o3d/core/string.h>
#include <o3d/core/mutex.h>
#include <o3d/core/runnable.h>
#include <o3d/core/stringmap.h>

namespace siis {

/**
 * @brief Strategy asset model.
 * @author Frederic Scherma
 * @date 2019-03-07
 */
class SIIS_API Asset
{
public:

    Asset(const o3d::String &symbol);
    ~Asset();

    const o3d::String& symbol() const { return m_symbol; }

    o3d::Double freeQuantity() const { return m_freeQuantity; }
    o3d::Double lockedQuantity() const { return m_lockedQuantity; }

    void setQuantity(o3d::Double freeQuantity, o3d::Double lockedQuantity, o3d::Double timestamp);

private:

    o3d::String m_symbol;
    o3d::Double m_lastTimestamp;

    o3d::Double m_freeQuantity;
    o3d::Double m_lockedQuantity;
};

} // namespace siis

#endif // SIIS_ASSET_H
