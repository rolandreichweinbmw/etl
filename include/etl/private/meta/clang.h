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

#ifndef ETL_PRIVATE_META_CLANG_H_INCLUDED
#define ETL_PRIVATE_META_CLANG_H_INCLUDED

#define ETL_HAS_META_REFLECT_INVOKE       0
#define ETL_HAS_META_IS_ACCESS_SPECIFIED  0
#define ETL_META_NEEDS_INFO_ARRAY         0
#define ETL_HAS_META_IS_FINAL             0
#define ETL_HAS_META_BIT_WIDTH_OPTION     0

///\defgroup meta meta
///\ingroup etl

namespace etl
{
  namespace meta
  {
    // Clang non-STL reflection: not yet implemented.
    // TODO: Not yet available. Test if detection is correct.
  } // namespace meta
} // namespace etl

#endif // ETL_PRIVATE_META_CLANG_H_INCLUDED
