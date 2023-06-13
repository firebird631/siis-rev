/**
 * @brief SiiS strategy HMA MA default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-05-30
 */

#ifndef SIIS_HMAMAPARAMETERS_H
#define SIIS_HMAMAPARAMETERS_H

namespace siis {

/**
 * @brief Strategy HMA MA default parameters.
 * @author Frederic Scherma
 * @date 2023-05-30
 * @todo
 */
static const char* HmaMaParameters = R"JSON(
{
    "reversal": true,
    "hedging": false,
    "allow-short": true,
    "dual": false,
    "max-trades": 3,
    "trade-delay": 30,
    "need-update": false,
    "trade-type": "ind-margin",
    "contexts": {
        "hma-ma-5m-1m": {
            "mode": "trade",
            "method": "standard",
            "max-trades": 3,
            "min-profit": 0.5,
            "trend": {"type": "sma", "ma-timeframe": "4h"},
            "sig": {"type": "sma", "ma-timeframe": "5m", "min-adx": 40},
            "confirm": {"type": "candle", "timeframe": "1m", "risk-reward": 1.0, "target-scale": 10.0},
            "entry": {"type": "last", "timeframe": "5m", "depth": 1, "orientation": "up", "timeout": "1h"},
            "take-profit": {"type": "fixed-pct", "distance": "1.25%", "timeframe": "5m", "depth": 1, "orientation": "up"},
            "stop-loss": {"type": "fixed-pct", "distance": "0.75%", "timeframe": "5m", "depth": 2, "orientation": "dn"},
            "breakeven": {"type": "fixed-pct", "timeframe": "1m", "distance": "1.0%"},
            "dynamic-stop-loss": {"type": "fixed-pct", "distance": "1.0%", "timeframe": "5m"}
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

#endif // SIIS_HMAMAPARAMETERS_H
