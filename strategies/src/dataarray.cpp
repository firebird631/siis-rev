/**
 * @brief SiiS 16 bytes aligned double data array for manipulation.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-19
 */

#include "siis/dataarray.h"
#include "siis/utils/common.h"

#include <ta-lib/ta_func.h>

using namespace siis;
using o3d::Debug;
using o3d::Logger;

DataArray &DataArray::operator*=(const DataArray &a)
{
    O3D_ASSERT(getSize() == a.getSize());

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] *= a[i];
    }

    return *this;
}

DataArray &DataArray::operator*=(o3d::Double scale)
{
    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] *= scale;
    }

    return *this;
}

DataArray &DataArray::operator/=(const DataArray &a)
{
    O3D_ASSERT(getSize() == a.getSize());

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] /= a[i];
    }

    return *this;
}

DataArray &DataArray::operator+=(const DataArray &a)
{
    O3D_ASSERT(getSize() == a.getSize());

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] += a[i];
    }

    return *this;
}

DataArray &DataArray::operator-=(const DataArray &a)
{
    O3D_ASSERT(getSize() == a.getSize());

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] -= a[i];
    }

    return *this;
}

DataArray siis::DataArray::operator*(const DataArray &a)
{
    O3D_ASSERT(getSize() == a.getSize());

    DataArray r(getSize(), m_threshold);

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        r[i] = m_data[i] * a.m_data[i];
    }

    return r;
}

DataArray DataArray::operator*(o3d::Double scale)
{
    DataArray r(getSize(), m_threshold);

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        r[i] = m_data[i] * scale;
    }

    return r;
}

DataArray siis::DataArray::operator/(const DataArray &a)
{
    O3D_ASSERT(getSize() == a.getSize());

    DataArray r(getSize(), m_threshold);

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        r[i] = m_data[i] / a.m_data[i];
    }

    return r;
}

DataArray &DataArray::mult(const DataArray &a, const DataArray &b)
{
    O3D_ASSERT(a.getSize() == b.getSize());

    if (getSize() != a.getSize()) {
        setSize(a.getSize());
    }

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] = a.m_data[i] * b.m_data[i];
    }

    return *this;
}

DataArray &DataArray::mult(const DataArray &a, o3d::Double scale)
{
    if (getSize() != a.getSize()) {
        setSize(a.getSize());
    }

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] = a.m_data[i] * scale;
    }

    return *this;
}

DataArray &DataArray::div(const DataArray &a, const DataArray &b)
{
    O3D_ASSERT(a.getSize() == b.getSize());

    if (getSize() != a.getSize()) {
        setSize(a.getSize());
    }

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] = a.m_data[i] / b.m_data[i];
    }

    return *this;
}

DataArray &DataArray::add(const DataArray &a, const DataArray &b)
{
    O3D_ASSERT(a.getSize() == b.getSize());

    if (getSize() != a.getSize()) {
        setSize(a.getSize());
    }

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] = a.m_data[i] + b.m_data[i];
    }

    return *this;
}

DataArray &DataArray::sub(const DataArray &a, const DataArray &b)
{
    O3D_ASSERT(a.getSize() == b.getSize());

    if (getSize() != a.getSize()) {
        setSize(a.getSize());
    }

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        m_data[i] = a.m_data[i] - b.m_data[i];
    }

    return *this;
}

DataArray &DataArray::sma(o3d::Int32 len, DataArray &out) const
{
    int b, n;  // len-1 offset on output data
    TA_RetCode res = ::TA_SMA(0, getSize()-1, m_data, len, &b, &n, out.getData()+len-1);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == len-1);

    return out;
}

DataArray &DataArray::ema(o3d::Int32 len, DataArray &out) const
{
    int b, n;  // len-1 offset on output data
    TA_RetCode res = ::TA_EMA(0, getSize()-1, m_data, len, &b, &n, out.getData()+len-1);
    if (res != TA_SUCCESS) {
        O3D_WARNING(siis::taErrorToStr(res));
    }

    O3D_ASSERT(b == len-1);

    return out;
}

o3d::Int32 DataArray::cross(const DataArray &a) const
{
    o3d::Int32 size = getSize();

    if (a.getSize() != size || size < 2) {
        return 0;
    }

    if (m_data[size-2] > a.m_data[size-2] && m_data[size-1] < a.m_data[size-1]) {
        return -1;
    } else if (m_data[size-2] < a.m_data[size-2] && m_data[size-2] > a.m_data[size-2]) {
        return 1;
    }

    return 0;
}

DataArray siis::DataArray::operator+(const DataArray &a)
{
    O3D_ASSERT(getSize() == a.getSize());

    DataArray r(getSize(), m_threshold);

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        r[i] = m_data[i] + a.m_data[i];
    }

    return r;
}

DataArray siis::DataArray::operator-(const DataArray &a)
{
    O3D_ASSERT(getSize() == a.getSize());

    DataArray r(getSize(), m_threshold);

    for (o3d::Int32 i = 0; i < getSize(); ++i) {
        r[i] = m_data[i] - a.m_data[i];
    }

    return r;
}
