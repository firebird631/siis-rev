/**
 * @brief SiiS math utils.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-07
 */

#ifndef SIIS_MATH_H
#define SIIS_MATH_H

#include "../base.h"
#include "../constants.h"

#include <cmath>

namespace siis {

template<class T>
inline T truncate(T number, o3d::Int32 digits)
{
    T stepper = pow(10.0, digits);
    return trunc(stepper * number) / stepper;
}

template<class T>
inline T round(T number, o3d::Int32 decimals)
{
    return floor((number*pow(10.0, decimals))+0.5) / pow(10.0, decimals);
}

} // namespace siis

#endif // SIIS_MATH_H
