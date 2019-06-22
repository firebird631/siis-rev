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
    "maxTrades": 3,
    "tradeDelay": 30,
    "minTimeframe": 0,
    "needUpdate": false,
    "minVol24h": 0,
    "minPrice": 0,
    "timeframes": {
        "weekly": {
            "enabled": true,
            "timeframe": "1w",
            "subTimeframe": "daily",
            "mode": "C",
            "depth": 22,
            "history": 22,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeEma": {"len": 8},
                "rsi": {"len": 21},
                "stochRsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 21},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "daily": {
            "enabled": true,
            "timeframe": "1d",
            "subTimeframe": "4h",
            "mode": "A",
            "depth": 41,
            "history": 41,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeEma": {"len" : 8},
                "rsi": {"len" : 21},
                "stochRsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "4hours": {
            "enabled": true,
            "timeframe": "4h",
            "subTimeframe": "1h",
            "mode": "D",
            "depth": 56,
            "history": 56,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeEma": {"len" : 8},
                "rsi": {"len" : 21},
                "stochRsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
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
            "subTimeframe": "15m",
            "mode": "A",
            "depth": 41,
            "history": 41,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeEma": {"len" : 8},
                "rsi": {"len" : 21},
                "stochRsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "15min": {
            "enabled": true,
            "timeframe": "15m",
            "subTimeframe": "5m",
            "mode": "A",
            "depth": 41,
            "history": 41,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeEma": {"len" : 8},
                "rsi": {"len" : 21},
                "stochRsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "5min": {
            "enabled": true,
            "timeframe": "5m",
            "subTimeframe": "1m",
            "mode": "A",
            "depth": 41,
            "history": 41,
            "indicators": {
                "price": {"method": "HLC"},
                "volumeEma": {"len" : 8},
                "rsi": {"len" : 21},
                "stochRsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "1min": {
            "enabled": false,
            "timeframe": "1m",
            "subTimeframe": "t",
            "mode": "B",
            "depth": 20,
            "history": 20,
            "indicators": {
                "price": {"method": "HLC"},
                "rsi": {"len" : 8},
                "stochRsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        }
    }
})JSON";

} // namespace siis

#endif // SIIS_IAPARAMETERS_H
