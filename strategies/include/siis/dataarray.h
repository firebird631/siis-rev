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

    inline o3d::Double last() const { return m_size < 1 ? 0.0 : m_data[m_size-1]; }
    inline o3d::Double prev() const { return m_size < 2 ? 0.0 : m_data[m_size-2]; }

    /**
     * @brief at Return the value at index and works with negative index.
     * @param i Positive or negative index or 0
     */
    inline const o3d::Double& at(o3d::Int32 n) const {
        if (n >= m_size) throw o3d::E_IndexOutOfRange("DataArray:at(n>=size)");

        if (n < 0) {
            n = m_size + n;
            if (n < 0) throw o3d::E_IndexOutOfRange("DataArray:at(n<0)");
        }

        return m_data[n];
    }

    /**
     * @brief at Changes the value at index and works with negative index.
     * @param i Positive or negative index or 0
     * @param v New value to set
     */
    inline void set(o3d::Int32 n, o3d::Double v) const {
        if (n >= m_size) throw o3d::E_IndexOutOfRange("DataArray:set(n>=size)");

        if (n < 0) {
            n = m_size + n;
            if (n < 0) throw o3d::E_IndexOutOfRange("DataArray:set(n<0)");
        }

        m_data[n] = v;
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

    /**
     * @brief zero Make all values set to zero
     * @param len Use this len in place of array len if greater than 0.
     */
    DataArray& zero(o3d::Int32 len=-1);

    /**
     * @brief nan Make all values set to NaN.
     * @param len Use this len in place of array len if greater than 0.
     */
    DataArray& nan(o3d::Int32 len=-1);

    /**
     * @brief zeroNan Replace any nan value by zero.
     * @param len Use this len in place of array len if greater than 0.
     */
    DataArray& zeroNan(o3d::Int32 len=-1);

    /**
     * @brief defaultNan Replace any nan value by given default value.
     * @param len Use this len in place of array len if greater than 0.
     * @param v Default value.
     */
    DataArray& defaultNan(o3d::Int32 len, o3d::Double v);

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

    /**
     * @brief cross With the two last values and a single value
     * @return -1, 1 or 0 (this cross under a, this cross upper a, none)
     */
    o3d::Int32 cross(o3d::Double value) const;

    /**
     * @brief cross With the two last values
     * @return -1, 1 or 0 (this cross under a, this cross upper a, none)
     */
    static o3d::Int32 cross(const DataArray &a, const DataArray &b);

    /**
     * @brief cross With the two last values and a single value
     * @return -1, 1 or 0 (this cross under a, this cross upper a, none)
     */
    static o3d::Int32 cross(const DataArray &a, o3d::Double value);
};

} // namespace siis

#endif // SIIS_DATAARRAY_H
