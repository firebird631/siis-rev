/**
 * @brief SiiS strategy asset model.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#include "siis/asset.h"

using namespace siis;

Asset::Asset(const o3d::String &symbol) :
    m_symbol(symbol),
    m_lastTimestamp(0.0),
    m_freeQuantity(0.0),
    m_lockedQuantity(0.0)
{

}

Asset::~Asset()
{

}

void Asset::setQuantity(o3d::Double freeQty, o3d::Double lockedQty, o3d::Double timestamp)
{
    m_freeQuantity = freeQty;
    m_lockedQuantity = lockedQty;
    m_lastTimestamp = timestamp;
}
