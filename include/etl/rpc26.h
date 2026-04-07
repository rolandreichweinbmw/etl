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

#ifndef ETL_RPC26_INCLUDED
#define ETL_RPC26_INCLUDED

#include "platform.h"
#include "alignment.h"
#include "array.h"
#include "atomic.h"
#include "basic_string.h"
#include "error_handler.h"
#include "future.h"
#include "meta.h"
#include "placement_new.h"
#include "queue_mpmc_mutex.h"
#include "queue_spsc_atomic.h"
#include "tuple.h"
#include "type_traits.h"
#include "vector.h"

#include <stdint.h>
#include <string.h>

#if ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION

namespace etl
{
  namespace rpc26
  {
    //=========================================================================
    // Forward declarations
    //=========================================================================
    template <size_t MaxPayload>
    struct rpc_message;

    //=========================================================================
    // rpc_message — the unit of data pushed through FIFOs.
    // MaxPayload is the maximum serialized argument/result size in bytes.
    //=========================================================================
    template <size_t MaxPayload>
    struct rpc_message
    {
      uint16_t call_id;             ///< Unique ID to match response to request.
      uint16_t func_index;          ///< Index of the method in the reflected member list.
      uint16_t payload_size;        ///< Actual number of bytes used in payload.
      uint8_t  service_id;          ///< Target service (for routing via rpc_router).
      uint8_t  client_id;           ///< Source client  (for response routing via rpc_router).
      uint8_t  payload[MaxPayload]; ///< Serialized arguments (request) or result (response).

      rpc_message()
        : call_id(0)
        , func_index(0)
        , payload_size(0)
        , service_id(0)
        , client_id(0)
      {
        // payload left uninitialised for performance; only payload_size bytes are valid.
      }
    };

    //=========================================================================
    // Serialization helpers.
    //
    // Supported types (in priority order):
    //   1. Primitives (arithmetic, enum): memcpy of sizeof(T) bytes.
    //   2. C-style fixed arrays T[N]: serialize N elements sequentially.
    //   3. etl::array<T, N>: serialize N elements sequentially.
    //   4. etl::vector<T, N> / etl::ivector<T>: uint16_t length prefix +
    //      elements.
    //   5. etl::string<N> / etl::ibasic_string<char>: uint16_t length
    //      prefix + character data (no null terminator on wire).
    //   6. Aggregate structs/classes: C++26 reflection iterates non-static
    //      data members recursively, tight wire format without padding.
    //
    // All serialization is recursive — a struct containing an
    // etl::vector<Point2D, 8> serializes each Point2D field-by-field.
    //=========================================================================
    namespace detail
    {
      // Forward declarations — needed because branches reference each other.
      template <typename T>
      size_t serialized_size_of(const T& value);

      template <typename T>
      size_t serialize_one(uint8_t* buffer, size_t offset, const T& value);

      template <typename T>
      size_t deserialize_one(const uint8_t* buffer, size_t offset, T& value);

      //*********************************************************************
      /// Type detection traits.
      //*********************************************************************

      /// Primitive: arithmetic or enum — memcpy-safe.
      template <typename T>
      struct is_primitive_serializable : etl::bool_constant<etl::is_arithmetic<T>::value || etl::is_enum<T>::value>
      {
      };

      /// etl::array<T, N> detector.
      template <typename T>
      struct is_etl_array : etl::false_type
      {
      };

      template <typename T, size_t N>
      struct is_etl_array<etl::array<T, N>> : etl::true_type
      {
      };

      /// etl::ivector<T> detector (matches etl::vector<T, N> too via base).
      template <typename T, typename = void>
      struct is_etl_vector : etl::false_type
      {
      };

      template <typename T>
      struct is_etl_vector<T, typename etl::enable_if<etl::is_base_of<etl::vector_base, T>::value>::type> : etl::true_type
      {
      };

      /// etl::ibasic_string<char> detector (matches etl::string<N> too).
      template <typename T, typename = void>
      struct is_etl_string : etl::false_type
      {
      };

      template <typename T>
      struct is_etl_string<T, typename etl::enable_if<etl::is_base_of<etl::string_base, T>::value>::type> : etl::true_type
      {
      };

      //=====================================================================
      // serialized_size_of — compute tight wire size at runtime.
      //=====================================================================

      /// C-style fixed array T[N].
      template <typename T, size_t N>
      size_t serialized_size_of(const T (&value)[N])
      {
        size_t total = 0;
        for (size_t i = 0; i < N; ++i)
        {
          total += serialized_size_of(value[i]);
        }
        return total;
      }

