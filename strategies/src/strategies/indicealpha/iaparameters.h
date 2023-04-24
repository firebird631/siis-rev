/**
 * @brief SiiS strategy indicealpha default parameters.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-17
 */

#ifndef SIIS_IAPARAMETERS_H
#define SIIS_IAPARAMETERS_H

namespace siis {

/**
 * @brief Strategy indicealpha default parameters.
 * @author Frederic Scherma
 * @date 2019-06-03
 */
static const char* IndiceAlphaParameters = R"JSON(
{
    "reversal": true,
    "pyramided": 0,
    "hedging": false,
    "max-trades": 3,
    "trade-delay": 30,
    "minTimeframe": 0,
    "need-update": false,
    "baseTimeframe": "t",
    "minTradedTimeframe': "1m",
    "maxTradedTimeframe': "1m",
    "minVol24h": 0,
    "minPrice": 0,
    "timeframes": {
        "weekly": {
            "enabled": true,
            "timeframe": "1w",
            "sub-timeframe": "daily",
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
            "sub-timeframe": "4h",
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
        "4hours": {
            "enabled": true,
            "timeframe": "4h",
            "sub-timeframe": "1h",
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
            "sub-timeframe": "15m",
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
            "sub-timeframe": "5m",
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
            "sub-timeframe": "1m",
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
            "sub-timeframe": "t",
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

#endif // SIIS_IAPARAMETERS_H
