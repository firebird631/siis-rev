/**
 * @brief SiiS strategy Ichimoku default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#ifndef SIIS_ICHIMOKUSTPARAMETERS_H
#define SIIS_ICHIMOKUSTPARAMETERS_H

namespace siis {

/**
 * @brief Strategy Ichimoku default parameters.
 * @author Frederic Scherma
 * @date 2024-07-15
 */
static const char* IchimokuStParameters = R"JSON(
{
    "reversal": true,
    "hedging": false,
    "allow-short": true,
    "max-trades": 3,
    "trade-delay": 30,
    "need-update": false,
    "trade-type": "ind-margin",
    "contexts": {
        "ichimoku-st-32b-16b": {
            "mode": "trade",
            "method": "standard",
            "max-trades": 1,
            "min-profit": 0.5,
            "ichimoku": {"analyser": "5m"},
            "range-filter": {"type": "sma-hl", "analyser": "1m"},
            "confirm": {"type": "candle", "timeframe": "15s", "risk-bias": 1, "profit-scale": 2.0},
            "entry": {"type": "last", "timeframe": "5m", "timeout": "1h"},
            "take-profit": {"type": "fixed", "timeframe": "15s", "distance": "1.25%"},
            "stop-loss": {"type": "fixed", "timeframe": "15s", "distance": "0.75%"},
            "breakeven": {"type": "fixed", "timeframe": "15s", "distance": "1.0%"},
            "dynamic-stop-loss": {"type": "custom", "timeframe": "15s", "distance": "0.01%"}
        }
    },
    "timeframes": {
        "5min": {
            "enabled": true,
            "timeframe": "5m",
            "mode": "ichimoku",
            "depth": 105,
            "history": 105,
            "indicators": {
                "price": {"method": "close"},
                "ichimoku": {"tenkanSenLen": 9, "kijunSenLen": 26, "senkouSpanBLen": 52}
            }
        },
        "1min": {
            "enabled": true,
            "timeframe": "1m",
            "mode": "range-filter",
            "depth": 60,
            "history": 60,
            "indicators": {
                "price": {"method": "close"},
                "supertrend": {"len": 14, "coeff": 2.5},
                "fast_ma_high": {"len": 7},
                "fast_ma_low": {"len": 7},
                "slow_ma_high": {"len": 35},
                "slow_ma_low": {"len": 35}
            }
        },
        "15sec": {
            "enabled": true,
            "timeframe": "15s",
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

#endif // SIIS_ICHIMOKUSTPARAMETERS_H
