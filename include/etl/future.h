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

#ifndef ETL_FUTURE_INCLUDED
#define ETL_FUTURE_INCLUDED

#include "platform.h"
#include "alignment.h"
#include "atomic.h"
#include "error_handler.h"
#include "exception.h"
#include "file_error_numbers.h"
#include "placement_new.h"
#include "type_traits.h"

#include <stdint.h>

namespace etl
{
  //***************************************************************************
  /// Exception types for future/promise.
  //***************************************************************************
  class future_exception : public etl::exception
  {
  public:

    future_exception(string_type reason_, string_type file_name_, numeric_type line_number_)
      : exception(reason_, file_name_, line_number_)
    {
    }
  };

  class future_not_ready : public etl::future_exception
  {
  public:

    future_not_ready(string_type file_name_, numeric_type line_number_)
      : future_exception(ETL_ERROR_TEXT("future:not ready", ETL_FUTURE_FILE_ID"A"), file_name_, line_number_)
    {
    }
  };

  class promise_already_satisfied : public etl::future_exception
  {
  public:

    promise_already_satisfied(string_type file_name_, numeric_type line_number_)
      : future_exception(ETL_ERROR_TEXT("promise:already satisfied", ETL_FUTURE_FILE_ID"B"), file_name_, line_number_)
    {
    }
  };

  //***************************************************************************
  /// Shared state between a promise and a future.
  /// No heap allocation. The shared_state is typically embedded in a
  /// fixed-size pending-calls table owned by the RPC client.
  ///
  /// Thread safety: set_value() and is_ready()/get() may be called from
  /// different threads provided that at most ONE thread calls set_value()
  /// and at most ONE thread calls get(). The atomic ready flag provides
  /// acquire/release ordering.
  //***************************************************************************
  template <typename T>
  class shared_state
  {
  public:

    shared_state()
      : ready_(false)
    {
    }

    ~shared_state()
    {
      if (ready_.load(etl::memory_order_acquire))
      {
        reinterpret_cast<T*>(&storage_)->~T();
      }
    }

    //*************************************************************************
    /// Store the result value and mark the state as ready.
    //*************************************************************************
    void set_value(const T& value)
    {
      ::new (&storage_) T(value);
      ready_.store(true, etl::memory_order_release);
    }

    //*************************************************************************
    /// Store the result value (move) and mark the state as ready.
    //*************************************************************************
    void set_value(T&& value)
    {
      ::new (&storage_) T(etl::move(value));
      ready_.store(true, etl::memory_order_release);
    }

    //*************************************************************************
    /// Check whether the result is available.
    //*************************************************************************
    bool is_ready() const
    {
      return ready_.load(etl::memory_order_acquire);
    }

    //*************************************************************************
    /// Get the stored value. Undefined behaviour if not ready.
    //*************************************************************************
    T& get()
    {
      return *reinterpret_cast<T*>(&storage_);
    }

    const T& get() const
    {
      return *reinterpret_cast<const T*>(&storage_);
    }

    //*************************************************************************
    /// Reset for reuse.
    //*************************************************************************
    void reset()
    {
      if (ready_.load(etl::memory_order_acquire))
      {
        reinterpret_cast<T*>(&storage_)->~T();
        ready_.store(false, etl::memory_order_release);
      }
    }

  private:

    // Non-copyable.
    shared_state(const shared_state&) ETL_DELETE;
    shared_state& operator=(const shared_state&) ETL_DELETE;

    typename etl::aligned_storage<sizeof(T), etl::alignment_of<T>::value>::type storage_;
    etl::atomic_bool                                                            ready_;
  };

  //***************************************************************************
  /// Specialisation for void — no value storage, just a ready flag.
  //***************************************************************************
  template <>
  class shared_state<void>
  {
  public:

    shared_state()
      : ready_(false)
    {
    }

    void set_value()
    {
      ready_.store(true, etl::memory_order_release);
    }

