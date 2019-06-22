/**
 * @brief SiiS strategy base signal proxy.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-21
 */

#ifndef SIIS_BASESIGNAL_H
#define SIIS_BASESIGNAL_H

#include "../base.h"

namespace siis {

/**
 * @brief Strategy base signal proxy.
 * @author Frederic Scherma
 * @date 2019-03-21
 */
class SIIS_API BaseSignal
{
public:

    enum BaseType {
        STATUS = 0,
        ACCOUNT = 1,
        ORDER = 2,
        POSITION = 3,
        MARKET = 4,
        ASSET = 5
    };

    static constexpr o3d::Double TIMESTAMP_UNDEFINED = -1.0;
    static constexpr o3d::Double PRICE_UNDEFINED = -1.0;
    static constexpr o3d::Double QUANTITY_UNDEFINED = -1.0;
    static constexpr o3d::Double RATE_UNDEFINED = -1.0;
    static constexpr o3d::Int8 FLAG_UNDEFINED = -1;
    static constexpr o3d::Int8 VALUE_UNDEFINED = -1;    

    BaseSignal(BaseType baseType) : m_baseType(baseType) {}

    /**
     * @brief baseType Base type of the signal.
     */
    BaseType baseType() const { return m_baseType; }

private:

    BaseType m_baseType;
};

} // namespace siis

#endif // SIIS_BASESIGNAL_H
