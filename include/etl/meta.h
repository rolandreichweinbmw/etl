///\file

/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
https://www.etlcpp.com

Copyright(c) 2026 BMW AG

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef ETL_META_INCLUDED
#define ETL_META_INCLUDED

#include "platform.h"
#include "concepts.h"
#include "iterator.h"
#include "string_view.h"
#include "type_traits.h"

// The Bloomberg clang-p2996 fork only enables P3096 function parameter
// reflection when the -fparameter-reflection option is used.
#if defined(__has_feature)
  #if __has_feature(parameter_reflection)
    #define ETL_META_HAS_CLANG_PARAMETER_REFLECTION 1
  #else
    #define ETL_META_HAS_CLANG_PARAMETER_REFLECTION 0
  #endif
#else
  #define ETL_META_HAS_CLANG_PARAMETER_REFLECTION 1
#endif

// Case 1: Via the standard <meta> header from STL, in ETL_USING_STL case for clang and gcc
#if ETL_USING_STL
  #include <initializer_list>
  #include <optional>
  #include <string>
  #include <string_view>
  #include <vector>
  #if __has_include(<meta>)
    #include <meta>
    #include <source_location> // Workaround for GCC 16: <meta> depends on <source_location> but doesn't include it.
    #define ETL_HAS_REFLECTION       1
    #if defined(__clang__)
      #define ETL_HAS_PARAMETER_REFLECTION ETL_META_HAS_CLANG_PARAMETER_REFLECTION
    #else
      #define ETL_HAS_PARAMETER_REFLECTION 1
    #endif
    #include "private/meta/stl.h"
  #else
    #define ETL_HAS_REFLECTION       0
    #define ETL_HAS_PARAMETER_REFLECTION 0
  #endif

// Case 2: Via bloomberg p2996 clang (shows version 21.0.0)
#elif defined(ETL_COMPILER_CLANG) && ETL_COMPILER_FULL_VERSION == 210000
#define ETL_HAS_REFLECTION           1
#define ETL_HAS_PARAMETER_REFLECTION ETL_META_HAS_CLANG_PARAMETER_REFLECTION
#include "private/meta/clang-p2996.h"

// Case 3: GCC 16 with reflection support
#elif defined(ETL_COMPILER_GCC) && __cpp_impl_reflection >= 202506L
// Just activate for now, the detection might still be incorrect by experimental compilers
#define ETL_HAS_REFLECTION           1
#define ETL_HAS_PARAMETER_REFLECTION 1
#include "private/meta/gcc.h"

// Case 4: Clang with reflection
// TODO: Not yet available. Test if detection is correct.
#elif defined(ETL_COMPILER_CLANG) && __cpp_impl_reflection >= 202506L
#define ETL_HAS_REFLECTION           1
#define ETL_HAS_PARAMETER_REFLECTION 1
#include "private/meta/clang.h"

// Case 5: Unsupported
#else
#define ETL_HAS_REFLECTION 0
#define ETL_HAS_PARAMETER_REFLECTION 0
#endif

// Optional reflection facilities that are not provided by every implementation.
#if !defined(ETL_HAS_META_REFLECT_INVOKE)
  #define ETL_HAS_META_REFLECT_INVOKE 0
#endif
#if !defined(ETL_HAS_META_IS_ACCESS_SPECIFIED)
  #define ETL_HAS_META_IS_ACCESS_SPECIFIED 0
#endif
#if !defined(ETL_META_NEEDS_INFO_ARRAY)
  #define ETL_META_NEEDS_INFO_ARRAY 0
#endif
#if !defined(ETL_HAS_META_IS_FINAL)
  #define ETL_HAS_META_IS_FINAL 0
#endif
#if !defined(ETL_HAS_META_BIT_WIDTH_OPTION)
  #define ETL_HAS_META_BIT_WIDTH_OPTION 0
#endif

#endif // ETL_META_INCLUDED
