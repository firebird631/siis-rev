/**
 * @brief SiiS strategy SuperTrend range-bar default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#ifndef SIIS_SUPERTRENDRBPARAMETERS_H
#define SIIS_SUPERTRENDRBPARAMETERS_H

namespace siis {

/**
 * @brief Strategy SuperTrend range bar default parameters.
 * @author Frederic Scherma
 * @date 2023-09-30
 */
static const char* SuperTrendRbParameters = R"JSON(
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
        "st-16b-8b": {
            "mode": "trade",
            "method": "standard",
            "max-trades": 1,
            "min-profit": 0.5,
            "sig": {"type": "supertrend", "analyser": "16tick"},
            "confirm": {"type": "bar", "analyser": "8tick", "risk-reward": 0.0, "profit-scale": 0.0},
            "entry": {"type": "last", "timeframe": "15s", "timeout": "1h"},
            "take-profit": {"type": "fixed", "timeframe": "15s", "distance": "1.25%"},
            "stop-loss": {"type": "fixed", "timeframe": "15s", "distance": "0.75%"},
            "breakeven": {"type": "fixed", "timeframe": "15s", "distance": "1.0%"},
            "dynamic-stop-loss": {"type": "custom", "timeframe": "15s", "distance": "0.01%"}
        }
    },
    "tickbars": {
        "16tick": {
            "enabled": true,
            "tickbar": "16",
            "mode": "sig",
            "depth": 50,
            "history": 50,
            "indicators": {
                "price": {"method": "HLC"},
                "supertrend": {"len": 14, "coeff": 2.5},
                "hma": {"len": 22},
                "hma3": {"len": 11}
            }
        },
        "8tick": {
            "enabled": true,
            "tickbar": "8",
            "mode": "conf",
            "depth": 5,
            "history": 5,
            "indicators": {
                "price": {"method": "close"}
            }
        }
    }
})JSON";

} // namespace siis

#endif // SIIS_SUPERTRENDRBPARAMETERS_H
