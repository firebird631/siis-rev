/**
 * @brief SiiS strategy SuperTrend default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-09-30
 */

#ifndef SIIS_SUPERTRENDPARAMETERS_H
#define SIIS_SUPERTRENDPARAMETERS_H

namespace siis {

/**
 * @brief Strategy SuperTrend default parameters.
 * @author Frederic Scherma
 * @date 2023-09-30
 */
static const char* SuperTrendParameters = R"JSON(
{
    "reversal": true,
    "hedging": false,
    "allow-short": true,
    "dual": false,
    "max-trades": 3,
    "trade-delay": 30,
    "need-update": false,
    "trade-type": "ind-margin",
    "kahlman": false,
    "contexts": {
        "st-5m-1m": {
            "mode": "trade",
            "method": "standard",
            "max-trades": 1,
            "min-profit": 0.5,
            "sig": {"type": "supertrend", "timeframe": "5m"},
            "confirm": {"type": "candle", "timeframe": "1m", "risk-reward": 0.0, "profit-scale": 0.0},
            "entry": {"type": "last", "timeframe": "5m", "timeout": "1h"},
            "take-profit": {"type": "fixed", "timeframe": "1m", "distance": "1.25%"},
            "stop-loss": {"type": "fixed", "timeframe": "1m", "distance": "0.75%"},
            "breakeven": {"type": "fixed", "timeframe": "5m", "distance": "1.0%"},
            "dynamic-stop-loss": {"type": "custom", "timeframe": "5m", "distance": "0.01%"}
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
                "price": {"method": "HLC"},
                "supertrend": {"len": 14, "coeff": 2.5},
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
                "price": {"method": "close"}
            }
        }
    }
})JSON";

} // namespace siis

#endif // SIIS_SUPERTRENDPARAMETERS_H
