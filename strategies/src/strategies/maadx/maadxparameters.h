/**
 * @brief SiiS strategy MAADX default parameters.
 * @copyright Copyright (C) 2023 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2023-04-24
 */

#ifndef SIIS_MAADXPARAMETERS_H
#define SIIS_MAADXPARAMETERS_H

namespace siis {

/**
 * @brief Strategy MAADXdefault parameters.
 * @author Frederic Scherma
 * @date 2023-04-24
 */
static const char* MaAdxParameters = R"JSON(
{
    "reversal": true,
    "pyramided": 0,
    "hedging": false,
    "max-trades": 3,
    "trade-delay": 30,
    "need-update": false,
    "timeframes": {
        "4hours": {
            "enabled": true,
            "timeframe": "4h",
            "sub-timeframe": "5m",
            "mode": "A",
            "depth": 56,
            "history": 56,
            "indicators": {
                "price": {"method": "HLC"},
                "rsi": {"len" : 21},
                "stochrsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "slowSma": {"len": 200},
                "midSma": {"len": 55},
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
                "rsi": {"len" : 21},
                "stochrsi": {"len": 13, "fastK_Len": 13, "fastD_Len": 13},
                "sma": {"len": 20},
                "ema": {"len": 8},
                "atr": {"len": 14, "factor": 2.5}
            }
        },
        "1min": {
            "enabled": true,
            "timeframe": "1m",
            "sub-timeframe": "t",
            "mode": "B",
            "depth": 20,
            "history": 20,
            "indicators": {
                "price": {"method": "HLC"}
            }
        }
    }
})JSON";

} // namespace siis

#endif // SIIS_MAADXPARAMETERS_H
