 /**
 * @brief SiiS base.
 * @copyright Copyright (C) 2019 SiiS
 * @author Frederic SCHERMA (frederic.scherma@gmail.com)
 * @date 2019-03-05
 */

#ifndef SIIS_BASE_HPP
#define SIIS_BASE_HPP

#include <o3d/core/base.h>

// strict in type declaration
#ifndef STRICT
    #define STRICT
#endif

#define SIIS_LIBRARY

// If no object export/import mode defined suppose IMPORT
#if !defined(SIIS_LIBRARY)
  #define SIIS_API
  #define SIIS_API_PRIVATE
  #define SIIS_API_TEMPLATE
#else
  #if (defined(O3D_UNIX) || defined(O3D_MACOSX) || defined(SWIG))
    #if (__GNUC__ >= 4) || defined(__clang__)
      #define SIIS_API __attribute__ ((visibility ("default")))
      #define SIIS_API_PRIVATE __attribute__ ((visibility ("hidden")))
      #define SIIS_API_TEMPLATE
    #else
      #define SIIS_API
      #define SIIS_API_PRIVATE
      #define SIIS_API_TEMPLATE
    #endif
  #endif
#endif

#if !defined(SIIS_PLUGIN)
  #if (defined(O3D_UNIX) || defined(O3D_MACOSX) || defined(SWIG))
    #if (__GNUC__ >= 4) || defined(__clang__)
      #define SIIS_PLUGIN_API __attribute__ ((visibility ("default")))
      #define SIIS_PLUGIN_API_PRIVATE __attribute__ ((visibility ("hidden")))
      #define SIIS_PLUGIN_API_TEMPLATE
    #else
      #define SIIS_PLUGIN_API
      #define SIIS_PLUGIN_API_PRIVATE
      #define SIIS_PLUGIN_API_TEMPLATE
    #endif
  #endif
#endif

#endif // SIIS_BASE_HPP
