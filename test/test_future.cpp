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

#include "etl/future.h"

namespace
{
  //---------------------------------------------------------------------------
  // A simple struct to test non-primitive types in future/promise.
  //---------------------------------------------------------------------------
  struct Point
  {
    int x;
    int y;

    bool operator==(const Point& other) const
    {
      return x == other.x && y == other.y;
    }
  };

  SUITE(test_future)
  {
    //*************************************************************************
    // shared_state<T> tests
    //*************************************************************************

    TEST(test_shared_state_int_default_not_ready)
    {
      etl::shared_state<int> state;
      CHECK(!state.is_ready());
    }

    TEST(test_shared_state_int_set_value_becomes_ready)
    {
      etl::shared_state<int> state;
      state.set_value(42);
      CHECK(state.is_ready());
      CHECK_EQUAL(42, state.get());
    }

    TEST(test_shared_state_int_get_const)
    {
      etl::shared_state<int> state;
      state.set_value(77);
      const etl::shared_state<int>& cref = state;
      CHECK_EQUAL(77, cref.get());
    }

    TEST(test_shared_state_int_reset)
    {
      etl::shared_state<int> state;
      state.set_value(10);
      CHECK(state.is_ready());
      state.reset();
      CHECK(!state.is_ready());
    }

    TEST(test_shared_state_int_reset_when_not_ready)
    {
      etl::shared_state<int> state;
      // Should be safe to reset even when not ready.
      state.reset();
      CHECK(!state.is_ready());
    }

    TEST(test_shared_state_double)
    {
      etl::shared_state<double> state;
      state.set_value(3.14);
      CHECK(state.is_ready());
      CHECK_CLOSE(3.14, state.get(), 0.001);
    }

    TEST(test_shared_state_struct)
    {
      etl::shared_state<Point> state;
      Point                    p{10, 20};
      state.set_value(p);
      CHECK(state.is_ready());
      CHECK_EQUAL(10, state.get().x);
      CHECK_EQUAL(20, state.get().y);
    }

    //*************************************************************************
    // shared_state<void> tests
    //*************************************************************************

    TEST(test_shared_state_void_default_not_ready)
    {
      etl::shared_state<void> state;
      CHECK(!state.is_ready());
    }

    TEST(test_shared_state_void_set_value_becomes_ready)
    {
      etl::shared_state<void> state;
      state.set_value();
      CHECK(state.is_ready());
    }

    TEST(test_shared_state_void_reset)
    {
      etl::shared_state<void> state;
      state.set_value();
      CHECK(state.is_ready());
      state.reset();
      CHECK(!state.is_ready());
    }

    //*************************************************************************
    // future<T> tests
    //*************************************************************************

    TEST(test_future_int_default_invalid)
    {
      etl::future<int> f;
      CHECK(!f.valid());
      CHECK(!f.is_ready());
    }

    TEST(test_future_int_from_state_valid_not_ready)
    {
      etl::shared_state<int> state;
      etl::future<int>       f(state);
      CHECK(f.valid());
      CHECK(!f.is_ready());
    }

    TEST(test_future_int_becomes_ready_after_set)
    {
      etl::shared_state<int> state;
      etl::future<int>       f(state);
      state.set_value(55);
      CHECK(f.is_ready());
      CHECK_EQUAL(55, f.get());
    }

    TEST(test_future_int_const_get)
    {
      etl::shared_state<int> state;
      state.set_value(88);
      const etl::future<int> f(state);
      CHECK(f.is_ready());
      CHECK_EQUAL(88, f.get());
    }

    TEST(test_future_double)
    {
      etl::shared_state<double> state;
      etl::future<double>       f(state);
      state.set_value(2.718);
      CHECK(f.is_ready());
      CHECK_CLOSE(2.718, f.get(), 0.001);
    }

    TEST(test_future_struct)
    {
      etl::shared_state<Point> state;
      etl::future<Point>       f(state);
      state.set_value(Point{3, 4});
      CHECK(f.is_ready());
      CHECK_EQUAL(3, f.get().x);
      CHECK_EQUAL(4, f.get().y);
    }

    //*************************************************************************
    // future<void> tests
    //*************************************************************************

    TEST(test_future_void_default_invalid)
    {
      etl::future<void> f;
      CHECK(!f.valid());
      CHECK(!f.is_ready());
    }

    TEST(test_future_void_from_state)
    {
      etl::shared_state<void> state;
      etl::future<void>       f(state);
      CHECK(f.valid());
      CHECK(!f.is_ready());
      state.set_value();
      CHECK(f.is_ready());
    }

    //*************************************************************************
    // promise<T> tests
    //*************************************************************************

    TEST(test_promise_int_default_invalid)
    {
      etl::promise<int> p;
      CHECK(!p.valid());
    }

    TEST(test_promise_int_set_value_fulfils_future)
    {
      etl::shared_state<int> state;
      etl::promise<int>      p(state);
      etl::future<int>       f = p.get_future();
      CHECK(f.valid());
      CHECK(!f.is_ready());
      p.set_value(123);
      CHECK(f.is_ready());
      CHECK_EQUAL(123, f.get());
    }

    TEST(test_promise_double)
    {
      etl::shared_state<double> state;
      etl::promise<double>      p(state);
      etl::future<double>       f = p.get_future();
      p.set_value(1.41421);
      CHECK(f.is_ready());
      CHECK_CLOSE(1.41421, f.get(), 0.00001);
    }

    TEST(test_promise_struct)
    {
      etl::shared_state<Point> state;
      etl::promise<Point>      p(state);
      etl::future<Point>       f = p.get_future();
      p.set_value(Point{-5, 7});
      CHECK(f.is_ready());
      CHECK_EQUAL(-5, f.get().x);
      CHECK_EQUAL(7, f.get().y);
    }

    //*************************************************************************
    // promise<void> tests
    //*************************************************************************

    TEST(test_promise_void_default_invalid)
    {
      etl::promise<void> p;
      CHECK(!p.valid());
    }

    TEST(test_promise_void_set_value_fulfils_future)
    {
      etl::shared_state<void> state;
      etl::promise<void>      p(state);
      etl::future<void>       f = p.get_future();
      CHECK(f.valid());
      CHECK(!f.is_ready());
      p.set_value();
      CHECK(f.is_ready());
    }

    //*************************************************************************
    // Integration / edge-case tests
    //*************************************************************************

    TEST(test_multiple_futures_same_state)
    {
      // Two futures can observe the same shared_state.
      etl::shared_state<int> state;
      etl::future<int>       f1(state);
      etl::future<int>       f2(state);
      CHECK(!f1.is_ready());
      CHECK(!f2.is_ready());
      state.set_value(999);
      CHECK(f1.is_ready());
      CHECK(f2.is_ready());
      CHECK_EQUAL(999, f1.get());
      CHECK_EQUAL(999, f2.get());
    }

    TEST(test_shared_state_reuse_after_reset)
    {
      etl::shared_state<int> state;
      etl::promise<int>      p(state);
      etl::future<int>       f = p.get_future();
      p.set_value(1);
      CHECK_EQUAL(1, f.get());

      // Reset and reuse.
      state.reset();
      CHECK(!f.is_ready());

      // Set a different value.
      state.set_value(2);
      CHECK(f.is_ready());
      CHECK_EQUAL(2, f.get());
    }

    TEST(test_move_semantics_set_value)
    {
      etl::shared_state<int> state;
      int                    val = 50;
      state.set_value(etl::move(val));
      CHECK(state.is_ready());
      CHECK_EQUAL(50, state.get());
    }
  }
} // namespace
