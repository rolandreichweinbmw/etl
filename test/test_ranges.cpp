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

#include "unit_test_framework.h"

#include "etl/ranges.h"
#include "etl/vector.h"

#include <array>
#include <list>
#include <vector>

#include <ios>

#if ETL_USING_CPP17

// C++03 does not support move semantics as used in the ranges library
#if !defined(ETL_FORCE_TEST_CPP03_IMPLEMENTATION)

namespace
{
  class MoveInt
  {
    public:
      MoveInt(int v): _v{v}
      {
      }

      MoveInt(const MoveInt& other): _v{other._v}
      {
      }

      MoveInt& operator=(const MoveInt& other)
      {
        _v = other._v;
        return *this;
      }

      MoveInt(MoveInt&& other): _v{other._v}
      {
        other._v = 0;
      }

      MoveInt& operator=(MoveInt&& other)
      {
        _v = other._v;
        other._v = 0;
        return *this;
      }

      int get() const
      {
        return _v;
      }

      bool operator==(const MoveInt& other) const
      {
        return _v == other._v;
      }

      bool operator!=(const MoveInt& other) const
      {
        return !(*this == other);
      }

    private:
      int _v;
  };

}

namespace std
{
std::ostringstream& operator<<(std::ostringstream& s, const std::vector<int>& value)
{
  for (size_t i = 0; i < value.size(); i++)
  {
    s << value[i];
    if (i < value.size() - 1)
    {
      s << " ";
    }
  }
  return s;
}

std::ostringstream& operator<<(std::ostringstream& s, const etl::ivector<int>& value)
{
  for (size_t i = 0; i < value.size(); i++)
  {
    s << value[i];
    if (i < value.size() - 1)
    {
      s << " ";
    }
  }
  return s;
}

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& s, const etl::ivector<int>& value)
{
  for (size_t i = 0; i < value.size(); i++)
  {
    s << value[i];
    if (i < value.size() - 1)
    {
      s << " ";
    }
  }
  return s;
}

std::ostringstream& operator<<(std::ostringstream& s, const etl::ivector<MoveInt>& value)
{
  for (size_t i = 0; i < value.size(); i++)
  {
    s << value[i].get();
    if (i < value.size() - 1)
    {
      s << " ";
    }
  }
  return s;
}

template<class T>
std::ostringstream& operator<<(std::ostringstream& s, const etl::ranges::view_interface<T>& v)
{
  for (size_t i = 0; i < v.size(); i++)
  {
    s << v[i];
    if (i < v.size() - 1)
    {
      s << " ";
    }
  }
  return s;
}

template<class T>
std::ostringstream& operator<<(std::ostringstream& s, const etl::ranges::range_iterator<T>& v)
{
  auto value{v.get()};
  s << value;
  return s;
}
}

