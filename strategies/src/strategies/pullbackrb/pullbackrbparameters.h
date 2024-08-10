/**
 * @brief SiiS strategy pullback range-bar default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-09
 */

#ifndef SIIS_PULLBACKRBPARAMETERS_H
#define SIIS_PULLBACKRBPARAMETERS_H

namespace siis {

/**
 * @brief Strategy pullback range-bar default parameters.
 * @author Frederic Scherma
 * @date 2023-04-24
 */
static const char* PullbackRbParameters = R"JSON(
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
            "trend": {"type": "sma", "analyser": "trend"},
            "sig": {"type": "sma", "analyser": "sig", "min-adx": 40},
            "confirm": {"type": "candle", "analyser": "conf", "risk-reward": 1.0, "target-scale": 10.0},
            "entry": {"type": "last", "timeframe": "5m", "timeout": "1h"},
            "take-profit": {"type": "fixed", "timeframe": "5m", "distance": "1.25%"},
            "stop-loss": {"type": "fixed", "distance": "0.75%", "timeframe": "5m"},
            "breakeven": {"type": "fixed", "timeframe": "1m", "distance": "1.0%"},
            "dynamic-stop-loss": {"type": "fixed", "timeframe": "5m", "distance": "1.0%"}
        }
    },
    "tickbars": {
        "trend": {
            "enabled": true,
            "size": 200,
            "mode": "sr",
            "depth": 5,
            "history": 5,
            "indicators": {
                "price": {"method": "CLOSE"},
                "pivotpoint": {"method": "classical"}
            }
        },
         "sig": {
            "enabled": true,
            "size": 50,
            "mode": "bollinger",
            "depth": 45,
            "history": 45,
            "indicators": {
                "price": {"method": "CLOSE"},
                "bollinger": {"len": 20, "numDevUp": 2, "numDevDn": 2}
            }
        },
        "conf": {
            "enabled": true,
            "size": 10,
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

#endif // SIIS_PULLBACKRBPARAMETERS_H
