/**
 * @brief SiiS strategy Ichimoku range-bar default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-07-15
 */

#ifndef SIIS_ICHIMOKUSTRBPARAMETERS_H
#define SIIS_ICHIMOKUSTRBPARAMETERS_H

namespace siis {

/**
 * @brief Strategy Ichimoku range-bar default parameters.
 * @author Frederic Scherma
 * @date 2024-07-15
 */
static const char* IchimokuStRbParameters = R"JSON(
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
            "ichimoku": {"analyser": "32tick"},
            "range-filter": {"type": "sma-hl", "analyser": "16tick"},
            "confirm": {"type": "bar", "analyser": "8tick", "risk-bias": 1, "profit-scale": 2.0},
            "entry": {"type": "last", "timeframe": "5m", "timeout": "1h"},
            "take-profit": {"type": "fixed", "timeframe": "15s", "distance": "1.25%"},
            "stop-loss": {"type": "fixed", "timeframe": "15s", "distance": "0.75%"},
            "breakeven": {"type": "fixed", "timeframe": "15s", "distance": "1.0%"},
            "dynamic-stop-loss": {"type": "custom", "timeframe": "15s", "distance": "0.01%"}
        }
    },
    "tickbars": {
        "32tick": {
            "enabled": true,
            "size": 32,
            "mode": "ichimoku",
            "depth": 105,
            "history": 105,
            "indicators": {
                "price": {"method": "close"},
                "ichimoku": {"tenkanSenLen": 9, "kijunSenLen": 26, "senkouSpanBLen": 52}
            }
        },
        "16tick": {
            "enabled": true,
            "size": 16,
            "mode": "range-filter",
            "depth": 60,
            "history": 60,
            "indicators": {
                "price": {"method": "close"},
                "fast_ma_high": {"len": 7},
                "fast_ma_low": {"len": 7},
                "slow_ma_high": {"len": 35},
                "slow_ma_low": {"len": 35}
            }
        },
        "8tick": {
            "enabled": true,
            "size": 8,
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

#endif // SIIS_ICHIMOKUSTRBPARAMETERS_H