      /// Generic T: dispatches based on type traits.
      template <typename T>
      size_t serialized_size_of(const T& value)
      {
        if constexpr (is_primitive_serializable<T>::value)
        {
          (void)value;
          return sizeof(T);
        }
        else if constexpr (is_etl_array<T>::value)
        {
          size_t total = 0;
          for (size_t i = 0; i < T::SIZE; ++i)
          {
            total += serialized_size_of(value[i]);
          }
          return total;
        }
        else if constexpr (is_etl_vector<T>::value)
        {
          // Length prefix + elements.
          size_t total = sizeof(uint16_t);
          for (size_t i = 0; i < value.size(); ++i)
          {
            total += serialized_size_of(value[i]);
          }
          return total;
        }
        else if constexpr (is_etl_string<T>::value)
        {
          // Length prefix + character data.
          return sizeof(uint16_t) + value.size() * sizeof(typename T::value_type);
        }
        else
        {
          // Aggregate: sum fields via reflection.
          size_t         total   = 0;
#if ETL_META_NEEDS_INFO_ARRAY
          static constexpr auto _n = etl::meta::nonstatic_data_members_of(^^T, etl::meta::access_context::unchecked()).size();
          static constexpr auto _a = etl::meta::to_info_array<_n>(etl::meta::nonstatic_data_members_of(^^T, etl::meta::access_context::unchecked()));
          template for (constexpr auto member : _a)
#else
          constexpr auto members = etl::meta::nonstatic_data_members_of(^^T, etl::meta::access_context::unchecked());
          template for (constexpr auto member : members)
#endif
          {
            total += serialized_size_of(value.[:member:]);
          }
          return total;
        }
      }

      //=====================================================================
      // serialize_one — write a value into buffer at offset.
      //=====================================================================

      /// C-style fixed array T[N].
      template <typename T, size_t N>
      size_t serialize_one(uint8_t* buffer, size_t offset, const T (&value)[N])
      {
        for (size_t i = 0; i < N; ++i)
        {
          offset = serialize_one(buffer, offset, value[i]);
        }
        return offset;
      }

      /// Generic T.
      template <typename T>
      size_t serialize_one(uint8_t* buffer, size_t offset, const T& value)
      {
        if constexpr (is_primitive_serializable<T>::value)
        {
          memcpy(buffer + offset, &value, sizeof(T));
          return offset + sizeof(T);
        }
        else if constexpr (is_etl_array<T>::value)
        {
          for (size_t i = 0; i < T::SIZE; ++i)
          {
            offset = serialize_one(buffer, offset, value[i]);
          }
          return offset;
        }
        else if constexpr (is_etl_vector<T>::value)
        {
          // Write length prefix (uint16_t) then elements.
          uint16_t len = static_cast<uint16_t>(value.size());
          memcpy(buffer + offset, &len, sizeof(uint16_t));
          offset += sizeof(uint16_t);
          for (size_t i = 0; i < value.size(); ++i)
          {
            offset = serialize_one(buffer, offset, value[i]);
          }
          return offset;
        }
        else if constexpr (is_etl_string<T>::value)
        {
          // Write length prefix (uint16_t) then raw characters (no null).
          uint16_t len = static_cast<uint16_t>(value.size());
          memcpy(buffer + offset, &len, sizeof(uint16_t));
          offset += sizeof(uint16_t);
          size_t byte_count = value.size() * sizeof(typename T::value_type);
          memcpy(buffer + offset, value.data(), byte_count);
          return offset + byte_count;
        }
        else
        {
          // Aggregate: serialize each non-static data member in order.
#if ETL_META_NEEDS_INFO_ARRAY
          static constexpr auto _n = etl::meta::nonstatic_data_members_of(^^T, etl::meta::access_context::unchecked()).size();
          static constexpr auto _a = etl::meta::to_info_array<_n>(etl::meta::nonstatic_data_members_of(^^T, etl::meta::access_context::unchecked()));
          template for (constexpr auto member : _a)
#else
          constexpr auto members = etl::meta::nonstatic_data_members_of(^^T, etl::meta::access_context::unchecked());
          template for (constexpr auto member : members)
#endif
          {
            offset = serialize_one(buffer, offset, value.[:member:]);
          }
          return offset;
        }
      }

      //=====================================================================
      // deserialize_one — read a value from buffer at offset.
      //=====================================================================

      /// C-style fixed array T[N].
      template <typename T, size_t N>
      size_t deserialize_one(const uint8_t* buffer, size_t offset, T (&value)[N])
      {
        for (size_t i = 0; i < N; ++i)
        {
          offset = deserialize_one(buffer, offset, value[i]);
        }
        return offset;
      }

      /// Generic T.
      template <typename T>
      size_t deserialize_one(const uint8_t* buffer, size_t offset, T& value)
      {
        if constexpr (is_primitive_serializable<T>::value)
        {
          memcpy(&value, buffer + offset, sizeof(T));
          return offset + sizeof(T);
        }
        else if constexpr (is_etl_array<T>::value)
        {
          for (size_t i = 0; i < T::SIZE; ++i)
          {
            offset = deserialize_one(buffer, offset, value[i]);
          }
          return offset;
        }
        else if constexpr (is_etl_vector<T>::value)
        {
          // Read length prefix, then elements. Resize the vector.
          uint16_t len = 0;
          memcpy(&len, buffer + offset, sizeof(uint16_t));
          offset += sizeof(uint16_t);
          value.clear();
          for (uint16_t i = 0; i < len; ++i)
          {
            typename T::value_type elem{};
            offset = deserialize_one(buffer, offset, elem);
            value.push_back(elem);
          }
          return offset;
        }
        else if constexpr (is_etl_string<T>::value)
        {
          // Read length prefix, then raw characters.
          uint16_t len = 0;
          memcpy(&len, buffer + offset, sizeof(uint16_t));
          offset += sizeof(uint16_t);
          value.clear();
          size_t byte_count = len * sizeof(typename T::value_type);
          value.assign(reinterpret_cast<const typename T::value_type*>(buffer + offset), len);
          return offset + byte_count;
        }
        else
        {
          // Aggregate: deserialize each non-static data member in order.
#if ETL_META_NEEDS_INFO_ARRAY
          static constexpr auto _n = etl::meta::nonstatic_data_members_of(^^T, etl::meta::access_context::unchecked()).size();
          static constexpr auto _a = etl::meta::to_info_array<_n>(etl::meta::nonstatic_data_members_of(^^T, etl::meta::access_context::unchecked()));
          template for (constexpr auto member : _a)
#else
          constexpr auto members = etl::meta::nonstatic_data_members_of(^^T, etl::meta::access_context::unchecked());
          template for (constexpr auto member : members)
#endif
          {
            offset = deserialize_one(buffer, offset, value.[:member:]);
          }
          return offset;
        }
      }