namespace
{
  SUITE(test_ranges)
  {
    //*************************************************************************
    // Iterators.
    //*************************************************************************
    TEST(test_ranges_begin)
    {
      etl::vector<int, 10> v_in{ 3, 2, 1 };

      auto it = etl::ranges::begin(v_in);

      CHECK_EQUAL(*it, 3);
    }

    TEST(test_ranges_end)
    {
      etl::vector<int, 10> v_in{ 3, 2, 1 };

      auto it = etl::ranges::end(v_in);

      CHECK_EQUAL(it, ETL_OR_STD::end(v_in));
    }

    TEST(test_ranges_cbegin)
    {
      etl::vector<int, 10> v_in{ 3, 2, 1 };

      auto it = etl::ranges::cbegin(v_in);

      CHECK_EQUAL(*it, 3);
    }

    TEST(test_ranges_cend)
    {
      etl::vector<int, 10> v_in{ 3, 2, 1 };

      auto it = etl::ranges::cend(v_in);

      CHECK_EQUAL(it, ETL_OR_STD::cend(v_in));
    }

    TEST(test_ranges_rbegin)
    {
      etl::vector<int, 10> v_in{ 3, 2, 1 };

      auto it = etl::ranges::rbegin(v_in);

      CHECK_EQUAL(*it, 1);
    }

    TEST(test_ranges_rend)
    {
      etl::vector<int, 10> v_in{ 3, 2, 1 };

      auto it = etl::ranges::rend(v_in);

      CHECK_EQUAL(&(*it), &(*ETL_OR_STD::rend(v_in)));
    }

    TEST(test_ranges_crbegin)
    {
      etl::vector<int, 10> v_in{ 3, 2, 1 };

      auto it = etl::ranges::crbegin(v_in);

      CHECK_EQUAL(*it, 1);
    }

    TEST(test_ranges_crend)
    {
      etl::vector<int, 10> v_in{ 3, 2, 1 };

      auto it = etl::ranges::crend(v_in);

      CHECK_EQUAL(&(*it), &(*ETL_OR_STD::crend(v_in)));
    }

    TEST(test_ranges_size)
    {
      etl::vector<int, 10> v_in{ 11, 3, 2, 1 };

      CHECK_EQUAL(etl::ranges::size(v_in), 4);

      using size_type = decltype(etl::ranges::size(v_in));
      static_assert(etl::is_signed<size_type>::value == false, "Result of size must be unsigned");
    }

    TEST(test_ranges_ssize)
    {
      etl::vector<int, 10> v_in{ 11, 3, 2, 1 };

      CHECK_EQUAL(etl::ranges::ssize(v_in), 4);

      using signed_type = decltype(etl::ranges::ssize(v_in));
      using unsigned_type = decltype(etl::ranges::size(v_in));
      static_assert(etl::is_signed<signed_type>::value, "Result of ssize must be signed");
      static_assert(sizeof(signed_type) >= sizeof(unsigned_type), "Signed size type needs to be as wide as unsigned size type");
    }

    TEST(test_ranges_empty)
    {
      etl::vector<int, 10> v_in0{ 11, 3, 2, 1 };
      etl::vector<int, 10> v_in1{ };

      CHECK_EQUAL(etl::ranges::empty(v_in0), false);
      CHECK_EQUAL(etl::ranges::empty(v_in1), true);
    }

    TEST(test_ranges_data)
    {
      etl::vector<int, 10> v_in{ 11, 3, 2, 1 };

      CHECK_EQUAL(*etl::ranges::data(v_in), 11);
    }

    TEST(test_ranges_cdata)
    {
      etl::vector<int, 10> v_in{ 11, 3, 2, 1 };

      CHECK_EQUAL(*etl::ranges::cdata(v_in), 11);
    }

    //*************************************************************************
    // Range primitives.
    //*************************************************************************
    TEST(test_ranges_iterator_t)
    {
      using range_type = etl::vector<int, 10>;

      static_assert(etl::is_same<etl::ranges::iterator_t<range_type>, int*>::value, "Bad iterator type from etl::ranges::iterator_t");
    }

    TEST(test_ranges_const_iterator_t)
    {
      using range_type = etl::vector<int, 10>;

      static_assert(etl::is_same<etl::ranges::const_iterator_t<range_type>, const int*>::value, "Bad iterator type from etl::ranges::const_iterator_t");
    }

    TEST(test_ranges_sentinel_t)
    {
      using range_type = etl::vector<int, 10>;

      static_assert(etl::is_same<etl::ranges::sentinel_t<range_type>, int*>::value, "Bad sentinel type from etl::ranges::sentinel_t");
    }

    TEST(test_ranges_const_sentinel_t)
    {
      using range_type = etl::vector<int, 10>;

      static_assert(etl::is_same<etl::ranges::const_sentinel_t<range_type>, const int*>::value, "Bad sentinel type from etl::ranges::const_sentinel_t");
    }

    TEST(test_ranges_range_size_t)
    {
      using range_type0 = int[10];
      using range_type1 = etl::vector<int, 10>;
      using range_type2 = etl::ranges::empty_view<int>;

      static_assert(etl::is_same<etl::ranges::range_size_t<range_type0>, size_t>::value, "Bad size type from etl::ranges::range_size_t");
      static_assert(etl::is_same<etl::ranges::range_size_t<range_type1>, size_t>::value, "Bad size type from etl::ranges::range_size_t");
      static_assert(etl::is_same<etl::ranges::range_size_t<range_type2>, size_t>::value, "Bad size type from etl::ranges::range_size_t");
    }

    TEST(test_ranges_range_difference_t)
    {
      using range_type0 = int[10];
      using range_type1 = etl::vector<int, 10>;
      using range_type2 = etl::ranges::empty_view<int>;

      static_assert(etl::is_same<etl::ranges::range_difference_t<range_type0>, ptrdiff_t>::value, "Bad size type from etl::ranges::range_difference_t");
      static_assert(etl::is_same<etl::ranges::range_difference_t<range_type1>, ptrdiff_t>::value, "Bad size type from etl::ranges::range_difference_t");
      static_assert(etl::is_same<etl::ranges::range_difference_t<range_type2>, ptrdiff_t>::value, "Bad size type from etl::ranges::range_difference_t");
    }

    TEST(test_ranges_range_value_t)
    {
      using range_type0 = int[10];
      using range_type1 = etl::vector<int, 10>;
      using range_type2 = etl::ranges::empty_view<int>;

      static_assert(etl::is_same<etl::ranges::range_value_t<range_type0>, int>::value, "Bad size type from etl::ranges::range_value_t");
      static_assert(etl::is_same<etl::ranges::range_value_t<range_type1>, int>::value, "Bad size type from etl::ranges::range_value_t");
      static_assert(etl::is_same<etl::ranges::range_value_t<range_type2>, int>::value, "Bad size type from etl::ranges::range_value_t");
    }


    TEST(test_ranges_subrange)
    {
      etl::vector<int, 10> v {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
      auto sr = etl::ranges::subrange{v.begin(), v.end()};

      CHECK_EQUAL(sr.begin(), v.begin());
      CHECK_EQUAL(sr.end(), v.end());
      CHECK_EQUAL(sr.empty(), false);
      CHECK_EQUAL(sr.size(), 10);

      sr.advance(1);
      CHECK_EQUAL(sr.size(), 9);
      CHECK_EQUAL(sr[0], 1);

      sr.advance(2);
      CHECK_EQUAL(sr.size(), 7);
      CHECK_EQUAL(sr[0], 3);

      CHECK_EQUAL(sr.next().size(), 6);
      CHECK_EQUAL(sr.next()[0], 4);
      CHECK_EQUAL(sr.next(1).size(), 6);
      CHECK_EQUAL(sr.next(1)[0], 4);
      CHECK_EQUAL(sr.next(2).size(), 5);
      CHECK_EQUAL(sr.next(2)[0], 5);

      CHECK_EQUAL(sr.prev().size(), 8);
      CHECK_EQUAL(sr.prev()[0], 2);
      CHECK_EQUAL(sr.prev(1).size(), 8);
      CHECK_EQUAL(sr.prev(1)[0], 2);
      CHECK_EQUAL(sr.prev(2).size(), 9);
      CHECK_EQUAL(sr.prev(2)[0], 1);
    }

    //*************************************************************************
    // Range factories.
    //*************************************************************************
    TEST(test_ranges_empty_view)
    {
      auto ev = etl::ranges::empty_view<int>{};

      CHECK_EQUAL(ev.begin(), nullptr);
      CHECK_EQUAL(ev.end(), nullptr);
      CHECK_EQUAL(ev.data(), nullptr);
      CHECK_EQUAL(ev.size(), 0);
      CHECK_EQUAL(ev.empty(), true);
    }

    TEST(test_ranges_views_empty)
    {
      auto e = etl::ranges::views::empty<int>;

      CHECK_EQUAL(e.begin(), nullptr);
      CHECK_EQUAL(e.end(), nullptr);
      CHECK_EQUAL(e.data(), nullptr);
      CHECK_EQUAL(e.size(), 0);
      CHECK_EQUAL(e.empty(), true);
    }

    TEST(test_ranges_single_view)
    {
      auto s0 = etl::ranges::single_view<int>(12);
      CHECK_EQUAL(s0.size(), 1);

      auto s = etl::ranges::single_view(23);

      CHECK_EQUAL(*s.begin(), 23);
      CHECK_EQUAL(s.end(), s.begin() + 1);
      CHECK_EQUAL(*s.data(), 23);
      CHECK_EQUAL(s.size(), 1);
      CHECK_EQUAL(s.empty(), false);

      *s.begin() = 45;
      CHECK_EQUAL(*s.data(), 45);
      CHECK_EQUAL(*s.begin(), 45);
    }

    TEST(test_ranges_views_single)
    {
      auto s = etl::ranges::views::single(23);

      CHECK_EQUAL(*s.begin(), 23);
      CHECK_EQUAL(s.end(), s.begin() + 1);
      CHECK_EQUAL(*s.data(), 23);
      CHECK_EQUAL(s.size(), 1);
      CHECK_EQUAL(s.empty(), false);

      *s.begin() = 45;
      CHECK_EQUAL(*s.data(), 45);
      CHECK_EQUAL(*s.begin(), 45);
    }

    TEST(test_ranges_iota_view)
    {
      auto iv = etl::ranges::iota_view(1, 7);

      int compare = 1;
      for (auto i: iv)
      {
        CHECK_EQUAL(i, compare);
        ++compare;
      }

      CHECK_EQUAL(*iv.begin(), 1);
      CHECK_EQUAL(iv.end(), iv.begin() + 6);
      CHECK_EQUAL(iv.size(), 6);
      CHECK_EQUAL(iv.empty(), false);

      CHECK_EQUAL(iv[4], 5);

      auto iv2 = etl::ranges::iota_view(3);
      CHECK_EQUAL(iv2[0], 3);
      CHECK_EQUAL(iv2[1], 4);
      CHECK_EQUAL(iv2[2], 5);
      CHECK_EQUAL(iv2[3], 6);
      CHECK_EQUAL(iv2[4], 7);

      auto iv3 = etl::ranges::iota_view<int>();
      CHECK_EQUAL(iv3.size(), 0);
      CHECK_EQUAL(iv3.end(), iv3.begin());
      CHECK_EQUAL(iv3.empty(), true);
    }

    TEST(test_ranges_views_iota)
    {
      auto iv = etl::ranges::views::iota(1, 7);

      CHECK_EQUAL(*iv.begin(), 1);
      CHECK_EQUAL(iv.end(), iv.begin() + 6);
      CHECK_EQUAL(iv.size(), 6);
      CHECK_EQUAL(iv.empty(), false);

      CHECK_EQUAL(iv[4], 5);
    }

    TEST(test_ranges_iota_view_pipe_take)
    {
      auto iv = etl::ranges::iota_view(3) | etl::views::take(5);

      auto i = iv.begin();
      auto j = i;
      CHECK_EQUAL(*i, 3);
      i++;
      CHECK_EQUAL(*i, 4);
      ++i;
      CHECK_EQUAL(*i, 5);

      CHECK_EQUAL(i - j, 2);

      CHECK_EQUAL(iv.size(), 5);
      CHECK_EQUAL(iv.front(), 3);
      CHECK_EQUAL(iv.back(), 7);
      CHECK_EQUAL(iv[0], 3);
      CHECK_EQUAL(iv[1], 4);
      CHECK_EQUAL(iv[2], 5);
      CHECK_EQUAL(iv[3], 6);
      CHECK_EQUAL(iv[4], 7);
    }

    TEST(test_ranges_repeat_view)
    {
      // bounded
      auto iv = etl::ranges::repeat_view(1, 7);

      for (auto i: iv)
      {
        CHECK_EQUAL(i, 1);
      }

      CHECK_EQUAL(*iv.begin(), 1);
      CHECK_EQUAL(iv.end(), iv.begin() + 7);
      CHECK_EQUAL(iv.size(), 7);
      CHECK_EQUAL(iv.empty(), false);

      CHECK_EQUAL(iv[4], 1);

      // unbounded
      auto iv2 = etl::ranges::repeat_view(3);
      CHECK_EQUAL(iv2[0], 3);
      CHECK_EQUAL(iv2[1], 3);
      CHECK_EQUAL(iv2[2], 3);
      CHECK_EQUAL(iv2[3], 3);
      CHECK_EQUAL(iv2[4], 3);

      auto iv3 = etl::ranges::repeat_view<int>();
      CHECK_EQUAL(iv3.size(), 0);
      CHECK_EQUAL(iv3.end(), iv3.begin());
      CHECK_EQUAL(iv3.empty(), true);
    }

    TEST(test_ranges_views_repeat)
    {
      auto iv = etl::ranges::views::repeat(1, 7);

      CHECK_EQUAL(*iv.begin(), 1);
      CHECK_EQUAL(iv.end(), iv.begin() + 7);
      CHECK_EQUAL(iv.size(), 7);
      CHECK_EQUAL(iv.empty(), false);

      CHECK_EQUAL(iv[4], 1);
    }

    TEST(test_ranges_repeat_view_pipe_take)
    {
      auto iv = etl::ranges::repeat_view(3) | etl::views::take(5);

      auto i = iv.begin();
      CHECK_EQUAL(*i, 3);
      i++;
      CHECK_EQUAL(*i, 3);
      ++i;
      CHECK_EQUAL(*i, 3);

      CHECK_EQUAL(iv.size(), 5);
      CHECK_EQUAL(iv.front(), 3);
      CHECK_EQUAL(iv.back(), 3);
      CHECK_EQUAL(iv[0], 3);
      CHECK_EQUAL(iv[1], 3);
      CHECK_EQUAL(iv[2], 3);
      CHECK_EQUAL(iv[3], 3);
      CHECK_EQUAL(iv[4], 3);
    }

    TEST(test_ranges_repeat_view_pipe_take_bounded)
    {
      auto iv = etl::ranges::repeat_view(3, 30) | etl::views::take(5);

      auto i = iv.begin();
      CHECK_EQUAL(*i, 3);
      i++;
      CHECK_EQUAL(*i, 3);
      ++i;
      CHECK_EQUAL(*i, 3);

      CHECK_EQUAL(iv.size(), 5);
      CHECK_EQUAL(iv.front(), 3);
      CHECK_EQUAL(iv.back(), 3);
      CHECK_EQUAL(iv[0], 3);
      CHECK_EQUAL(iv[1], 3);
      CHECK_EQUAL(iv[2], 3);
      CHECK_EQUAL(iv[3], 3);
      CHECK_EQUAL(iv[4], 3);
    }

    TEST(test_ranges_repeat_view_pipe_take_bounded_limited)
    {
      auto iv = etl::ranges::repeat_view(3, 4) | etl::views::take(5);

      auto i = iv.begin();
      CHECK_EQUAL(*i, 3);
      i++;
      CHECK_EQUAL(*i, 3);
      ++i;
      CHECK_EQUAL(*i, 3);

      CHECK_EQUAL(iv.size(), 4);
      CHECK_EQUAL(iv.front(), 3);
      CHECK_EQUAL(iv.back(), 3);
      CHECK_EQUAL(iv[0], 3);
      CHECK_EQUAL(iv[1], 3);
      CHECK_EQUAL(iv[2], 3);
      CHECK_EQUAL(iv[3], 3);
    }

    //*************************************************************************
    // Range adaptors
    //*************************************************************************
    TEST(test_ranges_iterate_c_array)
    {
      auto even = [](int i) -> bool { return 0 == i % 2; };

      int v_in[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 2, 4, 6, 8 };

      for (int i : etl::views::filter(v_in, even))
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
    }

    TEST(test_ranges_iterate_plain)
    {
      auto even = [](int i) -> bool { return 0 == i % 2; };

      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 2, 4, 6, 8 };

      for (int i : etl::views::filter(v_in, even))
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
    }

    TEST(test_ranges_iterate_pipe)
    {
      auto even = [](int i) -> bool { return 0 == i % 2; };

      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 2, 4, 6, 8 };

      for (int i : v_in | etl::views::filter(even))
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
    }

