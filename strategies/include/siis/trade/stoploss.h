/**
 * @brief SiiS strategy trade initial stop-loss handler.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-22
 */

#ifndef SIIS_STOPLOSS_H
#define SIIS_STOPLOSS_H

#include "entryexit.h"

namespace siis {

class TradeSignal;

/**
 * @brief Strategy trade initial stop-loss handler.
 * @author Frederic Scherma
 * @date 2023-05-22
 * Inherit from this model for a custom policy and overrides update and init.
 */
class SIIS_API StopLoss : public EntryExit
{
public:

    StopLoss();

    void init(const Market *market, ContextConfig &conf);

    void updateSignal(TradeSignal &signal) const;
};

} // namespace siis

#endif // SIIS_STOPLOSS_H
