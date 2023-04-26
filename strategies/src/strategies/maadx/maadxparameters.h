/**
 * @brief SiiS strategy MAADX default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_MAADXPARAMETERS_H
#define SIIS_MAADXPARAMETERS_H

namespace siis {

/**
 * @brief Strategy MAADXdefault parameters.
 * @author Frederic Scherma
 * @date 2023-04-24
 */
static const char* MaAdxParameters = R"JSON(
{
    "reversal": true,
    "pyramided": 0,
    "hedging": false,
    "max-trades": 3,
    "trade-delay": 30,
    "need-update": false,
    "contexts": {
        "ma-adx-5m-1m": {
        }
    },
    "timeframes": {
        "4hour": {
            "enabled": true,
            "timeframe": "4h",
            "sub-timeframe": "5m",
            "mode": "trend",
            "depth": 25,
            "history": 25,
            "indicators": {
                "price": {"method": "HLC"},
                "slow_h_ma": {"len": 20},
                "slow_m_ma": {"len": 20},
                "slow_l_ma": {"len": 20}
            }
        },
         "5min": {
            "enabled": true,
            "timeframe": "5m",
            "sub-timeframe": "1m",
            "mode": "sig",
            "depth": 25,
            "history": 25,
            "indicators": {
                "price": {"method": "HLC"},
                "fast_h_ma": {"len": 6},
                "fast_m_ma": {"len": 6},
                "fast_l_ma": {"len": 6},
                "adx": {"len": 5}
            }
        },
        "1min": {
            "enabled": true,
            "timeframe": "1m",
            "sub-timeframe": "t",
            "mode": "conf",
            "depth": 10,
            "history": 10,
            "indicators": {
                "price": {"method": "HLC"}
            }
        }
    }
})JSON";

} // namespace siis

#endif // SIIS_MAADXPARAMETERS_H
