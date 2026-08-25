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
#include "etl/semaphore.h"
#include "etl/chrono.h"
#include "etl/type_traits.h"

#if ETL_HAS_SEMAPHORE

#include <thread>
#include <atomic>
#include <cstddef>

namespace
{
  SUITE(test_semaphore)
  {
    //*************************************************************************
    TEST(test_traits_has_semaphore)
    {
      CHECK_TRUE(etl::traits::has_semaphore);
    }

    //*************************************************************************
    TEST(test_max)
    {
      CHECK_EQUAL(ptrdiff_t(1),  (etl::counting_semaphore<1>::max()));
      CHECK_EQUAL(ptrdiff_t(4),  (etl::counting_semaphore<4>::max()));
      CHECK_EQUAL(ptrdiff_t(10), (etl::counting_semaphore<10>::max()));
      CHECK_EQUAL(ptrdiff_t(1),  etl::binary_semaphore::max());
    }

    //*************************************************************************
    TEST(test_try_acquire_when_empty)
    {
      etl::counting_semaphore<4> semaphore(0);

      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_try_acquire_when_not_empty)
    {
      etl::counting_semaphore<4> semaphore(1);

      CHECK_TRUE(semaphore.try_acquire());
      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_try_acquire_counts_down_to_zero)
    {
      etl::counting_semaphore<4> semaphore(3);

      CHECK_TRUE(semaphore.try_acquire());
      CHECK_TRUE(semaphore.try_acquire());
      CHECK_TRUE(semaphore.try_acquire());
      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_release_makes_acquire_possible)
    {
      etl::counting_semaphore<4> semaphore(0);

      CHECK_FALSE(semaphore.try_acquire());

      semaphore.release();

      CHECK_TRUE(semaphore.try_acquire());
      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_release_multiple)
    {
      etl::counting_semaphore<4> semaphore(0);

      semaphore.release(3);

      CHECK_TRUE(semaphore.try_acquire());
      CHECK_TRUE(semaphore.try_acquire());
      CHECK_TRUE(semaphore.try_acquire());
      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_release_zero_does_nothing)
    {
      etl::counting_semaphore<4> semaphore(0);

      semaphore.release(0);

      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_acquire_when_already_available)
    {
      etl::counting_semaphore<4> semaphore(2);

      semaphore.acquire();
      semaphore.acquire();

      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_binary_semaphore)
    {
      etl::binary_semaphore semaphore(0);

      CHECK_FALSE(semaphore.try_acquire());

      semaphore.release();

      CHECK_TRUE(semaphore.try_acquire());
      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_binary_semaphore_initially_available)
    {
      etl::binary_semaphore semaphore(1);

      CHECK_TRUE(semaphore.try_acquire());
      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_try_acquire_for_success)
    {
      etl::counting_semaphore<4> semaphore(1);

      CHECK_TRUE(semaphore.try_acquire_for(etl::chrono::milliseconds(50)));
    }

    //*************************************************************************
    TEST(test_try_acquire_for_timeout)
    {
      etl::counting_semaphore<4> semaphore(0);

      CHECK_FALSE(semaphore.try_acquire_for(etl::chrono::milliseconds(10)));
    }

    //*************************************************************************
    TEST(test_try_acquire_for_timeout_waits_for_the_duration)
    {
      etl::counting_semaphore<4> semaphore(0);

      etl::chrono::steady_clock::time_point start = etl::chrono::steady_clock::now();

      CHECK_FALSE(semaphore.try_acquire_for(etl::chrono::milliseconds(20)));

      etl::chrono::steady_clock::duration elapsed = etl::chrono::steady_clock::now() - start;

      CHECK_TRUE(elapsed >= etl::chrono::milliseconds(20));
    }

    //*************************************************************************
    TEST(test_try_acquire_until_success)
    {
      etl::counting_semaphore<4> semaphore(1);

      CHECK_TRUE(semaphore.try_acquire_until(etl::chrono::steady_clock::now() + etl::chrono::milliseconds(50)));
    }

    //*************************************************************************
    TEST(test_try_acquire_until_timeout)
    {
      etl::counting_semaphore<4> semaphore(0);

      CHECK_FALSE(semaphore.try_acquire_until(etl::chrono::steady_clock::now() + etl::chrono::milliseconds(10)));
    }

    //*************************************************************************
    TEST(test_try_acquire_until_time_point_already_passed)
    {
      etl::counting_semaphore<4> semaphore(0);

      CHECK_FALSE(semaphore.try_acquire_until(etl::chrono::steady_clock::now() - etl::chrono::milliseconds(10)));
    }

    //*************************************************************************
    TEST(test_acquire_blocks_until_released_by_another_thread)
    {
      etl::counting_semaphore<4> semaphore(0);
      std::atomic<bool>          acquired(false);

      std::thread consumer([&semaphore, &acquired]()
        {
          semaphore.acquire();
          acquired = true;
        });

      std::this_thread::sleep_for(std::chrono::milliseconds(10));

      CHECK_FALSE(acquired.load());

      semaphore.release();

      consumer.join();

      CHECK_TRUE(acquired.load());
    }

    //*************************************************************************
    TEST(test_try_acquire_for_succeeds_when_released_by_another_thread)
    {
      etl::counting_semaphore<4> semaphore(0);

      std::thread producer([&semaphore]()
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          semaphore.release();
        });

      CHECK_TRUE(semaphore.try_acquire_for(etl::chrono::milliseconds(2000)));

      producer.join();
    }

    //*************************************************************************
    TEST(test_multiple_threads_acquire_all_permits)
    {
      const size_t Number_Of_Threads = 4U;

      etl::counting_semaphore<4> semaphore(0);
      std::atomic<size_t>        count(0);
      std::thread                threads[Number_Of_Threads];

      for (size_t i = 0U; i < Number_Of_Threads; ++i)
      {
        threads[i] = std::thread([&semaphore, &count]()
          {
            semaphore.acquire();
            ++count;
          });
      }

      semaphore.release(ptrdiff_t(Number_Of_Threads));

      for (size_t i = 0U; i < Number_Of_Threads; ++i)
      {
        threads[i].join();
      }

      CHECK_EQUAL(Number_Of_Threads, count.load());
      CHECK_FALSE(semaphore.try_acquire());
    }

    //*************************************************************************
    TEST(test_used_as_a_mutual_exclusion_primitive)
    {
      const size_t Number_Of_Threads     = 4U;
      const size_t Increments_Per_Thread = 1000U;

      etl::binary_semaphore semaphore(1);
      size_t                count = 0U;
      std::thread           threads[Number_Of_Threads];

      for (size_t i = 0U; i < Number_Of_Threads; ++i)
      {
        threads[i] = std::thread([&semaphore, &count]()
          {
            for (size_t j = 0U; j < Increments_Per_Thread; ++j)
            {
              semaphore.acquire();
              ++count;
              semaphore.release();
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
    TEST(test_is_not_copyable)
    {
      CHECK_FALSE((etl::is_copy_constructible<etl::counting_semaphore<4> >::value));
      CHECK_FALSE((etl::is_copy_assignable<etl::counting_semaphore<4> >::value));
    }
  };
}

#endif