      //*********************************************************************
      /// Serialize zero or more arguments sequentially into buffer.
      /// Returns total bytes written.
      //*********************************************************************
      inline size_t serialize_args_impl(uint8_t* /*buffer*/, size_t offset)
      {
        return offset;
      }

      template <typename First, typename... Rest>
      size_t serialize_args_impl(uint8_t* buffer, size_t offset, const First& first, const Rest&... rest)
      {
        offset = serialize_one(buffer, offset, first);
        return serialize_args_impl(buffer, offset, rest...);
      }

      //*********************************************************************
      /// Deserialize into a tuple of values from buffer.
      //*********************************************************************
      template <typename Tuple, size_t... Is>
      size_t deserialize_tuple_impl([[maybe_unused]] const uint8_t* buffer, size_t offset, [[maybe_unused]] Tuple& t, etl::index_sequence<Is...>)
      {
        // Fold expression: deserialize each element in order.
        ((offset = deserialize_one(buffer, offset, etl::get<Is>(t))), ...);
        return offset;
      }

      template <typename... Args>
      etl::tuple<Args...> deserialize_args(const uint8_t* buffer)
      {
        etl::tuple<Args...> result{};
        deserialize_tuple_impl(buffer, 0, result, etl::make_index_sequence<sizeof...(Args)>{});
        return result;
      }
    } // namespace detail

    //*********************************************************************
    /// Public API: serialize argument pack into buffer.
    /// Returns number of bytes written.
    //*********************************************************************
    template <typename... Args>
    size_t serialize_args(uint8_t* buffer, const Args&... args)
    {
      return detail::serialize_args_impl(buffer, 0, args...);
    }

    //=========================================================================
    // rpc_channel — owns request + response SPSC atomic queues.
    // Lock-free, suitable for cross-thread communication on RTOS.
    //
    // Template parameters:
    //   MaxPayload — max serialized arg/result size in bytes
    //   QueueDepth — how many messages each FIFO can hold
    //=========================================================================
    template <size_t MaxPayload, size_t QueueDepth>
    class rpc_channel
    {
    public:

      using message_type = rpc_message<MaxPayload>;

      //*********************************************************************
      /// Push a request message (caller -> callee).
      //*********************************************************************
      bool push_request(const message_type& msg)
      {
        return request_queue_.push(msg);
      }

      //*********************************************************************
      /// Pop a request message (callee side).
      //*********************************************************************
      bool pop_request(message_type& msg)
      {
        return request_queue_.pop(msg);
      }

      //*********************************************************************
      /// Push a response message (callee -> caller).
      //*********************************************************************
      bool push_response(const message_type& msg)
      {
        return response_queue_.push(msg);
      }

      //*********************************************************************
      /// Pop a response message (caller side).
      //*********************************************************************
      bool pop_response(message_type& msg)
      {
        return response_queue_.pop(msg);
      }

    private:

      etl::queue_spsc_atomic<message_type, QueueDepth> request_queue_;
      etl::queue_spsc_atomic<message_type, QueueDepth> response_queue_;
    };

    //=========================================================================
    // Compile-time method table helpers.
    // Uses C++26 reflection to discover non-special public member functions
    // of a service class and assign each a sequential func_index.
    //=========================================================================
    namespace detail
    {
      //***********************************************************************
      /// Consteval: count the callable (non-special, public) member functions
      /// of a reflected class.
      //***********************************************************************
      consteval size_t count_rpc_methods(etl::meta::info service_type)
      {
        size_t count   = 0;
        auto   members = etl::meta::members_of(service_type, etl::meta::access_context::unchecked());
        for (auto m : members)
        {
          if (etl::meta::is_function(m) && etl::meta::is_public(m) && !etl::meta::is_special_member_function(m) && !etl::meta::is_constructor(m)
              && !etl::meta::is_destructor(m))
          {
            ++count;
          }
        }
        return count;
      }

      //***********************************************************************
      /// Consteval helper: check if reflected member 'fn' is the N-th
      /// callable RPC method (0-based) of the class.
      //***********************************************************************
      consteval bool is_rpc_method(etl::meta::info fn)
      {
        return etl::meta::is_function(fn) && etl::meta::is_public(fn) && !etl::meta::is_special_member_function(fn) && !etl::meta::is_constructor(fn)
               && !etl::meta::is_destructor(fn);
      }

