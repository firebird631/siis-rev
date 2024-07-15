/**
 * @brief SiiS strategy pullback default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#ifndef SIIS_PULLBACKPARAMETERS_H
#define SIIS_PULLBACKPARAMETERS_H

namespace siis {

/**
 * @brief Strategy pullback default parameters.
 * @author Frederic Scherma
 * @date 2023-04-24
 */
static const char* PullbackParameters = R"JSON(
{
    "reversal": true,
    "hedging": false,
    "allow-short": true,
    "max-trades": 3,
    "trade-delay": 30,
    "need-update": false,
    "trade-type": "ind-margin",
    "contexts": {
        "pullback-5m-1m": {
            "mode": "trade",
            "method": "standard",
            "max-trades": 3,
            "min-profit": 0.5,
            "trend": {"type": "sma", "analyser": "4hour"},
            "sig": {"type": "sma", "analyser": "5min", "min-adx": 40},
            "confirm": {"type": "candle", "analyser": "1min", "risk-reward": 1.0, "target-scale": 10.0},
            "entry": {"type": "last", "timeframe": "5m", "timeout": "1h"},
            "take-profit": {"type": "fixed", "timeframe": "5m", "distance": "1.25%"},
            "stop-loss": {"type": "fixed", "distance": "0.75%", "timeframe": "5m"},
            "breakeven": {"type": "fixed", "timeframe": "1m", "distance": "1.0%"},
            "dynamic-stop-loss": {"type": "fixed", "timeframe": "5m", "distance": "1.0%"}
        }
    },
    "timeframes": {
        "30min": {
            "enabled": true,
            "timeframe": "30m",
            "mode": "sr",
            "depth": 5,
            "history": 5,
            "indicators": {
                "price": {"method": "CLOSE"},
                "pivotpoint": {"method": "classical"}
            }
        },
         "5min": {
            "enabled": true,
            "timeframe": "5m",
            "mode": "bollinger",
            "depth": 45,
            "history": 45,
            "indicators": {
                "price": {"method": "CLOSE"},
                "bollinger": {"len": 20, "numDevUp": 2, "numDevDn": 2}
            }
        },
        "1min": {
            "enabled": true,
            "timeframe": "1m",
            "mode": "conf",
            "depth": 5,
            "history": 5,
            "indicators": {
                "price": {"method": "CLOSE"}
            }
        }
    }
})JSON";

} // namespace siis

#endif // SIIS_PULLBACKPARAMETERS_H