    TEST(test_ranges_drop_functional)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 2, 3, 4, 5, 6, 7, 8, 9 };

      auto rv = etl::views::drop(v_in, 2);

      CHECK_EQUAL(etl::views::all(v_out_expected), rv);
      CHECK_EQUAL(rv.base().base(), v_in);
      CHECK_EQUAL(*rv.begin(), 2);
      CHECK_EQUAL(rv.size(), 8);

      CHECK_EQUAL(*rv.cbegin(), 2);
      CHECK_EQUAL(rv.empty(), false);
      CHECK_EQUAL(rv, true);
      CHECK_EQUAL(rv.front(), 2);
      CHECK_EQUAL(rv.back(), 9);
      CHECK_EQUAL(rv[7], 9);

      rv[1] = 33;
      CHECK_EQUAL(rv[1], 33);
      CHECK_EQUAL(v_in[3], 33);

      v_in[2] = 44;
      CHECK_EQUAL(rv[0], 44);
      CHECK_EQUAL(v_in[2], 44);
    }

    TEST(test_ranges_iterate_pipe_drop)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 2, 3, 4, 5, 6, 7, 8, 9 };

      auto rv = v_in | etl::views::drop(2);
      for (int i : rv)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
      CHECK_EQUAL(rv.base().base(), v_in);
      CHECK_EQUAL(*rv.begin(), 2);
      CHECK_EQUAL(rv.size(), 8);

      CHECK_EQUAL(*rv.cbegin(), 2);
      CHECK_EQUAL(rv.empty(), false);
      CHECK_EQUAL(rv, true);
      CHECK_EQUAL(rv.front(), 2);
      CHECK_EQUAL(rv.back(), 9);
      CHECK_EQUAL(rv[7], 9);

      rv[1] = 33;
      CHECK_EQUAL(rv[1], 33);
      CHECK_EQUAL(v_in[3], 33);

      v_in[2] = 44;
      CHECK_EQUAL(rv[0], 44);
      CHECK_EQUAL(v_in[2], 44);
    }

    TEST(test_ranges_iterate_pipe_drop_out_of_bounds)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{};

      auto rv = v_in | etl::views::drop(12);
      for (int i : rv)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
      CHECK_EQUAL(rv, 0);
    }

    TEST(test_ranges_iterate_pipe_twice)
    {
      auto even = [](int i) -> bool { return 0 == i % 2; };

      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 2, 4, 6, 8 };

      for (int i : v_in | etl::views::filter(even) | etl::views::drop(1))
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
    }

    TEST(test_ranges_iterate_pipe_drop_while)
    {
      auto below_three = [](int i) -> bool { return i < 3; };

      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 3, 4, 5, 6, 7, 8, 9 };

      auto rv = v_in | etl::views::drop_while(below_three);
      for (int i : rv)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
      CHECK_EQUAL(rv.base().base(), v_in);
      CHECK_EQUAL(*rv.begin(), 3);
      CHECK_EQUAL(rv.size(), 7);

      CHECK_EQUAL(*rv.cbegin(), 3);
      CHECK_EQUAL(rv.empty(), false);
      CHECK_EQUAL(rv, true);
      CHECK_EQUAL(rv.front(), 3);
      CHECK_EQUAL(rv.back(), 9);
      CHECK_EQUAL(rv[6], 9);
    }

    TEST(test_ranges_take_functional)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out_expected{ 0, 1, 2, 3 };

      auto rv = etl::views::take(v_in, 4);

      CHECK_EQUAL(etl::views::all(v_out_expected), rv);
      CHECK_EQUAL(rv.base().base(), v_in);
      CHECK_EQUAL(*rv.begin(), 0);
      CHECK_EQUAL(rv.size(), 4);

      CHECK_EQUAL(*rv.cbegin(), 0);
      CHECK_EQUAL(rv.empty(), false);
      CHECK_EQUAL(rv, true);
      CHECK_EQUAL(rv.front(), 0);
      CHECK_EQUAL(rv.back(), 3);
      CHECK_EQUAL(rv[2], 2);

      rv[2] = 33;
      CHECK_EQUAL(rv[2], 33);
      CHECK_EQUAL(v_in[2], 33);

      v_in[3] = 44;
      CHECK_EQUAL(rv[3], 44);
      CHECK_EQUAL(v_in[3], 44);
    }

    TEST(test_ranges_iterate_pipe_take)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 1, 2, 3 };

      auto rv = v_in | etl::views::take(4);
      for (int i : rv)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
      CHECK_EQUAL(rv.base().base(), v_in);
      CHECK_EQUAL(*rv.begin(), 0);
      CHECK_EQUAL(rv.size(), 4);

      CHECK_EQUAL(*rv.cbegin(), 0);
      CHECK_EQUAL(rv.empty(), false);
      CHECK_EQUAL(rv, true);
      CHECK_EQUAL(rv.front(), 0);
      CHECK_EQUAL(rv.back(), 3);
      CHECK_EQUAL(rv[2], 2);

      rv[2] = 33;
      CHECK_EQUAL(rv[2], 33);
      CHECK_EQUAL(v_in[2], 33);

      v_in[3] = 44;
      CHECK_EQUAL(rv[3], 44);
      CHECK_EQUAL(v_in[3], 44);
    }

    TEST(test_ranges_iterate_pipe_take_while)
    {
      auto below_three = [](int i) -> bool { return i < 3; };

      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 1, 2 };

      auto rv = v_in | etl::views::take_while(below_three);
      for (int i : rv)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
      CHECK_EQUAL(rv.base().base(), v_in);
      CHECK_EQUAL(*rv.begin(), 0);
      CHECK_EQUAL(rv.size(), 3);

      CHECK_EQUAL(*rv.cbegin(), 0);
      CHECK_EQUAL(rv.empty(), false);
      CHECK_EQUAL(rv, true);
      CHECK_EQUAL(rv.front(), 0);
      CHECK_EQUAL(rv.back(), 2);
      CHECK_EQUAL(rv[2], 2);
    }

    TEST(test_ranges_reverse_view_functional)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 4, 3, 2, 1, 0 };

      auto rv = etl::ranges::reverse_view(v_in);
      for (int i : rv)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
      CHECK_EQUAL(rv.base().base(), v_in);
      CHECK_EQUAL(*rv.begin(), 4);
      CHECK_EQUAL(rv.size(), 5);

      CHECK_EQUAL(*rv.cbegin(), 4);
      CHECK_EQUAL(rv.empty(), false);
      CHECK_EQUAL(rv, true);
      CHECK_EQUAL(rv.front(), 4);
      CHECK_EQUAL(rv.back(), 0);
      CHECK_EQUAL(rv[2], 2);

      rv[0] = 22;
      CHECK_EQUAL(rv[0], 22);
      CHECK_EQUAL(v_in[4], 22);

      v_in[0] = 11;
      CHECK_EQUAL(rv.back(), 11);
      CHECK_EQUAL(rv[4], 11);
    }

    TEST(test_ranges_iterate_pipe_reverse)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 4, 3, 2, 1, 0 };

      auto rv = v_in | etl::views::reverse;
      for (int i : rv)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
      CHECK_EQUAL(rv.base().base(), v_in);
      CHECK_EQUAL(*rv.begin(), 4);
      CHECK_EQUAL(rv.size(), 5);
      CHECK_EQUAL(rv.front(), 4);
      CHECK_EQUAL(rv.back(), 0);
    }

    TEST(test_ranges_iterate_pipe_reverse_reverse)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 1, 2, 3, 4 };

      auto rv = v_in | etl::views::reverse | etl::views::reverse;
      for (int i : rv)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
      CHECK_EQUAL(rv.base(), v_in);
      CHECK_EQUAL(*rv.begin(), 0);
      CHECK_EQUAL(rv.size(), 5);
      CHECK_EQUAL(rv.front(), 0);
      CHECK_EQUAL(rv.back(), 4);
    }

    TEST(test_ranges_iterate_pipe_all)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

      for (int i : v_in | etl::views::all)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
    }

    TEST(test_ranges_iterate_pipe_ref)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

      auto r = v_in | etl::views::ref();
      for (int i : r)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);

      v_in[9] = 99;
      CHECK_EQUAL(r[9], 99);

      CHECK_EQUAL(r.base(), v_in);
      CHECK_EQUAL(*r.begin(), 0);
      CHECK_EQUAL(r.end(), r.begin() + 10);
      CHECK_EQUAL(r.empty(), false);
      CHECK_EQUAL(r.size(), 10);
      CHECK_EQUAL(r.data(), v_in.data());
    }

    TEST(test_ranges_iterate_pipe_owning)
    {
      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

      auto r = v_in | etl::views::owning();
      for (int i : r)
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);

      CHECK_EQUAL(0, v_in.size());

      etl::ranges::owning_view<etl::vector<int, 10>> ov2;

      CHECK_NOT_EQUAL(r.base(), v_in);
      CHECK_EQUAL(*r.begin(), 0);
      CHECK_EQUAL(r.end(), r.begin() + 10);
      CHECK_EQUAL(r.empty(), false);
      CHECK_EQUAL(r.size(), 10);
      CHECK_NOT_EQUAL(r.data(), v_in.data());

      //ov2 = r; // expected: compile error!
      ov2 = etl::move(r);
    }

    TEST(test_ranges_iterate_pipe_to)
    {
      auto even = [](int i) -> bool { return 0 == i % 2; };

      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out_expected{ 2, 4, 6, 8 };

      auto v_out = v_in | etl::views::filter(even) | etl::views::drop(1) | etl::ranges::to<etl::vector<int, 10>>();

      CHECK_EQUAL(v_out_expected, v_out);
    }

    TEST(test_ranges_iterate_pipe_as_rvalue)
    {
      etl::vector<MoveInt, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<MoveInt, 10> v_out;

      etl::vector<MoveInt, 10> v_out_expected_0{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
      etl::vector<MoveInt, 10> v_out_expected_1{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

      for (auto&& i : v_in | etl::views::as_rvalue)
      {
        v_out.emplace_back(etl::move(i));
      }

      CHECK_EQUAL(v_out_expected_0, v_in);
      CHECK_EQUAL(v_out_expected_1, v_out);
    }

    TEST(test_ranges_iterate_pipe_as_rvalue_functional)
    {
      etl::vector<MoveInt, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<MoveInt, 10> v_out;

      etl::vector<MoveInt, 10> v_out_expected_0{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
      etl::vector<MoveInt, 10> v_out_expected_1{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

      v_out = etl::views::as_rvalue(v_in) | etl::ranges::to<etl::vector<MoveInt, 10>>();

      CHECK_EQUAL(v_out_expected_0, v_in);
      CHECK_EQUAL(v_out_expected_1, v_out);
    }

    TEST(test_ranges_iterate_transform)
    {
      auto square = [](int i) -> int { return i * i; };

      etl::vector<int, 10> v_in{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
      etl::vector<int, 10> v_out;
      etl::vector<int, 10> v_out_expected{ 0, 1, 4, 9, 16, 25, 36, 49, 64, 81 };

      for (int i : v_in | etl::views::transform(square))
      {
        v_out.push_back(i);
      }

      CHECK_EQUAL(v_out_expected, v_out);
    }

    TEST(test_ranges_join_functional)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};

      auto result = etl::views::join(v);

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

      auto expected_view = etl::views::all(v_expected);
      CHECK_EQUAL(result, expected_view);
    }

    TEST(test_ranges_join_pipe)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};

      auto result = v | etl::views::join;

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

      auto expected_view = etl::views::all(v_expected);
      CHECK_EQUAL(result, expected_view);
    }

    TEST(test_ranges_join_degenerated_1)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 6> v{{1}, {1, 2, 3}, {1}, {7, 8, 9}, {1}};

      auto result = v | etl::views::join;

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 1, 2, 3, 1, 7, 8, 9, 1};
      auto expected_view = etl::views::all(v_expected);

      CHECK_EQUAL(9, result.size());
      CHECK_EQUAL(expected_view, result);
    }

    TEST(test_ranges_join_degenerated_2)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 7> v{{}, {1, 2, 3}, {}, {7, 8, 9}, {}};

      auto rv = v | etl::views::join;

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 2, 3, 7, 8, 9};
      auto expected_view = etl::views::all(v_expected);

      CHECK_EQUAL(6, rv.size());
      CHECK_EQUAL(expected_view, rv);
    }

    TEST(test_ranges_join_iterate)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};

      using result_type = etl::vector<int, 30>;
      auto result = v | etl::views::join | etl::ranges::to<result_type>();

      result_type result_v;
      for (auto i: result)
      {
        result_v.emplace_back(i);
      }
      result_type v_expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
      CHECK_EQUAL(result_v, v_expected);
    }

    TEST(test_ranges_join_view)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};

      using result_type = etl::vector<int, 30>;
      auto result = v | etl::views::join | etl::ranges::to<result_type>();

      result_type v_expected{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
      CHECK_EQUAL(result, v_expected);
    }

    TEST(test_ranges_join_with_functional)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
      range_type pattern{111, 222};

      auto result = etl::views::join_with(v, pattern);

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 2, 3, 111, 222, 4, 5, 6, 111, 222, 7, 8, 9, 111, 222, 10, 11, 12};

      auto expected_view = etl::views::all(v_expected);
      CHECK_EQUAL(result, expected_view);
    }

    TEST(test_ranges_join_with_pipe)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
      range_type pattern{111, 222};

      auto result = v | etl::views::join_with(pattern);

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 2, 3, 111, 222, 4, 5, 6, 111, 222, 7, 8, 9, 111, 222, 10, 11, 12};

      auto expected_view = etl::views::all(v_expected);
      CHECK_EQUAL(result, expected_view);
    }

    TEST(test_ranges_join_with_degenerated_1)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 6> v{{1}, {1, 2, 3}, {1}, {7, 8, 9}, {1}};
      range_type pattern{111, 222};

      auto result = v | etl::views::join_with(pattern);

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 111, 222, 1, 2, 3, 111, 222, 1, 111, 222, 7, 8, 9, 111, 222, 1};
      auto expected_view = etl::views::all(v_expected);

      CHECK_EQUAL(17, result.size());
      CHECK_EQUAL(expected_view, result);
    }

    TEST(test_ranges_join_with_degenerated_2)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 7> v{{}, {1, 2, 3}, {}, {7, 8, 9}, {}};
      range_type pattern{111, 222};

      auto rv = v | etl::views::join_with(pattern);

      using result_type = etl::vector<int, 30>;
      result_type v_expected{111, 222, 1, 2, 3, 111, 222, 111, 222, 7, 8, 9, 111, 222};
      auto expected_view = etl::views::all(v_expected);

      CHECK_EQUAL(14, rv.size());
      CHECK_EQUAL(expected_view, rv);
    }

    TEST(test_ranges_join_with_iterate)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
      range_type pattern{111, 222};

      using result_type = etl::vector<int, 30>;
      auto result = v | etl::views::join_with(pattern) | etl::ranges::to<result_type>();

      result_type result_v;
      for (auto i: result)
      {
        result_v.emplace_back(i);
      }
      result_type v_expected{1, 2, 3, 111, 222, 4, 5, 6, 111, 222, 7, 8, 9, 111, 222, 10, 11, 12};
      CHECK_EQUAL(result_v, v_expected);
    }

    TEST(test_ranges_join_with_view)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
      range_type pattern{111, 222};

      using result_type = etl::vector<int, 30>;
      auto result = v | etl::views::join_with(pattern) | etl::ranges::to<result_type>();

      result_type v_expected{1, 2, 3, 111, 222, 4, 5, 6, 111, 222, 7, 8, 9, 111, 222, 10, 11, 12};
      CHECK_EQUAL(result, v_expected);
    }

    TEST(test_ranges_join_with_functional_single)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
      int pattern{111};

      auto result = etl::views::join_with(v, pattern);

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 2, 3, 111, 4, 5, 6, 111, 7, 8, 9, 111, 10, 11, 12};

      auto expected_view = etl::views::all(v_expected);
      CHECK_EQUAL(result, expected_view);
    }

    TEST(test_ranges_join_with_functional_single_immediate)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};

      auto result = etl::views::join_with(v, 111);

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 2, 3, 111, 4, 5, 6, 111, 7, 8, 9, 111, 10, 11, 12};

      auto expected_view = etl::views::all(v_expected);
      CHECK_EQUAL(result, expected_view);
    }

    TEST(test_ranges_join_with_pipe_single)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
      int pattern{111};

      auto result = v | etl::views::join_with(pattern);

      using result_type = etl::vector<int, 30>;
      result_type v_expected{1, 2, 3, 111, 4, 5, 6, 111, 7, 8, 9, 111, 10, 11, 12};

      auto expected_view = etl::views::all(v_expected);
      CHECK_EQUAL(result, expected_view);
    }

    TEST(test_ranges_join_with_iterate_single)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
      int pattern{111};

      using result_type = etl::vector<int, 30>;
      auto result = v | etl::views::join_with(pattern) | etl::ranges::to<result_type>();

      result_type result_v;
      for (auto i: result)
      {
        result_v.emplace_back(i);
      }
      result_type v_expected{1, 2, 3, 111, 4, 5, 6, 111, 7, 8, 9, 111, 10, 11, 12};
      CHECK_EQUAL(result_v, v_expected);
    }

    TEST(test_ranges_join_with_view_single)
    {
      using range_type = etl::vector<int, 3>;
      etl::vector<range_type, 4> v{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
      int pattern{111};

      using result_type = etl::vector<int, 30>;
      auto result = v | etl::views::join_with(pattern) | etl::ranges::to<result_type>();

      result_type v_expected{1, 2, 3, 111, 4, 5, 6, 111, 7, 8, 9, 111, 10, 11, 12};
      CHECK_EQUAL(result, v_expected);
    }

    TEST(test_counted)
    {
      {
        std::vector<int> vec{1, 2, 3, 4, 5};

        auto result = etl::views::counted(vec.begin(), 3);

        CHECK_EQUAL(result.size(), 3);
        CHECK_EQUAL(result[0], 1);
        CHECK_EQUAL(result[1], 2);
        CHECK_EQUAL(result[2], 3);
      }
      {
        std::list<int> list{1, 2, 3, 4, 5};

        auto result = etl::views::counted(list.begin(), 3);

        CHECK_EQUAL(result.size(), 3);
        CHECK_EQUAL(result[0], 1);
        CHECK_EQUAL(result[1], 2);
        CHECK_EQUAL(result[2], 3);
      }
    }

    TEST(test_ranges_concat_view_1)
    {
      std::vector<int> vec{1, 2, 3, 4};

      using result_type = std::vector<int>;
      auto cv = etl::views::concat(vec);
      result_type result = etl::views::concat(vec) | etl::ranges::to<result_type>();

      result_type expected_result{1, 2, 3, 4};

      CHECK_EQUAL(cv.size(), 4);
      CHECK_EQUAL(result.size(), 4);

      CHECK_EQUAL(result, expected_result);
    }

    TEST(test_ranges_concat_view_2)
    {
      std::vector<int> vec{1, 2, 3, 4};
      std::list<int> list{6, 7, 8, 9, 10};

      auto cv = etl::views::concat(vec, list);
      using result_type = std::vector<int>;
      result_type result = etl::views::concat(vec, list) | etl::ranges::to<result_type>();

      result_type expected_result{1, 2, 3, 4, 6, 7, 8, 9, 10};

      CHECK_EQUAL(cv.size(), 9);
      CHECK_EQUAL(result.size(), 9);

      CHECK_EQUAL(result, expected_result);
    }

    TEST(test_ranges_concat_view_2_same_type)
    {
      std::vector<int> vec0{1, 2, 3, 4};
      std::vector<int> vec1{6, 7, 8, 9, 10};

      auto cv = etl::views::concat(vec0, vec1);

      using result_type = std::vector<int>;
      result_type result = etl::views::concat(vec0, vec1) | etl::ranges::to<result_type>();

      std::vector<int> expected_result{1, 2, 3, 4, 6, 7, 8, 9, 10};

      CHECK_EQUAL(cv.size(), 9);
      CHECK_EQUAL(result.size(), 9);

      CHECK_EQUAL(result, expected_result);
    }

    TEST(test_ranges_concat_view_3)
    {
      std::vector<int> vec{1, 2, 3, 4};
      std::list<int> list{6, 7, 8, 9, 10};
      std::array<int, 3> arr{20, 21, 22};

      auto cv = etl::views::concat(vec, list, arr);

      using result_type = std::vector<int>;
      result_type result = etl::views::concat(vec, list, arr) | etl::ranges::to<result_type>();

      std::vector<int> expected_result{1, 2, 3, 4, 6, 7, 8, 9, 10, 20, 21, 22};

      CHECK_EQUAL(cv.size(), 12);
      CHECK_EQUAL(result.size(), 12);

      CHECK_EQUAL(result, expected_result);

      auto it = cv.begin();
      CHECK(it != cv.end());
      CHECK_EQUAL(*it, 1);

      ++it;
      CHECK_EQUAL(*it, 2);
      it++;
      CHECK_EQUAL(*it, 3);
      CHECK_EQUAL(it[0], 3);
      CHECK_EQUAL(it[1], 4);
      CHECK_EQUAL(it[2], 6);
      it += 3;
      CHECK_EQUAL(*it, 7);
      it -= 2;
      CHECK_EQUAL(*it, 4);
      --it;
      CHECK_EQUAL(*it, 3);
      it--;
      CHECK_EQUAL(*it, 2);
      CHECK(it != cv.end());
      it += 8;
      CHECK_EQUAL(*it, 20);
      it += 2;
      CHECK_EQUAL(*it, 22);
      CHECK(it != cv.end());
      ++it;
      CHECK(it == cv.end());
    }

    TEST(test_ranges_concat_view_3_same_type)
    {
      std::vector<int> vec0{1, 2, 3, 4};
      std::vector<int> vec1{6, 7, 8, 9, 10};
      std::vector<int> vec2{20, 21, 22};

      auto cv = etl::views::concat(vec0, vec1, vec2);

      using result_type = std::vector<int>;
      result_type result = etl::views::concat(vec0, vec1, vec2) | etl::ranges::to<result_type>();

      std::vector<int> expected_result{1, 2, 3, 4, 6, 7, 8, 9, 10, 20, 21, 22};

      CHECK_EQUAL(cv.size(), 12);
      CHECK_EQUAL(result.size(), 12);

      CHECK_EQUAL(result, expected_result);

      auto it = cv.begin();
      CHECK(it != cv.end());
      CHECK_EQUAL(*it, 1);

      ++it;
      CHECK_EQUAL(*it, 2);
      it++;
      CHECK_EQUAL(*it, 3);
      CHECK_EQUAL(it[0], 3);
      CHECK_EQUAL(it[1], 4);
      CHECK_EQUAL(it[2], 6);
      it += 3;
      CHECK_EQUAL(*it, 7);
      it -= 2;
      CHECK_EQUAL(*it, 4);
      --it;
      CHECK_EQUAL(*it, 3);
      it--;
      CHECK_EQUAL(*it, 2);
      CHECK(it != cv.end());
      it += 8;
      CHECK_EQUAL(*it, 20);
      it += 2;
      CHECK_EQUAL(*it, 22);
      CHECK(it != cv.end());
      ++it;
      CHECK(it == cv.end());
    }

    TEST(test_ranges_concat_view_3_same_range)
    {
      std::vector<int> vec0{1, 2, 3, 4, 5};

      auto cv = etl::views::concat(vec0, vec0, vec0);

      using result_type = std::vector<int>;
      result_type result = etl::views::concat(vec0, vec0, vec0) | etl::ranges::to<result_type>();

      std::vector<int> expected_result{1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5};

      CHECK_EQUAL(cv.size(), 15);
      CHECK_EQUAL(result.size(), 15);

      CHECK_EQUAL(result, expected_result);

      auto it = cv.begin();
      CHECK(it != cv.end());
      CHECK_EQUAL(*it, 1);

      ++it;
      CHECK_EQUAL(*it, 2);
      it++;
      CHECK_EQUAL(*it, 3);
      CHECK_EQUAL(it[0], 3);
      CHECK_EQUAL(it[1], 4);
      CHECK_EQUAL(it[2], 5);
      it += 3;
      CHECK_EQUAL(*it, 1);
      it -= 2;
      CHECK_EQUAL(*it, 4);
      --it;
      CHECK_EQUAL(*it, 3);
      it--;
      CHECK_EQUAL(*it, 2);
      CHECK(it != cv.end());
      it += 11;
      CHECK_EQUAL(*it, 3);
      it += 2;
      CHECK_EQUAL(*it, 5);
      CHECK(it != cv.end());
      ++it;
      CHECK(it == cv.end());
    }
  }
}

#endif
#endif