      //***********************************************************************
      /// Compute the RPC func_index for a given reflected member function
      /// within its parent class. Returns the 0-based index among all
      /// RPC-eligible methods.
      //***********************************************************************
      consteval uint16_t func_index_of(etl::meta::info fn)
      {
        auto     parent  = etl::meta::parent_of(fn);
        auto     members = etl::meta::members_of(parent, etl::meta::access_context::unchecked());
        uint16_t idx     = 0;
        for (auto m : members)
        {
          if (m == fn)
            return idx;
          if (is_rpc_method(m))
            ++idx;
        }
        return idx; // Should not reach here for valid fn.
      }

      //***********************************************************************
      /// apply_tuple: call f(tuple_elements...) using index_sequence.
      //***********************************************************************
      template <typename F, typename Tuple, size_t... Is>
      auto apply_tuple_impl(F&& f, Tuple& t, etl::index_sequence<Is...>) -> decltype(f(etl::get<Is>(t)...))
      {
        return f(etl::get<Is>(t)...);
      }

      template <typename F, typename... Ts>
      auto apply_tuple(F&& f, etl::tuple<Ts...>& t) -> decltype(apply_tuple_impl(static_cast<F&&>(f), t, etl::make_index_sequence<sizeof...(Ts)>{}))
      {
        return apply_tuple_impl(static_cast<F&&>(f), t, etl::make_index_sequence<sizeof...(Ts)>{});
      }

      //***********************************************************************
      /// invoke_method: call a specific member function on impl with
      /// arguments deserialized from the request payload, serialize the
      /// result into the response payload.
      ///
      /// This is a template function parameterised on the member-function
      /// reflection only.  It extracts parameter types and return type
      /// internally via reflection, avoiding splice-in-template-arg issues.
      //***********************************************************************
      template <etl::meta::info Fn, typename Impl, size_t MaxPayload, size_t... Is>
      void invoke_method_impl(Impl& impl, const rpc_message<MaxPayload>& request, rpc_message<MaxPayload>& response, etl::index_sequence<Is...>)
      {
#if ETL_META_NEEDS_INFO_ARRAY
        static constexpr auto _pc = etl::meta::parameters_of(Fn).size();
        [[maybe_unused]] static constexpr auto params = etl::meta::to_info_array<_pc>(etl::meta::parameters_of(Fn));
#else
        [[maybe_unused]] constexpr auto params = etl::meta::parameters_of(Fn);
#endif
        using ReturnType      = typename[:etl::meta::return_type_of(Fn):];

        // Deserialize arguments from request payload.
        auto args = detail::deserialize_args<typename[:etl::meta::type_of(params[Is]):]...>(request.payload);

        // Call the actual method on the implementation.
        if constexpr (etl::is_void_v<ReturnType>)
        {
          detail::apply_tuple([&impl](auto&... a) { (impl.[:Fn:])(a...); }, args);
          response.payload_size = 0;
        }
        else
        {
          ReturnType result     = detail::apply_tuple([&impl](auto&... a) -> ReturnType { return (impl.[:Fn:])(a...); }, args);
          response.payload_size = static_cast<uint16_t>(detail::serialize_one(response.payload, 0, result));
        }

        response.call_id    = request.call_id;
        response.func_index = request.func_index;
        response.service_id = request.service_id;
        response.client_id  = request.client_id;
      }

      template <etl::meta::info Fn, typename Impl, size_t MaxPayload>
      void invoke_method(Impl& impl, const rpc_message<MaxPayload>& request, rpc_message<MaxPayload>& response)
      {
#if ETL_META_NEEDS_INFO_ARRAY
        static constexpr auto _pc = etl::meta::parameters_of(Fn).size();
        static constexpr auto params = etl::meta::to_info_array<_pc>(etl::meta::parameters_of(Fn));
#else
        constexpr auto params = etl::meta::parameters_of(Fn);
#endif
        invoke_method_impl<Fn>(impl, request, response, etl::make_index_sequence<params.size()>{});
      }
    } // namespace detail

    //=========================================================================
    // rpc_server — callee side.
    //
    // Template parameters:
    //   ServiceImpl — the user's class that implements the service methods.
    //   MaxPayload  — max serialized arg/result size in bytes.
    //   QueueDepth  — FIFO depth (messages).
    //
    // Usage: construct with a reference to the channel and impl, then call
    // process() cyclically from the RTOS callee task/thread.
    //=========================================================================
    template <typename ServiceImpl, size_t MaxPayload, size_t QueueDepth>
    class rpc_server
    {
    public:

      using channel_type = rpc_channel<MaxPayload, QueueDepth>;
      using message_type = rpc_message<MaxPayload>;

      rpc_server(channel_type& channel, ServiceImpl& impl)
        : channel_(channel)
        , impl_(impl)
      {
      }

