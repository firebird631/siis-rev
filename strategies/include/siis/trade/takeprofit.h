/**
 * @brief SiiS strategy trade initial take-profit handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#ifndef SIIS_TAKEPROFIT_H
#define SIIS_TAKEPROFIT_H

#include "entryexit.h"

namespace siis {

class TradeSignal;

/**
 * @brief Strategy trade initial take-profit handler.
 * @author Frederic Scherma
 * @date 2023-09-30
 * Inherit from this model for a custom policy and overrides update and init.
 */
class SIIS_API TakeProfit : public EntryExit
{
public:

    TakeProfit();

    void init(const Market *market, ContextConfig &conf);

    void updateSignal(TradeSignal &signal) const;
};

} // namespace siis

#endif // SIIS_TAKEPROFIT_H
