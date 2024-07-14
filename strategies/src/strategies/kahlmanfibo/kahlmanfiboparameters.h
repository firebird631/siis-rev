/**
 * @brief SiiS strategy KahlmanFibo default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-08-31
 */

#ifndef SIIS_KAHLMANFIBOPARAMETERS_H
#define SIIS_KAHLMANFIBOPARAMETERS_H

namespace siis {

/**
 * @brief Strategy KahlmanFibo default parameters.
 * @author Frederic Scherma
 * @date 2023-08-31
 */
static const char* KahlmanFiboParameters = R"JSON(
{
    "reversal": true,
    "hedging": false,
    "allow-short": true,
    "max-trades": 3,
    "trade-delay": 30,
    "need-update": false,
    "trade-type": "ind-margin",
    "kahlman": false,
    "contexts": {
        "kf-5m-1m": {
            "mode": "trade",
            "method": "standard",
            "max-trades": 3,
            "min-profit": 0.5,
            "sig": {"type": "hma+donchian", "timeframe": "5m", "max-wide": 5, "one-way": false},
            "confirm": {"type": "candle", "timeframe": "1m", "risk-reward": 1.0, "target-scale": 10.0},
            "entry": {"type": "last", "timeframe": "5m", "depth": 1, "orientation": "up", "timeout": "1h"},
            "take-profit": {"type": "fixed", "distance": "1.25%", "timeframe": "5m", "depth": 1, "orientation": "up"},
            "stop-loss": {"type": "fixed", "distance": "0.75%", "timeframe": "5m", "depth": 2, "orientation": "dn"},
            "breakeven": {"type": "fixed", "timeframe": "1m", "distance": "1.0%"},
            "dynamic-stop-loss": {"type": "fixed", "distance": "1.0%", "timeframe": "5m"}
        }
    },
    "timeframes": {
         "5min": {
            "enabled": true,
            "timeframe": "5m",
            "sub-timeframe": "1m",
            "mode": "sig",
            "depth": 50,
            "history": 50,
            "indicators": {
                "price": {"method": "HL"},
                "donchian": {"len": 24},
                "hma": {"len": 22},
                "hma3": {"len": 11}
            }
        },
        "1min": {
            "enabled": true,
            "timeframe": "1m",
            "sub-timeframe": "t",
            "mode": "conf",
            "depth": 5,
            "history": 5,
            "indicators": {
                "price": {"method": "HLC"}
            }
        }
    }
})JSON";

} // namespace siis

#endif // SIIS_KAHLMANFIBOPARAMETERS_H