      //*********************************************************************
      /// Process one pending request from the FIFO.
      /// Call this cyclically from the callee thread/task.
      /// Returns true if a request was processed, false if queue was empty.
      //*********************************************************************
      bool process()
      {
        message_type request;
        if (!channel_.pop_request(request))
        {
          return false; // Nothing pending.
        }

        message_type response;

        // Build a dispatch table at compile time using reflection.
        // We iterate all public non-special member functions and assign
        // each a sequential index. When request.func_index matches,
        // we invoke that method.
        uint16_t idx        = 0;
        bool     dispatched = false;

        // Expansion statement: iterate over all members of ServiceImpl
        // at compile time. For each RPC-eligible method, check if its
        // index matches the request's func_index at runtime.
#if ETL_META_NEEDS_INFO_ARRAY
        static constexpr auto _n = etl::meta::members_of(^^ServiceImpl, etl::meta::access_context::unchecked()).size();
        static constexpr auto _a = etl::meta::to_info_array<_n>(etl::meta::members_of(^^ServiceImpl, etl::meta::access_context::unchecked()));
        template for (constexpr auto fn : _a)
#else
        constexpr auto all_members = etl::meta::members_of(^^ServiceImpl, etl::meta::access_context::unchecked());
        template for (constexpr auto fn : all_members)
#endif
        {
          if constexpr (detail::is_rpc_method(fn))
          {
            if (!dispatched && idx == request.func_index)
            {
              detail::invoke_method<fn>(impl_, request, response);
              dispatched = true;
            }
            ++idx;
          }
        }

        if (dispatched)
        {
          channel_.push_response(response);
        }

        return dispatched;
      }

    private:

      channel_type& channel_;
      ServiceImpl&  impl_;
    };

    //=========================================================================
    // rpc_client — caller side.
    //
    // Generates stub methods via reflection. Each stub serializes arguments
    // into an rpc_message and pushes it into the request FIFO, returning
    // an etl::future<ReturnType> that will be fulfilled when poll() picks
    // up the matching response.
    //
    // Template parameters:
    //   ServiceInterface — the class whose public methods define the RPC API.
    //   MaxPayload       — max serialized arg/result size in bytes.
    //   QueueDepth       — FIFO depth (messages).
    //   MaxPending       — max number of in-flight async calls.
    //
    // Usage: construct with a reference to the channel, then call stubs via
    // call<&Service::method>(args...). Call poll() cyclically from the
    // caller thread/task to receive responses.
    //=========================================================================
    template <typename ServiceInterface, size_t MaxPayload, size_t QueueDepth, size_t MaxPending = 8>
    class rpc_client
    {
    public:

      using channel_type = rpc_channel<MaxPayload, QueueDepth>;
      using message_type = rpc_message<MaxPayload>;

    private:

      //*********************************************************************
      /// Maximum result object size. Deserialized results can be at most
      /// this large (the same bound as the payload buffer).
      //*********************************************************************
      static constexpr size_t MAX_RESULT_SIZE  = MaxPayload;
      static constexpr size_t MAX_RESULT_ALIGN = etl::alignment_of<uint64_t>::value;

      //*********************************************************************
      /// Size of the embedded shared_state storage inside each slot.
      /// A shared_state<T> contains aligned_storage<sizeof(T)> + atomic_bool,
      /// so we conservatively reserve enough for the largest T.
      //*********************************************************************
      static constexpr size_t STATE_STORAGE_SIZE =
        sizeof(typename etl::aligned_storage<MAX_RESULT_SIZE, MAX_RESULT_ALIGN>::type) + sizeof(etl::atomic_bool) + sizeof(void*); // padding headroom

      //*********************************************************************
      /// A single pending-call slot.
      ///
      /// Embeds storage for an etl::shared_state<ReturnType> so the caller
      /// does not need to manage one externally. The fulfil callback
      /// (set by call(), invoked by poll()) deserializes the response and
      /// calls set_value() on the in-place shared_state. The destroy
      /// callback correctly destructs the typed shared_state when the slot
      /// is released.
      //*********************************************************************
      struct pending_slot
      {
        uint16_t call_id;
        bool     in_use;

        /// Type-erased fulfil: deserialise response payload, call set_value.
        void (*fulfil_fn)(void* state_ptr, const uint8_t* payload, uint16_t payload_size);

        /// Type-erased destroy: destruct the shared_state<T> in place.
        void (*destroy_fn)(void* state_ptr);

        /// Raw storage for the embedded shared_state<ReturnType>.
        typename etl::aligned_storage<STATE_STORAGE_SIZE, MAX_RESULT_ALIGN>::type state_storage;

        pending_slot()
          : call_id(0)
          , in_use(false)
          , fulfil_fn(nullptr)
          , destroy_fn(nullptr)
        {
        }

        /// Get a void* to the embedded state storage.
        void* state_ptr()
        {
          return &state_storage;
        }
      };

    public:

      rpc_client(channel_type& channel, uint8_t service_id = 0, uint8_t client_id = 0)
        : channel_(channel)
        , next_call_id_(1)
        , service_id_(service_id)
        , client_id_(client_id)
      {
      }

      ~rpc_client()
      {
        // Destroy any shared_states (in-flight or completed but unconsumed).
        for (size_t i = 0; i < MaxPending; ++i)
        {
          if (pending_[i].destroy_fn != nullptr)
          {
            pending_[i].destroy_fn(pending_[i].state_ptr());
          }
        }
      }