    bool is_ready() const
    {
      return ready_.load(etl::memory_order_acquire);
    }

    void reset()
    {
      ready_.store(false, etl::memory_order_release);
    }

  private:

    shared_state(const shared_state&) ETL_DELETE;
    shared_state& operator=(const shared_state&) ETL_DELETE;

    etl::atomic_bool ready_;
  };

  //***************************************************************************
  /// A lightweight future — a non-owning handle to a shared_state<T>.
  /// The shared_state lifetime must be managed externally (e.g. by the
  /// RPC client's pending-call table).
  //***************************************************************************
  template <typename T>
  class future
  {
  public:

    future()
      : state_(nullptr)
    {
    }

    explicit future(shared_state<T>& state)
      : state_(&state)
    {
    }

    //*************************************************************************
    /// Returns true when the result is available.
    //*************************************************************************
    bool is_ready() const
    {
      return (state_ != nullptr) && state_->is_ready();
    }

    //*************************************************************************
    /// Returns true if this future is associated with a shared state.
    //*************************************************************************
    bool valid() const
    {
      return state_ != nullptr;
    }

    //*************************************************************************
    /// Get the result. Caller must ensure is_ready() == true.
    //*************************************************************************
    T& get()
    {
      ETL_ASSERT(state_ != nullptr && state_->is_ready(), ETL_ERROR(future_not_ready));
      return state_->get();
    }

    const T& get() const
    {
      ETL_ASSERT(state_ != nullptr && state_->is_ready(), ETL_ERROR(future_not_ready));
      return state_->get();
    }

  private:

    shared_state<T>* state_;
  };

  //***************************************************************************
  /// Specialisation for void.
  //***************************************************************************
  template <>
  class future<void>
  {
  public:

    future()
      : state_(nullptr)
    {
    }

    explicit future(shared_state<void>& state)
      : state_(&state)
    {
    }

    bool is_ready() const
    {
      return (state_ != nullptr) && state_->is_ready();
    }

    bool valid() const
    {
      return state_ != nullptr;
    }

    void get() const
    {
      ETL_ASSERT(state_ != nullptr && state_->is_ready(), ETL_ERROR(future_not_ready));
    }

  private:

    shared_state<void>* state_;
  };

  //***************************************************************************
  /// A lightweight promise — a non-owning handle that can set the value
  /// in a shared_state<T>.
  //***************************************************************************
  template <typename T>
  class promise
  {
  public:

    promise()
      : state_(nullptr)
    {
    }

    explicit promise(shared_state<T>& state)
      : state_(&state)
    {
    }

    //*************************************************************************
    /// Get the associated future.
    //*************************************************************************
    etl::future<T> get_future()
    {
      return etl::future<T>(*state_);
    }

    //*************************************************************************
    /// Set the value (fulfil the promise).
    //*************************************************************************
    void set_value(const T& value)
    {
      ETL_ASSERT(state_ != nullptr, ETL_ERROR(promise_already_satisfied));
      state_->set_value(value);
    }

    void set_value(T&& value)
    {
      ETL_ASSERT(state_ != nullptr, ETL_ERROR(promise_already_satisfied));
      state_->set_value(etl::move(value));
    }

    bool valid() const
    {
      return state_ != nullptr;
    }

  private:

    shared_state<T>* state_;
  };

  //***************************************************************************
  /// Specialisation for void.
  //***************************************************************************
  template <>
  class promise<void>
  {
  public:

    promise()
      : state_(nullptr)
    {
    }

    explicit promise(shared_state<void>& state)
      : state_(&state)
    {
    }

    etl::future<void> get_future()
    {
      return etl::future<void>(*state_);
    }

    void set_value()
    {
      ETL_ASSERT(state_ != nullptr, ETL_ERROR(promise_already_satisfied));
      state_->set_value();
    }

    bool valid() const
    {
      return state_ != nullptr;
    }

  private:

    shared_state<void>* state_;
  };

} // namespace etl

#endif // ETL_FUTURE_INCLUDED
