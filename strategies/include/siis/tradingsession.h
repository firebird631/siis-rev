/**
 * @brief SiiS strategy market trading session.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-03
 */

#ifndef SIIS_TRADINGSESSION_H
#define SIIS_TRADINGSESSION_H

#include "base.h"

#include <vector>

namespace siis {

/**
 * @brief Trading session per market.
 * Evening session from 00h00m00s000ms to 23h59m59s999ms in UTC, tuple with float time offset and time duration
 * Times are related to the configured instrument timezone.
 */
class SIIS_API TradingSession
{
public:

    TradingSession();

    o3d::Int8 dayOfWeek;
    o3d::Double fromTime;
    o3d::Double toTime;
};

} // namespace siis

#endif // SIIS_TRADINGSESSION_H
