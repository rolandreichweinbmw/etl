/******************************************************************************
The MIT License(MIT)

Embedded Template Library.
https://github.com/ETLCPP/etl
https://www.etlcpp.com

Copyright(c) 2026 John Wellbelove

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

#include "etl/platform.h"
#include "etl/mutex.h"
#include "etl/type_traits.h"

#if ETL_HAS_MUTEX

#include <thread>
#include <atomic>

namespace
{
  //***************************************************************************
  /// A mutex substitute that records the calls made to it.
  //***************************************************************************
  struct TestMutex
  {
    TestMutex()
      : lock_count(0)
      , unlock_count(0)
      , locked(false)
    {
    }

    void lock()
    {
      ++lock_count;
      locked = true;
    }

    void unlock()
    {
      ++unlock_count;
      locked = false;
    }

    int  lock_count;
    int  unlock_count;
    bool locked;
  };

  SUITE(test_mutex)
  {
    //*************************************************************************
    TEST(test_traits_has_mutex)
    {
      CHECK_TRUE(etl::traits::has_mutex);
    }

    //*************************************************************************
    TEST(test_lock_unlock)
    {
      etl::mutex mutex;

      mutex.lock();
      mutex.unlock();

      // Must be lockable again.
      mutex.lock();
      mutex.unlock();
    }

    //*************************************************************************
    TEST(test_try_lock_when_unlocked)
    {
      etl::mutex mutex;

      CHECK_TRUE(mutex.try_lock());

      mutex.unlock();
    }

    //*************************************************************************
    TEST(test_try_lock_when_locked_by_another_thread)
    {
      etl::mutex        mutex;
      std::atomic<bool> result(true);

      mutex.lock();

      std::thread other([&mutex, &result]()
        {
          result = mutex.try_lock();

          if (result)
          {
            mutex.unlock();
          }
        });

      other.join();

      CHECK_FALSE(result.load());

      mutex.unlock();
    }

    //*************************************************************************
    TEST(test_try_lock_succeeds_after_unlock)
    {
      etl::mutex        mutex;
      std::atomic<bool> result(false);

      mutex.lock();
      mutex.unlock();

      std::thread other([&mutex, &result]()
        {
          result = mutex.try_lock();

          if (result)
          {
            mutex.unlock();
          }
        });

      other.join();

      CHECK_TRUE(result.load());
    }

    //*************************************************************************
    TEST(test_mutual_exclusion)
    {
      const size_t Number_Of_Threads     = 4U;
      const size_t Increments_Per_Thread = 1000U;

      etl::mutex  mutex;
      size_t      count = 0U;
      std::thread threads[Number_Of_Threads];

      for (size_t i = 0U; i < Number_Of_Threads; ++i)
      {
        threads[i] = std::thread([&mutex, &count]()
          {
            for (size_t j = 0U; j < Increments_Per_Thread; ++j)
            {
              mutex.lock();
              ++count;
              mutex.unlock();
            }
          });
      }

      for (size_t i = 0U; i < Number_Of_Threads; ++i)
      {
        threads[i].join();
      }

      CHECK_EQUAL(Number_Of_Threads * Increments_Per_Thread, count);
    }

    //*************************************************************************
    TEST(test_mutual_exclusion_with_lock_guard)
    {
      const size_t Number_Of_Threads     = 4U;
      const size_t Increments_Per_Thread = 1000U;

      etl::mutex  mutex;
      size_t      count = 0U;
      std::thread threads[Number_Of_Threads];

      for (size_t i = 0U; i < Number_Of_Threads; ++i)
      {
        threads[i] = std::thread([&mutex, &count]()
          {
            for (size_t j = 0U; j < Increments_Per_Thread; ++j)
            {
              etl::lock_guard<etl::mutex> guard(mutex);
              ++count;
            }
          });
      }

      for (size_t i = 0U; i < Number_Of_Threads; ++i)
      {
        threads[i].join();
      }

      CHECK_EQUAL(Number_Of_Threads * Increments_Per_Thread, count);
    }

    //*************************************************************************
    TEST(test_lock_guard_locks_and_unlocks)
    {
      TestMutex mutex;

      {
        etl::lock_guard<TestMutex> guard(mutex);

        CHECK_TRUE(mutex.locked);
        CHECK_EQUAL(1, mutex.lock_count);
        CHECK_EQUAL(0, mutex.unlock_count);
      }

      CHECK_FALSE(mutex.locked);
      CHECK_EQUAL(1, mutex.lock_count);
      CHECK_EQUAL(1, mutex.unlock_count);
    }

    //*************************************************************************
    TEST(test_lock_guard_mutex_type)
    {
      CHECK_TRUE((etl::is_same<etl::mutex, etl::lock_guard<etl::mutex>::mutex_type>::value));
      CHECK_TRUE((etl::is_same<TestMutex,  etl::lock_guard<TestMutex>::mutex_type>::value));
    }

    //*************************************************************************
    TEST(test_lock_guard_is_not_copyable)
    {
      CHECK_FALSE((etl::is_copy_constructible<etl::lock_guard<TestMutex> >::value));
    }

    //*************************************************************************
    TEST(test_lock_guard_unlocks_when_an_exception_is_thrown)
    {
#if ETL_USING_EXCEPTIONS
      TestMutex mutex;

      try
      {
        etl::lock_guard<TestMutex> guard(mutex);

        CHECK_TRUE(mutex.locked);

        throw 1;
      }
      catch (int)
      {
      }

      CHECK_FALSE(mutex.locked);
      CHECK_EQUAL(1, mutex.lock_count);
      CHECK_EQUAL(1, mutex.unlock_count);
#endif
    }

    //*************************************************************************
    TEST(test_mutex_is_not_copyable)
    {
      CHECK_FALSE((etl::is_copy_assignable<etl::mutex>::value));
    }
  };
}

#endif
