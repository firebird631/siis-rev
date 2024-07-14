/**
 * @brief SiiS strategy forexalpha default parameters.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_FAPARAMETERS_H
#define SIIS_FAPARAMETERS_H

namespace siis {

/**
 * @brief Strategy forexalpha default parameters.
 * @author Frederic Scherma
 * @date 2019-03-17
 */
static const char* ForexAlphaParameters = R"JSON(
{
    "reversal": true,
    "hedging": false,
    "max-trades": 3,
    "trade-delay": 30,
    "min-timeframe": 0,
    "need-update": false,
    "min-vol-24h": 0,
    "min-price": 0,
    "trade-type": "position",
    "timeframes": {
        "weekly": {
            "enabled": true,
            "timeframe": "1w",
            "mode": "C",
            "depth": 22,
            "history": 22,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeema": {"len": 8},
                "rsi": {"len": 21},
                "stochrsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 21},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "daily": {
            "enabled": true,
            "timeframe": "1d",
            "mode": "A",
            "depth": 41,
            "history": 41,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeema": {"len" : 8},
                "rsi": {"len" : 21},
                "stochrsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "4hour": {
            "enabled": true,
            "timeframe": "4h",
            "mode": "D",
            "depth": 56,
            "history": 56,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeema": {"len" : 8},
                "rsi": {"len" : 21},
                "stochrsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "slowSma": {"len": 200},
                "midSma": {"len": 55},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "hourly": {
            "enabled": true,
            "timeframe": "1h",
            "mode": "A",
            "depth": 41,
            "history": 41,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeema": {"len" : 8},
                "rsi": {"len" : 21},
                "stochrsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "15min": {
            "enabled": true,
            "timeframe": "15m",
            "mode": "A",
            "depth": 41,
            "history": 41,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeema": {"len" : 8},
                "rsi": {"len" : 21},
                "stochrsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "5min": {
            "enabled": true,
            "timeframe": "5m",
            "mode": "A",
            "depth": 41,
            "history": 41,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeema": {"len" : 8},
                "rsi": {"len" : 21},
                "stochrsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "1min": {
            "enabled": false,
            "timeframe": "1m",
            "mode": "B",
            "depth": 20,
            "history": 20,
            "indicators": {
                "price": {"method": "HLC"},
                "rsi": {"len" : 8},
                "stochrsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        }
    }
})JSON";

} // namespace siis

#endif // SIIS_FAPARAMETERS_H
