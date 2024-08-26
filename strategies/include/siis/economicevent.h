 /**
 * @brief SiiS economic event data model.
 * @copyright Copyright (C) 2024 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2024-08-05
 */

#ifndef SIIS_ECONOMICEVENT_H
#define SIIS_ECONOMICEVENT_H

#include "base.h"

#include <o3d/core/string.h>
#include <o3d/core/datetime.h>

namespace siis {

/**
 * @brief SiiS economic event data model.
 * @author Frederic Scherma
 * @date 2024-08-05
 */
class SIIS_API EconomicEventData
{
public:

    o3d::CString code;
    o3d::DateTime date;
    o3d::CString title;
    o3d::Int32 level {0};
    o3d::CString country;
    o3d::CString currency;
    o3d::CString previous;
    o3d::CString actual;
    o3d::CString forecast;
    o3d::CString reference;
    o3d::Int32 actualMeaning {-2};
    o3d::Int32 previousMeaning {-2};

    static o3d::String meaningToStr(o3d::Int32 meaning) {
        if (meaning == 1) {
            return "positive";
        } else if (meaning == 0) {
            return "neutral";
        } else if (meaning == -1) {
            return "negative";
        } else {
            return "unknown";
        }
    }

    inline o3d::String toString() const
    {
        return o3d::String("{0} at={1} UTC lvl={2} curr={3} (impact={4})")
                .arg(code.isEmpty() ? "-" : code).arg(date.buildString("%Y-%m-%d %H:%M").arg(level).arg(currency)
                .arg(EconomicEventData.meaningToStr(actualMeaning)));
    }

    inline o3d::Bool operator==(const EconomicEventData &cmp) const
    {
        // only compare with mandatory values (but sometime code is empty, but title will be)
        return ((date == cmp.date) && (level == cmp.level) &&
                (code == cmp.code) && (title == cmp.title) &&
                (country == cmp.country) && (currency == cmp.currency));
    }
};

} // namespace siis

#endif // SIIS_ECONOMICEVENT_H
