/**
 * @brief SiiS strategy trade initial entry handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-22
 */

#ifndef SIIS_ENTRY_H
#define SIIS_ENTRY_H

#include "entryexit.h"

namespace siis {

class TradeSignal;

/**
 * @brief Strategy trade initial entry handler.
 * @author Frederic Scherma
 * @date 2023-08-22
 * Inherit from this model for a custom policy and overrides update and init.
 */
class SIIS_API Entry : public EntryExit
{
public:

    Entry();

    void init(const Market *market, ContextConfig &conf);

    void updateSignal(TradeSignal &signal, const Market *market) const;

    o3d::Bool checkMaxSpread(const Market *market) const;

private:

    o3d::Double m_maxSpread;
};

} // namespace siis

#endif // SIIS_ENTRY_H