      //*********************************************************************
      /// Invoke a remote method by reflection.
      ///
      /// Fn is a compile-time reflection (etl::meta::info) of a member
      /// function of ServiceInterface.  The func_index, return type, and
      /// parameter types are all deduced automatically.
      ///
      /// Usage:
      ///   auto fut = client.call<^^Calculator::add>(3, 4);
      ///
      /// Returns an etl::future<ReturnType>.  The underlying shared_state
      /// is embedded inside the client's pending-slot array — no external
      /// shared_state management is required.
      //*********************************************************************

    private:

      template <etl::meta::info Fn, size_t... Is, typename... ArgTypes>
      auto call_impl(etl::index_sequence<Is...>, const ArgTypes&... args) -> etl::future<typename[:etl::meta::return_type_of(Fn):]>
      {
        using ReturnType             = typename[:etl::meta::return_type_of(Fn):];
        constexpr uint16_t FuncIndex = detail::func_index_of(Fn);

        // Find a free pending slot.
        pending_slot* slot = nullptr;
        for (size_t i = 0; i < MaxPending; ++i)
        {
          if (!pending_[i].in_use)
          {
            slot = &pending_[i];
            break;
          }
        }

        if (slot == nullptr)
        {
          // All slots busy — return invalid future.
          return etl::future<ReturnType>();
        }

        // Destroy any previous shared_state in this slot (from a prior,
        // already-completed call whose future has been consumed).
        if (slot->destroy_fn != nullptr)
        {
          slot->destroy_fn(slot->state_ptr());
          slot->destroy_fn = nullptr;
        }

        // Construct the shared_state<ReturnType> in-place inside the slot.
        auto* state = ::new (slot->state_ptr()) etl::shared_state<ReturnType>();

        // Build the request message.
        message_type msg;
        msg.call_id      = next_call_id_++;
        msg.func_index   = FuncIndex;
        msg.service_id   = service_id_;
        msg.client_id    = client_id_;
        msg.payload_size = static_cast<uint16_t>(serialize_args(msg.payload, args...));

        // Set up the pending slot.
        slot->call_id = msg.call_id;
        slot->in_use  = true;

        // Set the destroy callback for proper cleanup.
        slot->destroy_fn = [](void* sp)
        {
          static_cast<etl::shared_state<ReturnType>*>(sp)->~shared_state();
        };

        if constexpr (etl::is_void_v<ReturnType>)
        {
          slot->fulfil_fn = [](void* sp, const uint8_t* /*payload*/, uint16_t /*size*/)
          {
            static_cast<etl::shared_state<void>*>(sp)->set_value();
          };
        }
        else
        {
          slot->fulfil_fn = [](void* sp, const uint8_t* payload, uint16_t /*size*/)
          {
            ReturnType value{};
            detail::deserialize_one(payload, 0, value);
            static_cast<etl::shared_state<ReturnType>*>(sp)->set_value(value);
          };
        }

        // Push into the request queue.
        channel_.push_request(msg);

        // Return a future pointing to the embedded shared state.
        return etl::future<ReturnType>(*state);
      }

    public:

      template <etl::meta::info Fn, typename... ArgTypes>
      auto call(const ArgTypes&... args) -> etl::future<typename[:etl::meta::return_type_of(Fn):]>
      {
#if ETL_META_NEEDS_INFO_ARRAY
        static constexpr auto _pc = etl::meta::parameters_of(Fn).size();
        static constexpr auto params = etl::meta::to_info_array<_pc>(etl::meta::parameters_of(Fn));
#else
        constexpr auto params = etl::meta::parameters_of(Fn);
#endif
        return call_impl<Fn>(etl::make_index_sequence<params.size()>{}, args...);
      }

      //*********************************************************************
      /// Poll for responses from the server.
      /// Call this cyclically from the caller thread/task.
      /// Returns true if a response was processed.
      //*********************************************************************
      bool poll()
      {
        message_type response;
        if (!channel_.pop_response(response))
        {
          return false;
        }

        // Find the pending slot matching the call_id.
        for (size_t i = 0; i < MaxPending; ++i)
        {
          if (pending_[i].in_use && pending_[i].call_id == response.call_id)
          {
            // Fulfil the promise via the type-erased callback.
            pending_[i].fulfil_fn(pending_[i].state_ptr(), response.payload, response.payload_size);
            // Note: we do NOT destroy the shared_state here — the future
            // still references it. The state is destroyed when the slot is
            // reused by a subsequent call(), or when the client is destroyed.
            pending_[i].in_use = false;
            return true;
          }
        }

        // Orphan response (no matching pending call) — discard.
        return false;
      }

    private:

      channel_type& channel_;
      uint16_t      next_call_id_;
      uint8_t       service_id_;
      uint8_t       client_id_;
      pending_slot  pending_[MaxPending];
    };

