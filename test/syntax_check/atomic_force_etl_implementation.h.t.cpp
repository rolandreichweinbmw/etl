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

#include <etl/platform.h>

// Force the ETL implementation of etl::atomic (using the compiler built-ins) in
// preference to std::atomic, even when the STL is available. Only the Arm, GCC and
// Clang compilers provide an ETL implementation, so restrict the override to those.
#if defined(ETL_COMPILER_ARM5) || defined(ETL_COMPILER_ARM6) || defined(ETL_COMPILER_GCC) || defined(ETL_COMPILER_CLANG)
  #define ETL_ATOMIC_FORCE_ETL_IMPLEMENTATION
#endif

#include <etl/atomic.h>

// Verify that the option actually selected the ETL implementation over std::atomic.
#if defined(ETL_ATOMIC_FORCE_ETL_IMPLEMENTATION) && defined(ETL_ATOMIC_STD_INCLUDED)
  #error ETL_ATOMIC_FORCE_ETL_IMPLEMENTATION did not override std::atomic
#endif
