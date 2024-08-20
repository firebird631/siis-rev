/**
 * @brief SiiS strategy MAADX range-bar default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_MAADXRBPARAMETERS_H
#define SIIS_MAADXRBPARAMETERS_H

namespace siis {

/**
 * @brief Strategy MAADX range-bar default parameters.
 * @author Frederic Scherma
 * @date 2023-04-24
 */
static const char* MaAdxRbParameters = R"JSON(
{
    "reversal": true,
    "hedging": false,
    "allow-short": true,
    "max-trades": 3,
    "trade-delay": 30,
    "need-update": false,
    "trade-type": "ind-margin",
    "contexts": {
        "ma-adx-50rb-10rb": {
            "mode": "trade",
            "method": "standard",
            "max-trades": 1,
            "min-profit": 0.1,
            "trend": {"type": "sma", "analyser": "trend"},
            "sig": {"type": "sma", "analyser": "sig", "min-adx": 40},
            "confirm": {"type": "candle", "analyser": "conf", "target-scale": 20, "risk-reward": 0.5},
            "entry": {"type": "last", "timeframe": "5m", "timeout": "1h"},
            "take-profit": {"type": "fixed", "timeframe": "5m", "distance": "0.25%"},
            "stop-loss": {"type": "fixed", "timeframe": "5m", "distance": "0.15%"}
        }
    },
    "analysers": {
        "trend": {
            "enabled": true,
            "type": "range-bar",
            "size": 200,
            "mode": "trend",
            "depth": 25,
            "history": 25,
            "indicators": {
                "price": {"method": "close"},
                "slow_h_ma": {"len": 20},
                "slow_m_ma": {"len": 20},
                "slow_l_ma": {"len": 20}
            }
        },
         "sig": {
            "enabled": true,
            "type": "range-bar",
            "size": 50,
            "mode": "sig",
            "depth": 25,
            "history": 25,
            "indicators": {
                "price": {"method": "close"},
                "fast_h_ma": {"len": 6},
                "fast_m_ma": {"len": 6},
                "fast_l_ma": {"len": 6},
                "adx": {"len": 5}
            }
        },
        "conf": {
            "enabled": true,
            "type": "range-bar",
            "size": 10,
            "mode": "conf",
            "depth": 10,
            "history": 10,
            "indicators": {
                "price": {"method": "close"}
            }
        }
    }
})JSON";

} // namespace siis

#endif // SIIS_MAADXRBPARAMETERS_H