    //=========================================================================
    // rpc_router — multiplexing layer for N:M client-server topologies.
    //
    // Each client and each server connects to the router via a dedicated
    // SPSC channel (preserving the lock-free single-producer/single-consumer
    // contract). The router's route() method moves messages between channels
    // based on service_id (for requests) and client_id (for responses).
    //
    // Typical RTOS usage:
    //   - One "router task" calls route() cyclically.
    //   - Each server task calls server.process() cyclically.
    //   - Each client task calls client.call() / client.poll() as needed.
    //
    // Template parameters:
    //   MaxPayload  — max serialized arg/result size in bytes.
    //   QueueDepth  — FIFO depth per channel.
    //   MaxClients  — maximum number of client endpoints.
    //   MaxServers  — maximum number of server endpoints.
    //=========================================================================
    template <size_t MaxPayload, size_t QueueDepth, size_t MaxClients, size_t MaxServers>
    class rpc_router
    {
    public:

      using channel_type = rpc_channel<MaxPayload, QueueDepth>;
      using message_type = rpc_message<MaxPayload>;

      rpc_router()
        : num_clients_(0)
        , num_servers_(0)
      {
        for (size_t i = 0; i < MaxServers; ++i)
        {
          server_map_[i].service_id = 0;
          server_map_[i].registered = false;
        }
      }

      //*********************************************************************
      /// Connect a client endpoint.
      ///
      /// Returns a reference to the client's dedicated channel and assigns
      /// a client_id (used to route responses back). Pass this channel and
      /// client_id to the rpc_client constructor.
      //*********************************************************************
      channel_type& connect_client(uint8_t& client_id)
      {
        client_id = static_cast<uint8_t>(num_clients_);
        return client_channels_[num_clients_++];
      }

      //*********************************************************************
      /// Connect a server endpoint for the given service_id.
      ///
      /// Returns a reference to the server's dedicated channel. Pass this
      /// channel to the rpc_server constructor.
      //*********************************************************************
      channel_type& connect_server(uint8_t service_id)
      {
        size_t idx                  = num_servers_++;
        server_map_[idx].service_id = service_id;
        server_map_[idx].registered = true;
        return server_channels_[idx];
      }

      //*********************************************************************
      /// Route messages between client and server channels.
      ///
      /// Call this cyclically from a dedicated router task/thread.
      /// - Requests:  pulled from each client channel, forwarded to the
      ///              server channel matching the message's service_id.
      /// - Responses: pulled from each server channel, forwarded to the
      ///              client channel matching the message's client_id.
      ///
      /// Returns total number of messages routed in this call.
      //*********************************************************************
      size_t route()
      {
        size_t count = 0;

        // Route requests: client -> server (by service_id).
        for (size_t c = 0; c < num_clients_; ++c)
        {
          message_type msg;
          while (client_channels_[c].pop_request(msg))
          {
            // Stamp the client_id so the response can be routed back.
            msg.client_id = static_cast<uint8_t>(c);

            for (size_t s = 0; s < num_servers_; ++s)
            {
              if (server_map_[s].registered && server_map_[s].service_id == msg.service_id)
              {
                server_channels_[s].push_request(msg);
                ++count;
                break;
              }
            }
          }
        }

        // Route responses: server -> client (by client_id).
        for (size_t s = 0; s < num_servers_; ++s)
        {
          message_type msg;
          while (server_channels_[s].pop_response(msg))
          {
            uint8_t cid = msg.client_id;
            if (cid < num_clients_)
            {
              client_channels_[cid].push_response(msg);
              ++count;
            }
          }
        }

        return count;
      }

    private:

      struct server_entry
      {
        uint8_t service_id;
        bool    registered;
      };

      channel_type client_channels_[MaxClients];
      channel_type server_channels_[MaxServers];
      server_entry server_map_[MaxServers];
      size_t       num_clients_;
      size_t       num_servers_;
    };

    //=========================================================================
    // rpc_mpmc_channel — mutex-protected MPMC variant of rpc_channel.
    //
    // Uses etl::queue_mpmc_mutex instead of queue_spsc_atomic, allowing
    // multiple producers and multiple consumers on the same queue. This
    // is used by rpc_mpmc_router for multi-core routing scenarios where
    // route() may be called from several cores simultaneously.
    //
    // Template parameters:
    //   MaxPayload — max serialized arg/result size in bytes.
    //   QueueDepth — how many messages each FIFO can hold.
    //=========================================================================
    template <size_t MaxPayload, size_t QueueDepth>
    class rpc_mpmc_channel
    {
    public:

      using message_type = rpc_message<MaxPayload>;

      bool push_request(const message_type& msg)
      {
        return request_queue_.push(msg);
      }

      bool pop_request(message_type& msg)
      {
        return request_queue_.pop(msg);
      }

      bool push_response(const message_type& msg)
      {
        return response_queue_.push(msg);
      }

      bool pop_response(message_type& msg)
      {
        return response_queue_.pop(msg);
      }

    private:

      etl::queue_mpmc_mutex<message_type, QueueDepth> request_queue_;
      etl::queue_mpmc_mutex<message_type, QueueDepth> response_queue_;
    };

