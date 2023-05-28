/**
 * @brief SiiS strategy trade breakeven handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-04
 */

#ifndef SIIS_BREAKEVEN_H
#define SIIS_BREAKEVEN_H

#include "entryexit.h"

namespace siis {

/**
 * @brief Strategy trade breakeven handler.
 * @author Frederic Scherma
 * @date 2023-05-04
 * Inherit from this model for a custom policy and overrides update and init.
 */
class SIIS_API Breakeven : public EntryExit
{
public:

    Breakeven();

    void init(const Market *market, ContextConfig &conf);

    void updateΤrade(o3d::Double timestamp, o3d::Double lastTimestamp, Trade *trade) const;

    /**
     * @brief breakevenTiggerPrice According to the method, return the trigger price to set trade at breakeven.
     */
    o3d::Double breakevenTiggerPrice(o3d::Double price, o3d::Int32 direction) const;
};

} // namespace siis

#endif // SIIS_BREAKEVEN_H
