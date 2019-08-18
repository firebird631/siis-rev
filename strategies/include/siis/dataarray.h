/**
 * @brief SiiS 16 bytes aligned double data array for manipulation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#ifndef SIIS_DATAARRAY_H
#define SIIS_DATAARRAY_H

#include "base.h"

#include <o3d/core/templatearray.h>

namespace siis {

/**
 * @brief SiiS 16 bytes aligned double data array for manipulation.
 * @author Frederic Scherma
 * @date 2019-03-15
 */
class SIIS_API DataArray : public o3d::TemplateArray<o3d::Double, 16>
{
public:

    DataArray(o3d::Int32 isize = 256, o3d::Int32 ithres = 256) :
        o3d::TemplateArray<o3d::Double, 16>(isize, ithres)
    {
    }

    DataArray(const DataArray &dup) :
        o3d::TemplateArray<o3d::Double, 16>(dup)
    {
    }

    DataArray& operator= (const DataArray &dup)
    {
        o3d::TemplateArray<o3d::Double, 16>::operator=(dup);
        return *this;
    }

    DataArray& operator*= (const DataArray &a);
    DataArray& operator*= (o3d::Double scale);
    DataArray& operator/= (const DataArray &a);
    DataArray& operator+= (const DataArray &a);
    DataArray& operator-= (const DataArray &a);

    DataArray operator+ (const DataArray &a);
    DataArray operator- (const DataArray &a);
    DataArray operator* (const DataArray &a);
    DataArray operator* (o3d::Double scale);
    DataArray operator/ (const DataArray &a);

    DataArray& mult(const DataArray &a, const DataArray &b);
    DataArray& mult(const DataArray &a, o3d::Double scale);
    DataArray& div(const DataArray &a, const DataArray &b);
    DataArray& add(const DataArray &a, const DataArray &b);
    DataArray& sub(const DataArray &a, const DataArray &b);

    DataArray& sma(o3d::Int32 len, DataArray &out) const;
    DataArray& ema(o3d::Int32 len, DataArray &out) const;

    /**
     * @brief cross With the two last values
     * @return -1, 1 or 0 (this cross under a, this cross upper a, none)
     */
    o3d::Int32 cross(const DataArray &a) const;
};

} // namespace siis

#endif // SIIS_DATAARRAY_H