    //=========================================================================
    // rpc_mpmc_router — multi-core-safe routing/multiplexing layer.
    //
    // Like rpc_router, but uses rpc_mpmc_channel internally so that
    // route() can be called from multiple cores/threads simultaneously.
    // The mutex inside each queue serialises concurrent access.
    //
    // Each client still gets a dedicated rpc_channel (SPSC, lock-free)
    // for its own push/pop — only the internal hub queues are MPMC.
    //
    // Architecture:
    //
    //   Client A ─[SPSC]─┐                              ┌─[SPSC]─ Server 0
    //                     │  request_hub_ [MPMC]         │
    //   Client B ─[SPSC]─┤ ◄────────────────────────► ──┤
    //                     │  response_hub_ [MPMC]        │
    //   Client C ─[SPSC]─┘                              └─[SPSC]─ Server 1
    //
    //   Any core can call route() — the MPMC hub queues handle contention.
    //
    // Template parameters:
    //   MaxPayload  — max serialized arg/result size in bytes.
    //   QueueDepth  — FIFO depth per channel.
    //   MaxClients  — maximum number of client endpoints.
    //   MaxServers  — maximum number of server endpoints.
    //=========================================================================
    template <size_t MaxPayload, size_t QueueDepth, size_t MaxClients, size_t MaxServers>
    class rpc_mpmc_router
    {
    public:

      using spsc_channel_type = rpc_channel<MaxPayload, QueueDepth>;
      using message_type      = rpc_message<MaxPayload>;

      rpc_mpmc_router()
        : num_clients_(0)
        , num_servers_(0)
      {
        for (size_t i = 0; i < MaxServers; ++i)
        {
          server_map_[i].service_id = 0;
          server_map_[i].registered = false;
        }
      }

      //*********************************************************************
      /// Connect a client endpoint.
      /// Returns the client's dedicated SPSC channel and assigns a client_id.
      /// NOT thread-safe — call during init only.
      //*********************************************************************
      spsc_channel_type& connect_client(uint8_t& client_id)
      {
        client_id = static_cast<uint8_t>(num_clients_);
        return client_channels_[num_clients_++];
      }

      //*********************************************************************
      /// Connect a server endpoint for the given service_id.
      /// Returns the server's dedicated SPSC channel.
      /// NOT thread-safe — call during init only.
      //*********************************************************************
      spsc_channel_type& connect_server(uint8_t service_id)
      {
        size_t idx                  = num_servers_++;
        server_map_[idx].service_id = service_id;
        server_map_[idx].registered = true;
        return server_channels_[idx];
      }

      //*********************************************************************
      /// Route messages between client and server channels.
      ///
      /// THREAD-SAFE — may be called from multiple cores simultaneously.
      ///
      /// Internally uses MPMC hub queues to decouple the routing work:
      ///
      ///   Phase 1 (ingest):  pop from client SPSC queues → push to
      ///                      request_hub_ (MPMC).
      ///   Phase 2 (fanout):  pop from request_hub_ (MPMC) → push to
      ///                      correct server SPSC queue by service_id.
      ///   Phase 3 (ingest):  pop from server SPSC queues → push to
      ///                      response_hub_ (MPMC).
      ///   Phase 4 (fanout):  pop from response_hub_ (MPMC) → push to
      ///                      correct client SPSC queue by client_id.
      ///
      /// Returns total number of messages routed in this call.
      //*********************************************************************
      size_t route()
      {
        size_t       count = 0;
        message_type msg;

        // Phase 1: client SPSC → request hub (MPMC).
        for (size_t c = 0; c < num_clients_; ++c)
        {
          while (client_channels_[c].pop_request(msg))
          {
            msg.client_id = static_cast<uint8_t>(c);
            request_hub_.push(msg);
          }
        }

        // Phase 2: request hub (MPMC) → server SPSC by service_id.
        while (request_hub_.pop(msg))
        {
          for (size_t s = 0; s < num_servers_; ++s)
          {
            if (server_map_[s].registered && server_map_[s].service_id == msg.service_id)
            {
              server_channels_[s].push_request(msg);
              ++count;
              break;
            }
          }
        }

        // Phase 3: server SPSC → response hub (MPMC).
        for (size_t s = 0; s < num_servers_; ++s)
        {
          while (server_channels_[s].pop_response(msg))
          {
            response_hub_.push(msg);
          }
        }

        // Phase 4: response hub (MPMC) → client SPSC by client_id.
        while (response_hub_.pop(msg))
        {
          uint8_t cid = msg.client_id;
          if (cid < num_clients_)
          {
            client_channels_[cid].push_response(msg);
            ++count;
          }
        }

        return count;
      }

    private:

      struct server_entry
      {
        uint8_t service_id;
        bool    registered;
      };

      /// Per-client and per-server SPSC channels (lock-free endpoints).
      spsc_channel_type client_channels_[MaxClients];
      spsc_channel_type server_channels_[MaxServers];

      /// Central MPMC hub queues — these allow concurrent route() calls.
      /// Sized to hold the sum of all endpoint depths.
      etl::queue_mpmc_mutex<message_type, QueueDepth * MaxClients> request_hub_;
      etl::queue_mpmc_mutex<message_type, QueueDepth * MaxServers> response_hub_;

      server_entry server_map_[MaxServers];
      size_t       num_clients_;
      size_t       num_servers_;
    };

  } // namespace rpc26
} // namespace etl

#endif // ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION

#endif // ETL_RPC26_INCLUDED
