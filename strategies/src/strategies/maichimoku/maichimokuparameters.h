/**
 * @brief SiiS strategy MA Ichimoku default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-25
 */

#ifndef SIIS_MAICHIMOKUPARAMETERS_H
#define SIIS_MAICHIMOKUPARAMETERS_H

namespace siis {

/**
 * @brief Strategy MA Ichimoku default parameters.
 * @author Frederic Scherma
 * @date 2023-05-25
 */
static const char* MaIchimokuParameters = R"JSON(
{
    "reversal": true,
    "hedging": false,
    "allow-short": true,
    "max-trades": 3,
    "trade-delay": 30,
    "need-update": false,
    "trade-type": "ind-margin",
    "contexts": {
        "ma-ichimoku-5m-1m": {
            "mode": "trade",
            "method": "standard",
            "max-trades": 3,
            "min-profit": 0.5,
            "trend": {"type": "sma", "analyser": "4hour"},
            "sig": {"type": "sma", "analyser": "5min", "min-adx": 40},
            "confirm": {"type": "candle", "analyser": "1min", "risk-reward": 1.0, "target-scale": 10.0},
            "entry": {"type": "last", "timeframe": "5m", "timeout": "1h"},
            "take-profit": {"type": "fixed", "timeframe": "5m", "distance": "1.25%"},
            "stop-loss": {"type": "fixed", "timeframe": "5m", "distance": "0.75%"},
            "breakeven": {"type": "fixed", "timeframe": "1m", "distance": "1.0%"},
            "dynamic-stop-loss": {"type": "fixed", "timeframe": "5m", "distance": "1.0%"}
        }
    },
    "timeframes": {
        "4hour": {
            "enabled": true,
            "timeframe": "4h",
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

#endif // SIIS_MAICHIMOKUPARAMETERS_H
