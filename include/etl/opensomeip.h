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

#ifndef ETL_OPENSOMEIP_INCLUDED
#define ETL_OPENSOMEIP_INCLUDED

#include "platform.h"
#include "alignment.h"
#include "array.h"
#include "atomic.h"
#include "basic_string.h"
#include "error_handler.h"
#include "future.h"
#include "memory.h"
#include "meta.h"
#include "placement_new.h"
#include "tuple.h"
#include "type_traits.h"
#include "unaligned_type.h"
#include "vector.h"

#include <stdint.h>

#if ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION

namespace etl
{
  namespace someip
  {
    //=========================================================================
    // SOME/IP Message Type (8 bit)
    //=========================================================================
    enum message_type_t : uint8_t
    {
      REQUEST               = 0x00,
      REQUEST_NO_RETURN     = 0x01,
      NOTIFICATION          = 0x02,
      REQUEST_ACK           = 0x40, // Reserved
      REQUEST_NO_RETURN_ACK = 0x41, // Reserved
      NOTIFICATION_ACK      = 0x42, // Reserved
      RESPONSE              = 0x80,
      EXCEPTION             = 0x81,
      RESPONSE_ACK          = 0xC0, // Reserved
      EXCEPTION_ACK         = 0xC1  // Reserved
    };

    /// TP-Flag mask: bit 5 of message type indicates SOME/IP-TP segment.
    static constexpr uint8_t TP_FLAG = 0x20;

    //=========================================================================
    // SOME/IP Return Codes (8 bit)
    //=========================================================================
    enum return_code_t : uint8_t
    {
      E_OK                      = 0x00,
      E_NOT_OK                  = 0x01,
      E_UNKNOWN_SERVICE         = 0x02,
      E_UNKNOWN_METHOD          = 0x03,
      E_NOT_READY               = 0x04, // deprecated
      E_NOT_REACHABLE           = 0x05, // deprecated
      E_TIMEOUT                 = 0x06, // deprecated
      E_WRONG_PROTOCOL_VERSION  = 0x07,
      E_WRONG_INTERFACE_VERSION = 0x08,
      E_MALFORMED_MESSAGE       = 0x09,
      E_WRONG_MESSAGE_TYPE      = 0x0A
    };

    //=========================================================================
    // Well-known constants
    //=========================================================================
    static constexpr uint8_t  PROTOCOL_VERSION = 0x01;
    static constexpr uint16_t HEADER_SIZE      = 16; // bytes on wire

    /// Method ID ranges: 0x0000-0x7FFF for methods, 0x8000-0x8FFF for events.
    static constexpr uint16_t EVENT_METHOD_ID_BIT = 0x8000;

    //=========================================================================
    // Endian read/write uses etl::be_uint16_ext_t / etl::be_uint32_ext_t
    // from unaligned_type.h — no custom helpers needed.
    //=========================================================================

    //=========================================================================
    // Serialization detail
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

      template <typename T>
      struct is_primitive_serializable : etl::bool_constant<etl::is_arithmetic<T>::value || etl::is_enum<T>::value>
      {
      };

      template <typename T>
      struct is_etl_array : etl::false_type
      {
      };

      template <typename T, size_t N>
      struct is_etl_array<etl::array<T, N>> : etl::true_type
      {
      };

      template <typename T, typename = void>
      struct is_etl_vector : etl::false_type
      {
      };

      template <typename T>
      struct is_etl_vector<T, typename etl::enable_if<etl::is_base_of<etl::vector_base, T>::value>::type> : etl::true_type
      {
      };

      template <typename T, typename = void>
      struct is_etl_string : etl::false_type
      {
      };

      template <typename T>
      struct is_etl_string<T, typename etl::enable_if<etl::is_base_of<etl::string_base, T>::value>::type> : etl::true_type
      {
      };

      //=====================================================================
      // serialized_size_of
      //=====================================================================

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
          size_t total = sizeof(uint16_t);
          for (size_t i = 0; i < value.size(); ++i)
          {
            total += serialized_size_of(value[i]);
          }
          return total;
        }
        else if constexpr (is_etl_string<T>::value)
        {
          return sizeof(uint16_t) + value.size() * sizeof(typename T::value_type);
        }
        else
        {
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
      // serialize_one
      //=====================================================================

      template <typename T>
      size_t serialize_one(uint8_t* buffer, size_t offset, const T& value)
      {
        if constexpr (is_primitive_serializable<T>::value)
        {
          etl::mem_copy(reinterpret_cast<const uint8_t*>(&value), sizeof(T), buffer + offset);
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
          uint16_t len = static_cast<uint16_t>(value.size());
          etl::mem_copy(reinterpret_cast<const uint8_t*>(&len), sizeof(uint16_t), buffer + offset);
          offset += sizeof(uint16_t);
          for (size_t i = 0; i < value.size(); ++i)
          {
            offset = serialize_one(buffer, offset, value[i]);
          }
          return offset;
        }
        else if constexpr (is_etl_string<T>::value)
        {
          uint16_t len = static_cast<uint16_t>(value.size());
          etl::mem_copy(reinterpret_cast<const uint8_t*>(&len), sizeof(uint16_t), buffer + offset);
          offset += sizeof(uint16_t);
          size_t byte_count = value.size() * sizeof(typename T::value_type);
          etl::mem_copy(reinterpret_cast<const uint8_t*>(value.data()), byte_count, buffer + offset);
          return offset + byte_count;
        }
        else
        {
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
      // deserialize_one
      //=====================================================================

      template <typename T>
      size_t deserialize_one(const uint8_t* buffer, size_t offset, T& value)
      {
        if constexpr (is_primitive_serializable<T>::value)
        {
          etl::mem_copy(buffer + offset, sizeof(T), reinterpret_cast<uint8_t*>(&value));
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
          uint16_t len = 0;
          etl::mem_copy(buffer + offset, sizeof(uint16_t), reinterpret_cast<uint8_t*>(&len));
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
          uint16_t len = 0;
          etl::mem_copy(buffer + offset, sizeof(uint16_t), reinterpret_cast<uint8_t*>(&len));
          offset += sizeof(uint16_t);
          value.clear();
          size_t byte_count = len * sizeof(typename T::value_type);
          value.assign(reinterpret_cast<const typename T::value_type*>(buffer + offset), len);
          return offset + byte_count;
        }
        else
        {
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
      /// serialize_args_impl / deserialize_args
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

      template <typename Tuple, size_t... Is>
      size_t deserialize_tuple_impl([[maybe_unused]] const uint8_t* buffer, size_t offset, [[maybe_unused]] Tuple& t, etl::index_sequence<Is...>)
      {
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

      //=====================================================================
      // Reflection helpers
      //=====================================================================

      consteval bool is_service_method(etl::meta::info fn)
      {
        return etl::meta::is_function(fn) && etl::meta::is_public(fn) && !etl::meta::is_special_member_function(fn) && !etl::meta::is_constructor(fn)
               && !etl::meta::is_destructor(fn);
      }

      consteval uint16_t method_index_of(etl::meta::info fn)
      {
        auto     parent  = etl::meta::parent_of(fn);
        auto     members = etl::meta::members_of(parent, etl::meta::access_context::unchecked());
        uint16_t idx     = 0;
        for (auto m : members)
        {
          if (m == fn)
            return idx;
          if (is_service_method(m))
            ++idx;
        }
        return idx;
      }

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

    } // namespace detail

    //*********************************************************************
    /// Public API: serialize argument pack into buffer.
    //*********************************************************************
    template <typename... Args>
    size_t serialize_args(uint8_t* buffer, const Args&... args)
    {
      return detail::serialize_args_impl(buffer, 0, args...);
    }

    //=========================================================================
    // SOME/IP Header — 16 bytes on the wire (big-endian).
    //
    // Wire layout (per spec feat_req_someip_45):
    //   [0..1]  Service ID      (uint16, big-endian)
    //   [2..3]  Method ID       (uint16, big-endian)
    //   [4..7]  Length           (uint32, big-endian) — from Request ID to end
    //   [8..9]  Client ID       (uint16, big-endian)
    //   [10..11] Session ID     (uint16, big-endian)
    //   [12]    Protocol Version (uint8, always 0x01)
    //   [13]    Interface Version (uint8)
    //   [14]    Message Type    (uint8)
    //   [15]    Return Code     (uint8)
    //=========================================================================
    struct someip_header
    {
      uint16_t       service_id;
      uint16_t       method_id;
      uint32_t       length; ///< Bytes from client_id to end of payload (8 + payload_size).
      uint16_t       client_id;
      uint16_t       session_id;
      uint8_t        protocol_version;
      uint8_t        interface_version;
      message_type_t message_type;
      return_code_t  return_code;

      someip_header()
        : service_id(0)
        , method_id(0)
        , length(8) // minimum: 8 bytes for the fields after Length
        , client_id(0)
        , session_id(0)
        , protocol_version(PROTOCOL_VERSION)
        , interface_version(0)
        , message_type(REQUEST)
        , return_code(E_OK)
      {
      }

      /// Payload size derived from the Length field.
      uint32_t payload_size() const
      {
        return (length > 8) ? (length - 8) : 0;
      }

      /// Set Length from a payload size.
      void set_payload_size(uint32_t ps)
      {
        length = 8 + ps;
      }

      /// Encode header into 16-byte big-endian buffer.
      void encode(uint8_t* buf) const
      {
        etl::be_uint16_ext_t be16(buf + 0);
        be16 = service_id;
        etl::be_uint16_ext_t be16b(buf + 2);
        be16b = method_id;
        etl::be_uint32_ext_t be32(buf + 4);
        be32 = length;
        etl::be_uint16_ext_t be16c(buf + 8);
        be16c = client_id;
        etl::be_uint16_ext_t be16d(buf + 10);
        be16d   = session_id;
        buf[12] = protocol_version;
        buf[13] = interface_version;
        buf[14] = static_cast<uint8_t>(message_type);
        buf[15] = static_cast<uint8_t>(return_code);
      }

      /// Decode header from 16-byte big-endian buffer.
      void decode(const uint8_t* buf)
      {
        etl::be_uint16_ext_t be16a(const_cast<uint8_t*>(buf + 0));
        service_id = be16a.value();
        etl::be_uint16_ext_t be16b(const_cast<uint8_t*>(buf + 2));
        method_id = be16b.value();
        etl::be_uint32_ext_t be32(const_cast<uint8_t*>(buf + 4));
        length = be32.value();
        etl::be_uint16_ext_t be16c(const_cast<uint8_t*>(buf + 8));
        client_id = be16c.value();
        etl::be_uint16_ext_t be16d(const_cast<uint8_t*>(buf + 10));
        session_id        = be16d.value();
        protocol_version  = buf[12];
        interface_version = buf[13];
        message_type      = static_cast<message_type_t>(buf[14]);
        return_code       = static_cast<return_code_t>(buf[15]);
      }
    };

    //=========================================================================
    // someip_message — header + payload buffer.
    //
    // MaxPayload is the maximum payload size in bytes (e.g. 1400 for UDP).
    //=========================================================================
    template <size_t MaxPayload>
    struct someip_message
    {
      someip_header                   header;
      uint16_t                        payload_used; ///< Actual bytes in payload.
      etl::array<uint8_t, MaxPayload> payload;

      someip_message()
        : payload_used(0)
      {
      }

      /// Total wire size: 16-byte header + payload.
      size_t wire_size() const
      {
        return HEADER_SIZE + payload_used;
      }

      /// Encode entire message (header + payload) to wire buffer.
      /// Returns bytes written. buf must have room for wire_size().
      size_t encode(uint8_t* buf) const
      {
        someip_header h = header;
        h.set_payload_size(payload_used);
        h.encode(buf);
        if (payload_used > 0)
        {
          etl::mem_copy(payload.data(), payload_used, buf + HEADER_SIZE);
        }
        return HEADER_SIZE + payload_used;
      }

      /// Decode from wire buffer of 'len' bytes.
      /// Returns true if header is valid (length >= 8).
      bool decode(const uint8_t* buf, size_t len)
      {
        if (len < HEADER_SIZE)
          return false;
        header.decode(buf);
        if (header.length < 8)
          return false;
        uint32_t ps = header.payload_size();
        if (ps > MaxPayload)
          return false;
        if (HEADER_SIZE + ps > len)
          return false;
        payload_used = static_cast<uint16_t>(ps);
        if (payload_used > 0)
        {
          etl::mem_copy(buf + HEADER_SIZE, payload_used, payload.data());
        }
        return true;
      }

      /// Helper: build a response header from a request.
      void build_response_from(const someip_message& request, return_code_t rc = E_OK)
      {
        header.service_id        = request.header.service_id;
        header.method_id         = request.header.method_id;
        header.client_id         = request.header.client_id;
        header.session_id        = request.header.session_id;
        header.protocol_version  = PROTOCOL_VERSION;
        header.interface_version = request.header.interface_version;
        header.message_type      = (rc == E_OK) ? RESPONSE : EXCEPTION;
        header.return_code       = rc;
      }

      /// Helper: build an error response (no payload).
      void build_error_from(const someip_message& request, return_code_t rc)
      {
        build_response_from(request, rc);
        payload_used = 0;
        header.set_payload_size(0);
      }
    };

    //=========================================================================
    // Transport abstraction.
    //
    // i_transport is the interface that a TCP/UDP stack (e.g. LWIP) would
    // implement. For testing, loopback_transport provides an in-memory
    // buffer that connects a client and server directly.
    //=========================================================================
    class i_transport
    {
    public:

      virtual ~i_transport() {}

      /// Send raw bytes. Returns true on success.
      virtual bool send(const uint8_t* data, size_t length) = 0;

      /// Receive raw bytes into buffer. Sets 'received' to actual bytes read.
      /// Returns true if data was available, false if nothing to read.
      virtual bool receive(uint8_t* buffer, size_t max_length, size_t& received) = 0;
    };

    //=========================================================================
    // loopback_transport — in-memory loopback for testing.
    //
    // Data sent via send() is stored in a ring buffer and can be read via
    // receive(). MaxBuf is the maximum bytes stored.
    //=========================================================================
    template <size_t MaxBuf = 4096>
    class loopback_transport : public i_transport
    {
    public:

      loopback_transport()
        : head_(0)
        , tail_(0)
      {
      }

      bool send(const uint8_t* data, size_t length) override
      {
        // Simple linear buffer (not circular — sufficient for unit tests).
        if (tail_ + length > MaxBuf)
          return false;
        etl::mem_copy(data, length, buf_.data() + tail_);
        tail_ += length;
        return true;
      }

      bool receive(uint8_t* buffer, size_t max_length, size_t& received) override
      {
        size_t avail = tail_ - head_;
        if (avail == 0)
        {
          received = 0;
          return false;
        }
        received = (avail < max_length) ? avail : max_length;
        etl::mem_copy(buf_.data() + head_, received, buffer);
        head_ += received;
        // Reset when drained.
        if (head_ == tail_)
        {
          head_ = 0;
          tail_ = 0;
        }
        return true;
      }

      /// Number of bytes available to read.
      size_t available() const
      {
        return tail_ - head_;
      }

      /// Reset the buffer.
      void reset()
      {
        head_ = 0;
        tail_ = 0;
      }

    private:

      etl::array<uint8_t, MaxBuf> buf_;
      size_t                      head_;
      size_t                      tail_;
    };

    //=========================================================================
    // Session ID generator — wraps 0x0001..0xFFFF per spec.
    //=========================================================================
    class session_id_generator
    {
    public:

      session_id_generator()
        : next_(1)
      {
      }

      uint16_t next()
      {
        uint16_t id = next_;
        next_       = (next_ == 0xFFFF) ? static_cast<uint16_t>(1) : static_cast<uint16_t>(next_ + 1);
        return id;
      }

    private:

      uint16_t next_;
    };

    //=========================================================================
    // someip_server — receives SOME/IP messages from transport, dispatches
    // to reflected methods on ServiceImpl, sends SOME/IP responses back.
    //
    // Template parameters:
    //   ServiceImpl  — user class whose public methods are the service API.
    //   MaxPayload   — max SOME/IP payload size in bytes.
    //=========================================================================
    template <typename ServiceImpl, size_t MaxPayload>
    class someip_server
    {
    public:

      using message_type = someip_message<MaxPayload>;

      someip_server(i_transport& transport, ServiceImpl& impl, uint16_t service_id, uint8_t interface_version = 1)
        : transport_(transport)
        , impl_(impl)
        , service_id_(service_id)
        , interface_version_(interface_version)
      {
      }

      //*********************************************************************
      /// Process one incoming SOME/IP message from the transport.
      /// Returns true if a message was processed.
      //*********************************************************************
      bool process()
      {
        etl::array<uint8_t, HEADER_SIZE + MaxPayload> wire_buf = {};
        size_t                                        received = 0;

        if (!transport_.receive(wire_buf.data(), wire_buf.size(), received))
          return false;

        message_type request;
        if (!request.decode(wire_buf.data(), received))
          return false; // Silently drop malformed (length < 8).

        // Validate protocol version.
        if (request.header.protocol_version != PROTOCOL_VERSION)
        {
          send_error(request, E_WRONG_PROTOCOL_VERSION);
          return true;
        }

        // Validate interface version.
        if (request.header.interface_version != interface_version_)
        {
          send_error(request, E_WRONG_INTERFACE_VERSION);
          return true;
        }

        // Validate service ID.
        if (request.header.service_id != service_id_)
        {
          send_error(request, E_UNKNOWN_SERVICE);
          return true;
        }

        // Validate message type (only REQUEST and REQUEST_NO_RETURN accepted).
        if (request.header.message_type != REQUEST && request.header.message_type != REQUEST_NO_RETURN)
        {
          send_error(request, E_WRONG_MESSAGE_TYPE);
          return true;
        }

        // Dispatch by method_id via reflection.
        message_type response;
        bool         dispatched = dispatch(request, response);

        if (!dispatched)
        {
          send_error(request, E_UNKNOWN_METHOD);
          return true;
        }

        // Fire & forget: no response sent.
        if (request.header.message_type == REQUEST_NO_RETURN)
          return true;

        // Send response.
        response.build_response_from(request, E_OK);
        response.header.set_payload_size(response.payload_used);

        etl::array<uint8_t, HEADER_SIZE + MaxPayload> resp_buf;
        size_t                                        resp_len = response.encode(resp_buf.data());
        transport_.send(resp_buf.data(), resp_len);

        return true;
      }

      //*********************************************************************
      /// Publish a notification event to transport.
      //*********************************************************************
      template <typename T>
      void notify(uint16_t method_id, const T& value, uint8_t iface_ver = 0)
      {
        message_type msg;
        msg.header.service_id        = service_id_;
        msg.header.method_id         = method_id | EVENT_METHOD_ID_BIT;
        msg.header.client_id         = 0;
        msg.header.session_id        = session_gen_.next();
        msg.header.protocol_version  = PROTOCOL_VERSION;
        msg.header.interface_version = (iface_ver != 0) ? iface_ver : interface_version_;
        msg.header.message_type      = NOTIFICATION;
        msg.header.return_code       = E_OK;

        msg.payload_used = static_cast<uint16_t>(detail::serialize_one(msg.payload.data(), 0, value));
        msg.header.set_payload_size(msg.payload_used);

        etl::array<uint8_t, HEADER_SIZE + MaxPayload> wire_buf = {};
        size_t                                        len = msg.encode(wire_buf.data());
        transport_.send(wire_buf.data(), len);
      }

    private:

      //*********************************************************************
      /// Dispatch: iterate reflected methods, match by method_id.
      //*********************************************************************
      bool dispatch(const message_type& request, message_type& response)
      {
        uint16_t target_id = request.header.method_id;
        uint16_t idx       = 0;
        bool     found     = false;

#if ETL_META_NEEDS_INFO_ARRAY
        static constexpr auto _n = etl::meta::members_of(^^ServiceImpl, etl::meta::access_context::unchecked()).size();
        static constexpr auto _a = etl::meta::to_info_array<_n>(etl::meta::members_of(^^ServiceImpl, etl::meta::access_context::unchecked()));
        template for (constexpr auto fn : _a)
#else
        constexpr auto all_members = etl::meta::members_of(^^ServiceImpl, etl::meta::access_context::unchecked());
        template for (constexpr auto fn : all_members)
#endif
        {
          if constexpr (detail::is_service_method(fn))
          {
            if (!found && idx == target_id)
            {
              invoke_method<fn>(request, response);
              found = true;
            }
            ++idx;
          }
        }
        return found;
      }

      //*********************************************************************
      /// Invoke a single reflected method, deserializing args from request
      /// payload and serializing result into response payload.
      //*********************************************************************
      template <etl::meta::info Fn>
      void invoke_method(const message_type& request, message_type& response)
      {
#if ETL_META_NEEDS_INFO_ARRAY
        static constexpr auto _pc = etl::meta::parameters_of(Fn).size();
        static constexpr auto params = etl::meta::to_info_array<_pc>(etl::meta::parameters_of(Fn));
#else
        constexpr auto params = etl::meta::parameters_of(Fn);
#endif
        invoke_method_impl<Fn>(request, response, etl::make_index_sequence<params.size()>{});
      }

      template <etl::meta::info Fn, size_t... Is>
      void invoke_method_impl(const message_type& request, message_type& response, etl::index_sequence<Is...>)
      {
#if ETL_META_NEEDS_INFO_ARRAY
        static constexpr auto _pc = etl::meta::parameters_of(Fn).size();
        [[maybe_unused]] static constexpr auto params = etl::meta::to_info_array<_pc>(etl::meta::parameters_of(Fn));
#else
        [[maybe_unused]] constexpr auto params = etl::meta::parameters_of(Fn);
#endif
        using ReturnType      = typename[:etl::meta::return_type_of(Fn):];

        auto args = detail::deserialize_args< typename[:etl::meta::type_of(params[Is]):]...>(request.payload.data());

        if constexpr (etl::is_void_v<ReturnType>)
        {
          detail::apply_tuple([this](auto&... a) { (impl_.[:Fn:])(a...); }, args);
          response.payload_used = 0;
        }
        else
        {
          ReturnType result     = detail::apply_tuple([this](auto&... a) -> ReturnType { return (impl_.[:Fn:])(a...); }, args);
          response.payload_used = static_cast<uint16_t>(detail::serialize_one(response.payload.data(), 0, result));
        }
      }

      void send_error(const message_type& request, return_code_t rc)
      {
        // Only respond with error to REQUEST, not to REQUEST_NO_RETURN.
        if (request.header.message_type == REQUEST_NO_RETURN)
          return;
        if (request.header.message_type != REQUEST)
          return;

        message_type err;
        err.build_error_from(request, rc);

        etl::array<uint8_t, HEADER_SIZE + MaxPayload> wire_buf = {};
        size_t                                        len = err.encode(wire_buf.data());
        transport_.send(wire_buf.data(), len);
      }

      i_transport&         transport_;
      ServiceImpl&         impl_;
      uint16_t             service_id_;
      uint8_t              interface_version_;
      session_id_generator session_gen_;
    };

    //=========================================================================
    // someip_client — builds SOME/IP requests via C++26 reflection stubs,
    // sends them over transport, and correlates responses by session_id.
    //
    // Template parameters:
    //   ServiceInterface — class whose public methods define the RPC API.
    //   MaxPayload       — max SOME/IP payload size in bytes.
    //   MaxPending       — max number of in-flight async calls.
    //=========================================================================
    template <typename ServiceInterface, size_t MaxPayload, size_t MaxPending = 8>
    class someip_client
    {
    public:

      using message_type = someip_message<MaxPayload>;

    private:

      static constexpr size_t MAX_RESULT_SIZE  = MaxPayload;
      static constexpr size_t MAX_RESULT_ALIGN = etl::alignment_of<uint64_t>::value;

      static constexpr size_t STATE_STORAGE_SIZE =
        sizeof(typename etl::aligned_storage<MAX_RESULT_SIZE, MAX_RESULT_ALIGN>::type) + sizeof(etl::atomic_bool) + sizeof(void*); // padding headroom

      //*********************************************************************
      /// A single pending-call slot. Embeds storage for shared_state<T>.
      //*********************************************************************
      struct pending_slot
      {
        uint16_t session_id;
        bool     in_use;

        /// Type-erased fulfil: deserialize response payload, call set_value.
        void (*fulfil_fn)(void* state_ptr, const uint8_t* payload, uint16_t payload_size);

        /// Type-erased destroy: destruct the shared_state<T> in place.
        void (*destroy_fn)(void* state_ptr);

        /// Raw storage for the embedded shared_state<ReturnType>.
        typename etl::aligned_storage<STATE_STORAGE_SIZE, MAX_RESULT_ALIGN>::type state_storage;

        pending_slot()
          : session_id(0)
          , in_use(false)
          , fulfil_fn(nullptr)
          , destroy_fn(nullptr)
        {
        }

        void* state_ptr()
        {
          return &state_storage;
        }
      };

    public:

      someip_client(i_transport& transport, uint16_t service_id, uint16_t client_id = 0, uint8_t interface_version = 1)
        : transport_(transport)
        , service_id_(service_id)
        , client_id_(client_id)
        , interface_version_(interface_version)
      {
      }

      ~someip_client()
      {
        for (size_t i = 0; i < MaxPending; ++i)
        {
          if (pending_[i].destroy_fn != nullptr)
          {
            pending_[i].destroy_fn(pending_[i].state_ptr());
          }
        }
      }

      //*********************************************************************
      /// Invoke a remote method by reflection. Returns etl::future<ReturnType>.
      ///
      /// Usage:  auto fut = client.call<^^MyService::add>(3, 4);
      //*********************************************************************
      template <etl::meta::info Fn, typename... ArgTypes>
      auto call(const ArgTypes&... args) -> etl::future<typename[:etl::meta::return_type_of(Fn):]>
      {
        using ReturnType             = typename[:etl::meta::return_type_of(Fn):];
        constexpr uint16_t method_id = detail::method_index_of(Fn);

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
          return etl::future<ReturnType>(); // All slots busy.
        }

        // Destroy any previous shared_state in this slot.
        if (slot->destroy_fn != nullptr)
        {
          slot->destroy_fn(slot->state_ptr());
          slot->destroy_fn = nullptr;
        }

        // Construct shared_state<ReturnType> in-place.
        auto* state = ::new (slot->state_ptr()) etl::shared_state<ReturnType>();

        // Build SOME/IP request message.
        message_type msg;
        msg.header.service_id        = service_id_;
        msg.header.method_id         = method_id;
        msg.header.client_id         = client_id_;
        msg.header.session_id        = session_gen_.next();
        msg.header.protocol_version  = PROTOCOL_VERSION;
        msg.header.interface_version = interface_version_;
        msg.header.message_type      = REQUEST;
        msg.header.return_code       = E_OK;
        msg.payload_used             = static_cast<uint16_t>(serialize_args(msg.payload.data(), args...));
        msg.header.set_payload_size(msg.payload_used);

        // Set up slot.
        slot->session_id = msg.header.session_id;
        slot->in_use     = true;

        slot->destroy_fn = [](void* sp)
        {
          static_cast<etl::shared_state<ReturnType>*>(sp)->~shared_state();
        };

        if constexpr (etl::is_void_v<ReturnType>)
        {
          slot->fulfil_fn = [](void* sp, const uint8_t*, uint16_t)
          {
            static_cast<etl::shared_state<void>*>(sp)->set_value();
          };
        }
        else
        {
          slot->fulfil_fn = [](void* sp, const uint8_t* payload, uint16_t)
          {
            ReturnType value{};
            detail::deserialize_one(payload, 0, value);
            static_cast<etl::shared_state<ReturnType>*>(sp)->set_value(value);
          };
        }

        // Send over transport.
        etl::array<uint8_t, HEADER_SIZE + MaxPayload> wire_buf = {};
        size_t                                        len = msg.encode(wire_buf.data());
        transport_.send(wire_buf.data(), len);

        return etl::future<ReturnType>(*state);
      }

      //*********************************************************************
      /// Fire & forget: send REQUEST_NO_RETURN, no response expected.
      //*********************************************************************
      template <etl::meta::info Fn, typename... ArgTypes>
      bool fire_and_forget(const ArgTypes&... args)
      {
        constexpr uint16_t method_id = detail::method_index_of(Fn);

        message_type msg;
        msg.header.service_id        = service_id_;
        msg.header.method_id         = method_id;
        msg.header.client_id         = client_id_;
        msg.header.session_id        = 0; // optional for fire&forget
        msg.header.protocol_version  = PROTOCOL_VERSION;
        msg.header.interface_version = interface_version_;
        msg.header.message_type      = REQUEST_NO_RETURN;
        msg.header.return_code       = E_OK;
        msg.payload_used             = static_cast<uint16_t>(serialize_args(msg.payload.data(), args...));
        msg.header.set_payload_size(msg.payload_used);

        etl::array<uint8_t, HEADER_SIZE + MaxPayload> wire_buf = {};
        size_t                                        len = msg.encode(wire_buf.data());
        return transport_.send(wire_buf.data(), len);
      }

      //*********************************************************************
      /// Poll for responses from the server.
      /// Returns true if a response was processed.
      //*********************************************************************
      bool poll()
      {
        etl::array<uint8_t, HEADER_SIZE + MaxPayload> wire_buf = {};
        size_t                                        received = 0;

        if (!transport_.receive(wire_buf.data(), wire_buf.size(), received))
          return false;

        message_type response;
        if (!response.decode(wire_buf.data(), received))
          return false;

        // Match by session_id.
        last_return_code_ = response.header.return_code;
        for (size_t i = 0; i < MaxPending; ++i)
        {
          if (pending_[i].in_use && pending_[i].session_id == response.header.session_id)
          {
            pending_[i].fulfil_fn(pending_[i].state_ptr(), response.payload.data(), response.payload_used);
            pending_[i].in_use = false;
            return true;
          }
        }

        // Orphan response — discard.
        return false;
      }

      //*********************************************************************
      /// Get the return code from the last polled response (for error check).
      /// Call after poll() returns true if you need to inspect error codes.
      //*********************************************************************
      return_code_t last_return_code() const
      {
        return last_return_code_;
      }

      //*********************************************************************
      /// Receive a notification and deserialize into T.
      /// Returns true if a notification was received.
      //*********************************************************************
      template <typename T>
      bool receive_notification(T& value, uint16_t& method_id_out)
      {
        etl::array<uint8_t, HEADER_SIZE + MaxPayload> wire_buf = {};
        size_t                                        received = 0;

        if (!transport_.receive(wire_buf.data(), wire_buf.size(), received))
          return false;

        message_type msg;
        if (!msg.decode(wire_buf.data(), received))
          return false;

        if (msg.header.message_type != NOTIFICATION)
          return false;

        method_id_out = msg.header.method_id;
        detail::deserialize_one(msg.payload.data(), 0, value);
        return true;
      }

    private:

      i_transport&                         transport_;
      uint16_t                             service_id_;
      uint16_t                             client_id_;
      uint8_t                              interface_version_;
      session_id_generator                 session_gen_;
      etl::array<pending_slot, MaxPending> pending_;
      return_code_t                        last_return_code_ = E_OK;
    };

    //=========================================================================
    //
    //  SOME/IP-SD  (Service Discovery)
    //
    //  Per Open SOME/IP Specification — someip-sd.rst
    //
    //=========================================================================

    //=========================================================================
    // SD well-known identifiers
    //=========================================================================
    static constexpr uint16_t SD_SERVICE_ID        = 0xFFFF;
    static constexpr uint16_t SD_METHOD_ID         = 0x8100;
    static constexpr uint8_t  SD_INTERFACE_VERSION = 0x01;
    static constexpr uint16_t SD_CLIENT_ID         = 0x0000;

    //=========================================================================
    // SD Entry Types (uint8)
    //=========================================================================
    enum sd_entry_type_t : uint8_t
    {
      SD_FIND_SERVICE             = 0x00,
      SD_OFFER_SERVICE            = 0x01,
      SD_SUBSCRIBE_EVENTGROUP     = 0x06,
      SD_SUBSCRIBE_EVENTGROUP_ACK = 0x07
    };

    /// StopOffer is an OfferService with TTL=0.
    /// StopSubscribe is a SubscribeEventgroup with TTL=0.
    /// SubscribeNack is a SubscribeEventgroupAck with TTL=0.

    //=========================================================================
    // SD Option Types (uint8)
    //=========================================================================
    enum sd_option_type_t : uint8_t
    {
      SD_OPT_CONFIGURATION    = 0x01,
      SD_OPT_LOAD_BALANCING   = 0x02, // informational
      SD_OPT_IPV4_ENDPOINT    = 0x04,
      SD_OPT_IPV6_ENDPOINT    = 0x06,
      SD_OPT_IPV4_MULTICAST   = 0x14,
      SD_OPT_IPV6_MULTICAST   = 0x16,
      SD_OPT_IPV4_SD_ENDPOINT = 0x24,
      SD_OPT_IPV6_SD_ENDPOINT = 0x26,
      SD_OPT_MAC_GROUPCAST    = 0x15
    };

    //=========================================================================
    // L4 Protocol codes (per IANA)
    //=========================================================================
    enum sd_l4_protocol_t : uint8_t
    {
      SD_L4_TCP = 0x06,
      SD_L4_UDP = 0x11
    };

    //=========================================================================
    // SD Flags byte — bit layout of the first byte of the SD header.
    //=========================================================================
    static constexpr uint8_t SD_FLAG_REBOOT             = 0x80; ///< Bit 7: Reboot flag
    static constexpr uint8_t SD_FLAG_UNICAST            = 0x40; ///< Bit 6: Unicast support
    static constexpr uint8_t SD_FLAG_EXPLICIT_INIT_DATA = 0x20; ///< Bit 5: Explicit Initial Data Control

    /// TTL value meaning "valid until next reboot".
    static constexpr uint32_t SD_TTL_FOREVER = 0x00FFFFFF;

    /// Instance ID wildcard = all instances.
    static constexpr uint16_t SD_INSTANCE_ANY = 0xFFFF;

    /// Major version wildcard.
    static constexpr uint8_t SD_MAJOR_VERSION_ANY = 0xFF;

    /// Minor version wildcard.
    static constexpr uint32_t SD_MINOR_VERSION_ANY = 0xFFFFFFFF;

    //=========================================================================
    // sd_entry — 16-byte on-wire SD entry (service or eventgroup).
    //
    //  Bytes 0      : type
    //  Bytes 1      : index 1st options
    //  Bytes 2      : index 2nd options
    //  Bytes 3      : number of opt 1 (hi nibble) | number of opt 2 (lo nibble)
    //  Bytes 4-5    : service ID
    //  Bytes 6-7    : instance ID
    //  Bytes 8      : major version
    //  Bytes 9-11   : TTL (24-bit)
    //  Bytes 12-15  : minor version (service) or counter/eventgroup (eventgroup)
    //=========================================================================
    static constexpr size_t SD_ENTRY_SIZE = 16;

    struct sd_entry
    {
      sd_entry_type_t type;
      uint8_t         index_1st_options;
      uint8_t         index_2nd_options;
      uint8_t         num_opt_1; ///< number of options run 1
      uint8_t         num_opt_2; ///< number of options run 2
      uint16_t        service_id;
      uint16_t        instance_id;
      uint8_t         major_version;
      uint32_t        ttl; ///< 24-bit on wire

      // --- Service entry fields (FIND / OFFER) ---
      uint32_t minor_version;

      // --- Eventgroup entry fields (SUBSCRIBE / ACK) ---
      uint8_t  counter; ///< 4-bit on wire
      uint16_t eventgroup_id;

      sd_entry()
        : type(SD_FIND_SERVICE)
        , index_1st_options(0)
        , index_2nd_options(0)
        , num_opt_1(0)
        , num_opt_2(0)
        , service_id(0)
        , instance_id(0)
        , major_version(0)
        , ttl(0)
        , minor_version(0)
        , counter(0)
        , eventgroup_id(0)
      {
      }

      //-----------------------------------------------------------------------
      /// Encode this entry into 16 bytes at \p dest. Returns dest + 16.
      //-----------------------------------------------------------------------
      uint8_t* encode(uint8_t* dest) const
      {
        dest[0] = static_cast<uint8_t>(type);
        dest[1] = index_1st_options;
        dest[2] = index_2nd_options;
        dest[3] = static_cast<uint8_t>(((num_opt_1 & 0x0F) << 4) | (num_opt_2 & 0x0F));

        etl::be_uint16_ext_t sid(dest + 4);
        sid = service_id;

        etl::be_uint16_ext_t iid(dest + 6);
        iid = instance_id;

        // Byte 8: major version
        // Bytes 9-11: TTL (24-bit big-endian)
        uint32_t             ver_ttl = (static_cast<uint32_t>(major_version) << 24) | (ttl & 0x00FFFFFF);
        etl::be_uint32_ext_t vt(dest + 8);
        vt = ver_ttl;

        if (type == SD_FIND_SERVICE || type == SD_OFFER_SERVICE)
        {
          etl::be_uint32_ext_t mv(dest + 12);
          mv = minor_version;
        }
        else
        {
          // Eventgroup entry: byte 12 reserved, byte 13 hi-nibble = counter (4 bit),
          // bytes 14-15 = eventgroup ID
          dest[12] = 0;
          dest[13] = static_cast<uint8_t>((counter & 0x0F) << 4);

          etl::be_uint16_ext_t eg(dest + 14);
          eg = eventgroup_id;
        }

        return dest + SD_ENTRY_SIZE;
      }

      //-----------------------------------------------------------------------
      /// Decode from 16 bytes at \p src. Returns src + 16.
      //-----------------------------------------------------------------------
      const uint8_t* decode(const uint8_t* src)
      {
        type              = static_cast<sd_entry_type_t>(src[0]);
        index_1st_options = src[1];
        index_2nd_options = src[2];
        num_opt_1         = static_cast<uint8_t>((src[3] >> 4) & 0x0F);
        num_opt_2         = static_cast<uint8_t>(src[3] & 0x0F);

        etl::be_uint16_ext_t sid(const_cast<uint8_t*>(src + 4));
        service_id = static_cast<uint16_t>(sid);

        etl::be_uint16_ext_t iid(const_cast<uint8_t*>(src + 6));
        instance_id = static_cast<uint16_t>(iid);

        etl::be_uint32_ext_t vt(const_cast<uint8_t*>(src + 8));
        uint32_t             ver_ttl = static_cast<uint32_t>(vt);
        major_version                = static_cast<uint8_t>(ver_ttl >> 24);
        ttl                          = ver_ttl & 0x00FFFFFF;

        if (type == SD_FIND_SERVICE || type == SD_OFFER_SERVICE)
        {
          etl::be_uint32_ext_t mv(const_cast<uint8_t*>(src + 12));
          minor_version = static_cast<uint32_t>(mv);
          counter       = 0;
          eventgroup_id = 0;
        }
        else
        {
          minor_version = 0;
          counter       = static_cast<uint8_t>((src[13] >> 4) & 0x0F);

          etl::be_uint16_ext_t eg(const_cast<uint8_t*>(src + 14));
          eventgroup_id = static_cast<uint16_t>(eg);
        }

        return src + SD_ENTRY_SIZE;
      }

      /// True if this is a StopOffer / StopSubscribe / Nack (TTL == 0).
      bool is_stop() const
      {
        return ttl == 0;
      }
    };

    //=========================================================================
    // sd_ipv4_option — 12-byte IPv4 endpoint / multicast / SD-endpoint option.
    //
    //  Bytes 0-1  : length (always 0x0009 for IPv4 options)
    //  Byte  2    : type
    //  Byte  3    : reserved (0x00)
    //  Bytes 4-7  : IPv4 address (network byte order)
    //  Byte  8    : reserved (0x00)
    //  Byte  9    : L4 protocol
    //  Bytes 10-11: port
    //=========================================================================
    static constexpr size_t SD_IPV4_OPTION_SIZE = 12;

    struct sd_ipv4_option
    {
      sd_option_type_t       type;
      etl::array<uint8_t, 4> ipv4_address;
      sd_l4_protocol_t       protocol;
      uint16_t               port;

      sd_ipv4_option()
        : type(SD_OPT_IPV4_ENDPOINT)
        , ipv4_address{}
        , protocol(SD_L4_UDP)
        , port(0)
      {
      }

      uint8_t* encode(uint8_t* dest) const
      {
        // length = 0x0009
        etl::be_uint16_ext_t len(dest);
        len = static_cast<uint16_t>(0x0009);

        dest[2] = static_cast<uint8_t>(type);
        dest[3] = 0x00; // reserved

        etl::mem_copy(ipv4_address.data(), ipv4_address.data() + 4, dest + 4);

        dest[8] = 0x00; // reserved
        dest[9] = static_cast<uint8_t>(protocol);

        etl::be_uint16_ext_t p(dest + 10);
        p = port;

        return dest + SD_IPV4_OPTION_SIZE;
      }

      const uint8_t* decode(const uint8_t* src)
      {
        // skip length (bytes 0-1)
        type = static_cast<sd_option_type_t>(src[2]);
        // skip reserved byte 3

        etl::mem_copy(src + 4, src + 8, ipv4_address.data());

        // skip reserved byte 8
        protocol = static_cast<sd_l4_protocol_t>(src[9]);

        etl::be_uint16_ext_t p(const_cast<uint8_t*>(src + 10));
        port = static_cast<uint16_t>(p);

        return src + SD_IPV4_OPTION_SIZE;
      }
    };

    //=========================================================================
    // sd_ipv6_option — 24-byte IPv6 endpoint / multicast / SD-endpoint option.
    //
    //  Bytes 0-1  : length (always 0x0015 for IPv6 options)
    //  Byte  2    : type
    //  Byte  3    : reserved
    //  Bytes 4-19 : IPv6 address (16 bytes, network byte order)
    //  Byte  20   : reserved
    //  Byte  21   : L4 protocol
    //  Bytes 22-23: port
    //=========================================================================
    static constexpr size_t SD_IPV6_OPTION_SIZE = 24;

    struct sd_ipv6_option
    {
      sd_option_type_t        type;
      etl::array<uint8_t, 16> ipv6_address;
      sd_l4_protocol_t        protocol;
      uint16_t                port;

      sd_ipv6_option()
        : type(SD_OPT_IPV6_ENDPOINT)
        , ipv6_address{}
        , protocol(SD_L4_UDP)
        , port(0)
      {
      }

      uint8_t* encode(uint8_t* dest) const
      {
        etl::be_uint16_ext_t len(dest);
        len = static_cast<uint16_t>(0x0015);

        dest[2] = static_cast<uint8_t>(type);
        dest[3] = 0x00;

        etl::mem_copy(ipv6_address.data(), ipv6_address.data() + 16, dest + 4);

        dest[20] = 0x00;
        dest[21] = static_cast<uint8_t>(protocol);

        etl::be_uint16_ext_t p(dest + 22);
        p = port;

        return dest + SD_IPV6_OPTION_SIZE;
      }

      const uint8_t* decode(const uint8_t* src)
      {
        type = static_cast<sd_option_type_t>(src[2]);

        etl::mem_copy(src + 4, src + 20, ipv6_address.data());

        protocol = static_cast<sd_l4_protocol_t>(src[21]);

        etl::be_uint16_ext_t p(const_cast<uint8_t*>(src + 22));
        port = static_cast<uint16_t>(p);

        return src + SD_IPV6_OPTION_SIZE;
      }
    };

    //=========================================================================
    // sd_configuration_option — variable-length key=value configuration.
    //
    //  Bytes 0-1  : length (N + 1, where N is the config string length)
    //  Byte  2    : type (0x01)
    //  Byte  3    : reserved
    //  Bytes 4..  : configuration string (length-prefixed DNS-TXT-like entries)
    //=========================================================================
    template <size_t MaxConfigLen = 64>
    struct sd_configuration_option
    {
      etl::array<uint8_t, MaxConfigLen> data;
      size_t                            length;

      sd_configuration_option()
        : data{}
        , length(0)
      {
      }

      size_t wire_size() const
      {
        return 4 + length;
      }

      uint8_t* encode(uint8_t* dest) const
      {
        uint16_t             opt_len = static_cast<uint16_t>(length + 1);
        etl::be_uint16_ext_t len(dest);
        len = opt_len;

        dest[2] = static_cast<uint8_t>(SD_OPT_CONFIGURATION);
        dest[3] = 0x00;

        etl::mem_copy(data.data(), data.data() + length, dest + 4);

        return dest + wire_size();
      }

      const uint8_t* decode(const uint8_t* src, size_t available)
      {
        (void)available;
        etl::be_uint16_ext_t len(const_cast<uint8_t*>(src));
        uint16_t             opt_len = static_cast<uint16_t>(len);
        // type at src[2], reserved at src[3]

        length = (opt_len > 1) ? static_cast<size_t>(opt_len - 1) : 0u;
        if (length > MaxConfigLen)
        {
          length = MaxConfigLen;
        }

        etl::mem_copy(src + 4, src + 4 + length, data.data());

        size_t total = 4 + ((opt_len > 1) ? static_cast<size_t>(opt_len - 1) : 0u);
        return src + total;
      }
    };

    //=========================================================================
    // sd_header — the 12-byte SD-specific header that sits inside the
    // SOME/IP payload. Layout:
    //
    //  Bytes 0-3  : Flags (byte 0) + Reserved (bytes 1-3)
    //  Bytes 4-7  : Length of entries array (in bytes)
    //  ...entries...
    //  Next 4 bytes: Length of options array (in bytes)
    //  ...options...
    //=========================================================================
    struct sd_header
    {
      uint8_t flags;

      sd_header()
        : flags(0)
      {
      }

      void set_reboot(bool v)
      {
        if (v)
          flags |= SD_FLAG_REBOOT;
        else
          flags &= static_cast<uint8_t>(~SD_FLAG_REBOOT);
      }
      void set_unicast(bool v)
      {
        if (v)
          flags |= SD_FLAG_UNICAST;
        else
          flags &= static_cast<uint8_t>(~SD_FLAG_UNICAST);
      }
      void set_explicit_init_data(bool v)
      {
        if (v)
          flags |= SD_FLAG_EXPLICIT_INIT_DATA;
        else
          flags &= static_cast<uint8_t>(~SD_FLAG_EXPLICIT_INIT_DATA);
      }

      bool reboot() const
      {
        return (flags & SD_FLAG_REBOOT) != 0;
      }
      bool unicast() const
      {
        return (flags & SD_FLAG_UNICAST) != 0;
      }
      bool explicit_init_data() const
      {
        return (flags & SD_FLAG_EXPLICIT_INIT_DATA) != 0;
      }

      /// Encode flags + 3 reserved bytes (4 bytes total) at dest.
      uint8_t* encode_flags(uint8_t* dest) const
      {
        dest[0] = flags;
        dest[1] = 0x00;
        dest[2] = 0x00;
        dest[3] = 0x00;
        return dest + 4;
      }

      /// Decode flags from 4 bytes at src.
      const uint8_t* decode_flags(const uint8_t* src)
      {
        flags = src[0];
        return src + 4;
      }
    };

    //=========================================================================
    // sd_message — container holding one complete SD payload.
    //
    //  Template parameters:
    //    MaxEntries  — max number of entries (default 16)
    //    MaxOptions  — max number of IPv4 options (default 16)
    //
    //  Wire layout (inside the SOME/IP payload):
    //    [flags 4B][entries_length 4B][entries...][options_length 4B][options...]
    //=========================================================================
    template <size_t MaxEntries = 16, size_t MaxIPv4Options = 16>
    struct sd_message
    {
      sd_header                                  header;
      etl::array<sd_entry, MaxEntries>           entries;
      size_t                                     num_entries;
      etl::array<sd_ipv4_option, MaxIPv4Options> ipv4_options;
      size_t                                     num_ipv4_options;

      sd_message()
        : num_entries(0)
        , num_ipv4_options(0)
      {
      }

      //-----------------------------------------------------------------------
      /// Build the SOME/IP header for an SD message.
      //-----------------------------------------------------------------------
      someip_header make_someip_header(uint16_t session_id) const
      {
        someip_header h;
        h.service_id        = SD_SERVICE_ID;
        h.method_id         = SD_METHOD_ID;
        h.client_id         = SD_CLIENT_ID;
        h.session_id        = session_id;
        h.protocol_version  = PROTOCOL_VERSION;
        h.interface_version = SD_INTERFACE_VERSION;
        h.message_type      = NOTIFICATION;
        h.return_code       = E_OK;
        return h;
      }

      //-----------------------------------------------------------------------
      /// Encode the SD payload into \p dest. Returns bytes written.
      //-----------------------------------------------------------------------
      size_t encode_payload(uint8_t* dest, size_t max_len) const
      {
        (void)max_len;
        uint8_t* p = dest;

        // Flags + reserved (4 bytes)
        p = header.encode_flags(p);

        // Entries length (4 bytes) + entries
        uint32_t             entries_len = static_cast<uint32_t>(num_entries * SD_ENTRY_SIZE);
        etl::be_uint32_ext_t el(p);
        el = entries_len;
        p += 4;

        for (size_t i = 0; i < num_entries; ++i)
        {
          p = entries[i].encode(p);
        }

        // Options length (4 bytes) + options
        uint32_t             options_len = static_cast<uint32_t>(num_ipv4_options * SD_IPV4_OPTION_SIZE);
        etl::be_uint32_ext_t ol(p);
        ol = options_len;
        p += 4;

        for (size_t i = 0; i < num_ipv4_options; ++i)
        {
          p = ipv4_options[i].encode(p);
        }

        return static_cast<size_t>(p - dest);
      }

      //-----------------------------------------------------------------------
      /// Decode the SD payload from \p src of \p len bytes.
      /// Returns true on success.
      //-----------------------------------------------------------------------
      bool decode_payload(const uint8_t* src, size_t len)
      {
        if (len < 12)
        {
          return false;
        } // flags(4) + entries_len(4) + options_len(4) minimum

        const uint8_t* p = src;

        // Flags
        p = header.decode_flags(p);

        // Entries length
        etl::be_uint32_ext_t el(const_cast<uint8_t*>(p));
        uint32_t             entries_len = static_cast<uint32_t>(el);
        p += 4;

        size_t entry_count = entries_len / SD_ENTRY_SIZE;
        if (entry_count > MaxEntries)
        {
          entry_count = MaxEntries;
        }
        num_entries = entry_count;

        for (size_t i = 0; i < num_entries; ++i)
        {
          p = entries[i].decode(p);
        }

        // Skip any extra entry bytes we couldn't store
        const uint8_t* entries_end = src + 8 + entries_len;
        p                          = entries_end;

        // Options length
        if (static_cast<size_t>(p - src) + 4 > len)
        {
          return false;
        }
        etl::be_uint32_ext_t ol(const_cast<uint8_t*>(p));
        uint32_t             options_len = static_cast<uint32_t>(ol);
        p += 4;

        // Parse IPv4 options (we only handle IPv4 endpoint-sized options for now)
        num_ipv4_options           = 0;
        const uint8_t* options_end = p + options_len;

        while (p + 4 <= options_end && num_ipv4_options < MaxIPv4Options)
        {
          // Peek at length and type
          etl::be_uint16_ext_t opt_len_field(const_cast<uint8_t*>(p));
          uint16_t             opt_body_len = static_cast<uint16_t>(opt_len_field);
          uint8_t              opt_type     = p[2];
          size_t               opt_wire     = 4 + static_cast<size_t>(opt_body_len > 1 ? opt_body_len - 1 : 0);

          if (opt_body_len == 0x0009
              && (opt_type == SD_OPT_IPV4_ENDPOINT || opt_type == SD_OPT_IPV4_MULTICAST || opt_type == SD_OPT_IPV4_SD_ENDPOINT))
          {
            p = ipv4_options[num_ipv4_options].decode(p);
            ++num_ipv4_options;
          }
          else
          {
            p += opt_wire; // skip unknown options
          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      /// Convenience: add an entry. Returns false if full.
      //-----------------------------------------------------------------------
      bool add_entry(const sd_entry& e)
      {
        if (num_entries >= MaxEntries)
        {
          return false;
        }
        entries[num_entries++] = e;
        return true;
      }

      //-----------------------------------------------------------------------
      /// Convenience: add an IPv4 option. Returns the option index, or -1 if full.
      //-----------------------------------------------------------------------
      int add_ipv4_option(const sd_ipv4_option& opt)
      {
        if (num_ipv4_options >= MaxIPv4Options)
        {
          return -1;
        }
        int idx                          = static_cast<int>(num_ipv4_options);
        ipv4_options[num_ipv4_options++] = opt;
        return idx;
      }
    };

    //=========================================================================
    // SD entry builder helpers — fluent API for constructing common entries.
    //=========================================================================

    /// Build a FindService entry.
    inline sd_entry make_find_entry(uint16_t service_id, uint16_t instance_id = SD_INSTANCE_ANY, uint8_t major_version = SD_MAJOR_VERSION_ANY,
                                    uint32_t minor_version = SD_MINOR_VERSION_ANY, uint32_t ttl = 3)
    {
      sd_entry e;
      e.type          = SD_FIND_SERVICE;
      e.service_id    = service_id;
      e.instance_id   = instance_id;
      e.major_version = major_version;
      e.minor_version = minor_version;
      e.ttl           = ttl;
      return e;
    }

    /// Build an OfferService entry (ttl=0 means StopOffer).
    inline sd_entry make_offer_entry(uint16_t service_id, uint16_t instance_id, uint8_t major_version, uint32_t minor_version,
                                     uint32_t ttl = SD_TTL_FOREVER)
    {
      sd_entry e;
      e.type          = SD_OFFER_SERVICE;
      e.service_id    = service_id;
      e.instance_id   = instance_id;
      e.major_version = major_version;
      e.minor_version = minor_version;
      e.ttl           = ttl;
      return e;
    }

    /// Build a SubscribeEventgroup entry (ttl=0 means StopSubscribe).
    inline sd_entry make_subscribe_entry(uint16_t service_id, uint16_t instance_id, uint8_t major_version, uint16_t eventgroup_id,
                                         uint32_t ttl = SD_TTL_FOREVER, uint8_t counter = 0)
    {
      sd_entry e;
      e.type          = SD_SUBSCRIBE_EVENTGROUP;
      e.service_id    = service_id;
      e.instance_id   = instance_id;
      e.major_version = major_version;
      e.eventgroup_id = eventgroup_id;
      e.counter       = counter;
      e.ttl           = ttl;
      return e;
    }

    /// Build a SubscribeEventgroupAck entry (ttl=0 means Nack).
    inline sd_entry make_subscribe_ack_entry(uint16_t service_id, uint16_t instance_id, uint8_t major_version, uint16_t eventgroup_id,
                                             uint32_t ttl = SD_TTL_FOREVER, uint8_t counter = 0)
    {
      sd_entry e;
      e.type          = SD_SUBSCRIBE_EVENTGROUP_ACK;
      e.service_id    = service_id;
      e.instance_id   = instance_id;
      e.major_version = major_version;
      e.eventgroup_id = eventgroup_id;
      e.counter       = counter;
      e.ttl           = ttl;
      return e;
    }

    /// Build an IPv4 endpoint option.
    inline sd_ipv4_option make_ipv4_endpoint(const etl::array<uint8_t, 4>& addr, sd_l4_protocol_t proto, uint16_t port)
    {
      sd_ipv4_option opt;
      opt.type         = SD_OPT_IPV4_ENDPOINT;
      opt.ipv4_address = addr;
      opt.protocol     = proto;
      opt.port         = port;
      return opt;
    }

    /// Build an IPv4 multicast option.
    inline sd_ipv4_option make_ipv4_multicast(const etl::array<uint8_t, 4>& addr, uint16_t port)
    {
      sd_ipv4_option opt;
      opt.type         = SD_OPT_IPV4_MULTICAST;
      opt.ipv4_address = addr;
      opt.protocol     = SD_L4_UDP;
      opt.port         = port;
      return opt;
    }

    //=========================================================================
    // SD server-side phase enum — per SOME/IP-SD spec state machine.
    //=========================================================================
    enum sd_server_phase_t : uint8_t
    {
      SD_PHASE_DOWN,         ///< Service not yet started / network down
      SD_PHASE_INITIAL_WAIT, ///< Waiting before first offer
      SD_PHASE_REPETITION,   ///< Sending offers with exponential backoff
      SD_PHASE_MAIN          ///< Steady-state, periodic offers
    };

    //=========================================================================
    // sd_service_instance — tracks one offered service on the server side.
    //
    //  The user calls tick(elapsed_ms) periodically. When it is time to
    //  send an offer the service returns needs_offer()==true and the user
    //  calls mark_offered().
    //
    //  Template-free: all timing parameters are plain members.
    //=========================================================================
    struct sd_service_instance
    {
      uint16_t service_id;
      uint16_t instance_id;
      uint8_t  major_version;
      uint32_t minor_version;

      // --- Timing parameters (milliseconds) ---
      uint32_t initial_delay_ms;   ///< Random delay before first offer
      uint32_t repetition_base_ms; ///< Base delay for repetition phase
      uint8_t  repetition_max;     ///< Number of repetition offers (e.g. 3)
      uint32_t main_cycle_ms;      ///< Cyclic offer interval in main phase
      uint32_t ttl;                ///< TTL to put into offer entries

      // --- Runtime state ---
      sd_server_phase_t phase;
      uint32_t          timer_ms;           ///< Countdown to next action
      uint8_t           repetition_count;   ///< How many repetition offers sent so far
      uint32_t          current_backoff_ms; ///< Current repetition interval

      sd_service_instance()
        : service_id(0)
        , instance_id(0)
        , major_version(0)
        , minor_version(0)
        , initial_delay_ms(500)
        , repetition_base_ms(1000)
        , repetition_max(3)
        , main_cycle_ms(5000)
        , ttl(SD_TTL_FOREVER)
        , phase(SD_PHASE_DOWN)
        , timer_ms(0)
        , repetition_count(0)
        , current_backoff_ms(0)
      {
      }

      /// Start the service — enters INITIAL_WAIT.
      void start()
      {
        phase              = SD_PHASE_INITIAL_WAIT;
        timer_ms           = initial_delay_ms;
        repetition_count   = 0;
        current_backoff_ms = repetition_base_ms;
      }

      /// Stop the service — caller should send StopOffer (TTL=0).
      void stop()
      {
        phase = SD_PHASE_DOWN;
      }

      /// Advance time by \p elapsed_ms. After calling, check needs_offer().
      void tick(uint32_t elapsed_ms)
      {
        if (phase == SD_PHASE_DOWN)
        {
          return;
        }

        if (elapsed_ms >= timer_ms)
        {
          timer_ms = 0;
        }
        else
        {
          timer_ms -= elapsed_ms;
        }
      }

      /// True if the state machine says we should send an offer now.
      bool needs_offer() const
      {
        return (phase != SD_PHASE_DOWN) && (timer_ms == 0);
      }

      /// Call after actually sending the offer.
      void mark_offered()
      {
        switch (phase)
        {
          case SD_PHASE_INITIAL_WAIT:
            phase              = SD_PHASE_REPETITION;
            repetition_count   = 1;
            current_backoff_ms = repetition_base_ms;
            timer_ms           = current_backoff_ms;
            break;

          case SD_PHASE_REPETITION:
            ++repetition_count;
            if (repetition_count >= repetition_max)
            {
              phase    = SD_PHASE_MAIN;
              timer_ms = main_cycle_ms;
            }
            else
            {
              current_backoff_ms *= 2; // exponential backoff
              timer_ms = current_backoff_ms;
            }
            break;

          case SD_PHASE_MAIN: timer_ms = main_cycle_ms; break;

          default: break;
        }
      }

      /// Build an offer entry for this service instance.
      sd_entry make_offer() const
      {
        return make_offer_entry(service_id, instance_id, major_version, minor_version, ttl);
      }

      /// Build a StopOffer entry.
      sd_entry make_stop_offer() const
      {
        return make_offer_entry(service_id, instance_id, major_version, minor_version, 0);
      }
    };

    //=========================================================================
    // SD client-side phase enum.
    //=========================================================================
    enum sd_client_phase_t : uint8_t
    {
      SD_CLIENT_DOWN,         ///< Not searching
      SD_CLIENT_INITIAL_WAIT, ///< Waiting before first find
      SD_CLIENT_REPETITION,   ///< Sending finds with exponential backoff
      SD_CLIENT_SERVICE_READY ///< Service found, subscribed
    };

    //=========================================================================
    // sd_found_service — records a service discovered via an OfferService entry.
    //=========================================================================
    struct sd_found_service
    {
      uint16_t service_id;
      uint16_t instance_id;
      uint8_t  major_version;
      uint32_t minor_version;
      uint32_t ttl; ///< Remaining TTL in ms (decremented by tick)
      bool     available;

      sd_found_service()
        : service_id(0)
        , instance_id(0)
        , major_version(0)
        , minor_version(0)
        , ttl(0)
        , available(false)
      {
      }
    };

    //=========================================================================
    // sd_client_instance — tracks one service the client is looking for.
    //
    //  Similar state machine to the server: INITIAL_WAIT → REPETITION.
    //  Once a matching OfferService is received, transitions to SERVICE_READY.
    //=========================================================================
    template <size_t MaxFound = 4>
    struct sd_client_instance
    {
      uint16_t service_id;
      uint16_t instance_id;   ///< SD_INSTANCE_ANY to accept any
      uint8_t  major_version; ///< SD_MAJOR_VERSION_ANY to accept any
      uint32_t minor_version; ///< SD_MINOR_VERSION_ANY to accept any

      // Timing
      uint32_t initial_delay_ms;
      uint32_t repetition_base_ms;
      uint8_t  repetition_max;

      // Runtime
      sd_client_phase_t phase;
      uint32_t          timer_ms;
      uint8_t           repetition_count;
      uint32_t          current_backoff_ms;

      etl::array<sd_found_service, MaxFound> found;
      size_t                                 num_found;

      sd_client_instance()
        : service_id(0)
        , instance_id(SD_INSTANCE_ANY)
        , major_version(SD_MAJOR_VERSION_ANY)
        , minor_version(SD_MINOR_VERSION_ANY)
        , initial_delay_ms(500)
        , repetition_base_ms(1000)
        , repetition_max(3)
        , phase(SD_CLIENT_DOWN)
        , timer_ms(0)
        , repetition_count(0)
        , current_backoff_ms(0)
        , num_found(0)
      {
      }

      void start()
      {
        phase              = SD_CLIENT_INITIAL_WAIT;
        timer_ms           = initial_delay_ms;
        repetition_count   = 0;
        current_backoff_ms = repetition_base_ms;
        num_found          = 0;
      }

      void stop()
      {
        phase     = SD_CLIENT_DOWN;
        num_found = 0;
      }

      void tick(uint32_t elapsed_ms)
      {
        if (phase == SD_CLIENT_DOWN)
        {
          return;
        }

        if (elapsed_ms >= timer_ms)
        {
          timer_ms = 0;
        }
        else
        {
          timer_ms -= elapsed_ms;
        }

        // Decrement TTLs of found services
        for (size_t i = 0; i < num_found; ++i)
        {
          if (found[i].available)
          {
            if (elapsed_ms >= found[i].ttl)
            {
              found[i].ttl       = 0;
              found[i].available = false;
            }
            else
            {
              found[i].ttl -= elapsed_ms;
            }
          }
        }
      }

      /// True if it's time to send a FindService.
      bool needs_find() const
      {
        return (phase == SD_CLIENT_INITIAL_WAIT || phase == SD_CLIENT_REPETITION) && timer_ms == 0;
      }

      /// Call after sending a FindService.
      void mark_find_sent()
      {
        switch (phase)
        {
          case SD_CLIENT_INITIAL_WAIT:
            phase              = SD_CLIENT_REPETITION;
            repetition_count   = 1;
            current_backoff_ms = repetition_base_ms;
            timer_ms           = current_backoff_ms;
            break;

          case SD_CLIENT_REPETITION:
            ++repetition_count;
            if (repetition_count >= repetition_max)
            {
              // Stay in repetition but stop sending (wait for offer)
              timer_ms = 0xFFFFFFFF; // effectively infinite
            }
            else
            {
              current_backoff_ms *= 2;
              timer_ms = current_backoff_ms;
            }
            break;

          default: break;
        }
      }

      /// Process an incoming OfferService entry. Returns true if it matches.
      bool process_offer(const sd_entry& offer)
      {
        if (offer.type != SD_OFFER_SERVICE)
        {
          return false;
        }
        if (offer.service_id != service_id)
        {
          return false;
        }
        if (instance_id != SD_INSTANCE_ANY && offer.instance_id != instance_id)
        {
          return false;
        }
        if (major_version != SD_MAJOR_VERSION_ANY && offer.major_version != major_version)
        {
          return false;
        }
        if (minor_version != SD_MINOR_VERSION_ANY && offer.minor_version != minor_version)
        {
          return false;
        }

        if (offer.is_stop())
        {
          // StopOffer — mark matching found service unavailable
          for (size_t i = 0; i < num_found; ++i)
          {
            if (found[i].service_id == offer.service_id && found[i].instance_id == offer.instance_id)
            {
              found[i].available = false;
              found[i].ttl       = 0;
            }
          }
          // Check if any are still available
          bool any = false;
          for (size_t i = 0; i < num_found; ++i)
          {
            if (found[i].available)
            {
              any = true;
              break;
            }
          }
          if (!any && phase == SD_CLIENT_SERVICE_READY)
          {
            phase    = SD_CLIENT_REPETITION; // go back to searching
            timer_ms = 0;                    // send find immediately
          }
          return true;
        }

        // Active offer — record or refresh
        // Convert TTL from seconds to ms (capped at 0xFFFFFFFF)
        uint32_t ttl_ms = (offer.ttl <= 0x00418937) ? offer.ttl * 1000 : 0xFFFFFFFF;

        for (size_t i = 0; i < num_found; ++i)
        {
          if (found[i].instance_id == offer.instance_id)
          {
            found[i].major_version = offer.major_version;
            found[i].minor_version = offer.minor_version;
            found[i].ttl           = ttl_ms;
            found[i].available     = true;
            phase                  = SD_CLIENT_SERVICE_READY;
            return true;
          }
        }

        // New instance
        if (num_found < MaxFound)
        {
          sd_found_service& f = found[num_found++];
          f.service_id        = offer.service_id;
          f.instance_id       = offer.instance_id;
          f.major_version     = offer.major_version;
          f.minor_version     = offer.minor_version;
          f.ttl               = ttl_ms;
          f.available         = true;
          phase               = SD_CLIENT_SERVICE_READY;
        }

        return true;
      }

      /// Build a FindService entry.
      sd_entry make_find() const
      {
        return make_find_entry(service_id, instance_id, major_version, minor_version, 3);
      }

      /// True if at least one matching service is currently available.
      bool is_available() const
      {
        for (size_t i = 0; i < num_found; ++i)
        {
          if (found[i].available)
          {
            return true;
          }
        }
        return false;
      }
    };

    //=========================================================================
    // sd_reboot_detector — detects remote reboots per SOME/IP-SD spec.
    //
    //  A remote has rebooted if:
    //    - reboot flag was true, and now the session ID is *lower* than
    //      the last seen session ID (with reboot flag still true), OR
    //    - reboot flag transitions from false → true.
    //
    //  Template parameter MaxRemotes: max tracked remote endpoints.
    //=========================================================================
    template <size_t MaxRemotes = 8>
    struct sd_reboot_detector
    {
      struct remote_state
      {
        uint16_t identifier; ///< e.g. hash of IP or some unique key
        uint16_t last_session_id;
        bool     last_reboot_flag;
        bool     in_use;

        remote_state()
          : identifier(0)
          , last_session_id(0)
          , last_reboot_flag(false)
          , in_use(false)
        {
        }
      };

      etl::array<remote_state, MaxRemotes> remotes;

      sd_reboot_detector()
        : remotes{}
      {
      }

      //-----------------------------------------------------------------------
      /// Process an incoming SD header. Returns true if a reboot was detected.
      /// \p remote_id is an opaque identifier for the remote (e.g. IP hash).
      //-----------------------------------------------------------------------
      bool check(uint16_t remote_id, uint16_t session_id, bool reboot_flag)
      {
        remote_state* slot = nullptr;

        // Find existing or empty slot
        for (size_t i = 0; i < MaxRemotes; ++i)
        {
          if (remotes[i].in_use && remotes[i].identifier == remote_id)
          {
            slot = &remotes[i];
            break;
          }
        }

        if (slot == nullptr)
        {
          // New remote — find empty slot
          for (size_t i = 0; i < MaxRemotes; ++i)
          {
            if (!remotes[i].in_use)
            {
              slot                   = &remotes[i];
              slot->in_use           = true;
              slot->identifier       = remote_id;
              slot->last_session_id  = session_id;
              slot->last_reboot_flag = reboot_flag;
              return false; // First contact, not a reboot
            }
          }
          return false; // No room, ignore
        }

        bool rebooted = false;

        if (reboot_flag)
        {
          if (!slot->last_reboot_flag)
          {
            // false → true transition = reboot
            rebooted = true;
          }
          else if (session_id < slot->last_session_id)
          {
            // Session ID wrapped backward while reboot flag stayed true = reboot
            rebooted = true;
          }
        }

        slot->last_session_id  = session_id;
        slot->last_reboot_flag = reboot_flag;

        return rebooted;
      }

      /// Remove tracking for a remote.
      void forget(uint16_t remote_id)
      {
        for (size_t i = 0; i < MaxRemotes; ++i)
        {
          if (remotes[i].in_use && remotes[i].identifier == remote_id)
          {
            remotes[i].in_use = false;
          }
        }
      }

      /// Clear all tracked remotes.
      void reset()
      {
        for (size_t i = 0; i < MaxRemotes; ++i)
        {
          remotes[i].in_use = false;
        }
      }
    };

    //=========================================================================
    //
    //  SOME/IP-TP  (Transport Protocol — large message segmentation)
    //
    //  Per Open SOME/IP Specification — someip-tp.rst
    //
    //=========================================================================

    static constexpr size_t  TP_HEADER_SIZE      = 4;    ///< 4-byte TP header after SOME/IP header
    static constexpr size_t  TP_OFFSET_ALIGNMENT = 16;   ///< Offsets / segment lengths must be multiples of 16
    static constexpr size_t  TP_MAX_SEGMENT_SIZE = 1392; ///< 87 × 16 — fits in 1400-byte UDP payload
    static constexpr uint8_t TP_MORE_FLAG        = 0x01; ///< Bit 0 of the TP header's last byte

    //=========================================================================
    // tp_header — 4-byte TP header immediately after the SOME/IP header.
    //
    //  Bits 31..4 : Offset (28 bits, represents offset in bytes; always multiple of 16)
    //  Bits 3..1  : Reserved (3 bits, set to 0)
    //  Bit  0     : More Segments flag (1 = more segments follow)
    //=========================================================================
    struct tp_header
    {
      uint32_t offset;        ///< Byte offset into original payload (multiple of 16)
      bool     more_segments; ///< true if more segments follow

      tp_header()
        : offset(0)
        , more_segments(false)
      {
      }

      tp_header(uint32_t off, bool more)
        : offset(off)
        , more_segments(more)
      {
      }

      /// Encode into 4 bytes at \p dest. Returns dest + 4.
      uint8_t* encode(uint8_t* dest) const
      {
        // Upper 28 bits = offset >> 4, lower 4 bits: reserved(3) + more(1)
        uint32_t             wire = (offset & 0xFFFFFFF0u) | (more_segments ? TP_MORE_FLAG : 0u);
        etl::be_uint32_ext_t w(dest);
        w = wire;
        return dest + TP_HEADER_SIZE;
      }

      /// Decode from 4 bytes at \p src. Returns src + 4.
      const uint8_t* decode(const uint8_t* src)
      {
        etl::be_uint32_ext_t w(const_cast<uint8_t*>(src));
        uint32_t             wire = static_cast<uint32_t>(w);
        offset                    = wire & 0xFFFFFFF0u;
        more_segments             = (wire & TP_MORE_FLAG) != 0;
        return src + TP_HEADER_SIZE;
      }
    };

    //=========================================================================
    // tp_segmenter — splits an original SOME/IP message into TP segments.
    //
    //  Usage:
    //    tp_segmenter seg;
    //    seg.reset(header, payload_ptr, payload_len);
    //    while (seg.has_next()) {
    //      size_t n = seg.next_segment(buf, buf_size);
    //      transport.send(buf, n);
    //    }
    //
    //  Template parameter MaxSegmentPayload controls segment data size
    //  (default 1392 = 87×16).
    //=========================================================================
    template <size_t MaxSegmentPayload = TP_MAX_SEGMENT_SIZE>
    struct tp_segmenter
    {
      static_assert((MaxSegmentPayload % TP_OFFSET_ALIGNMENT) == 0, "MaxSegmentPayload must be a multiple of 16");

      /// Reset for a new original message.
      void reset(const someip_header& hdr, const uint8_t* payload, size_t payload_len)
      {
        original_header_ = hdr;
        payload_         = payload;
        total_len_       = payload_len;
        bytes_sent_      = 0;
      }

      bool has_next() const
      {
        return bytes_sent_ < total_len_;
      }

      /// Write the next segment (SOME/IP header + TP header + segment data) to \p dest.
      /// Returns total bytes written. \p max_len is the dest buffer size.
      size_t next_segment(uint8_t* dest, size_t /*max_len*/)
      {
        if (!has_next())
        {
          return 0;
        }

        size_t remaining    = total_len_ - bytes_sent_;
        bool   is_last      = (remaining <= MaxSegmentPayload);
        size_t seg_data_len = is_last ? remaining : MaxSegmentPayload;

        // SOME/IP header: length = 8 (header tail) + 4 (TP header) + seg_data_len
        someip_header seg_hdr = original_header_;
        seg_hdr.message_type  = static_cast<message_type_t>(static_cast<uint8_t>(seg_hdr.message_type) | TP_FLAG);
        seg_hdr.length        = static_cast<uint32_t>(8 + TP_HEADER_SIZE + seg_data_len);

        uint8_t* p = dest;
        seg_hdr.encode(p);
        p += HEADER_SIZE;

        // TP header
        tp_header tp(static_cast<uint32_t>(bytes_sent_), !is_last);
        p = tp.encode(p);

        // Segment data
        etl::mem_copy(payload_ + bytes_sent_, payload_ + bytes_sent_ + seg_data_len, p);
        p += seg_data_len;

        bytes_sent_ += seg_data_len;

        return static_cast<size_t>(p - dest);
      }

      size_t bytes_sent() const
      {
        return bytes_sent_;
      }
      size_t total_length() const
      {
        return total_len_;
      }

    private:

      someip_header  original_header_;
      const uint8_t* payload_    = nullptr;
      size_t         total_len_  = 0;
      size_t         bytes_sent_ = 0;
    };

    //=========================================================================
    // tp_reassembly_status — result of processing a segment.
    //=========================================================================
    enum tp_reassembly_status_t : uint8_t
    {
      TP_INCOMPLETE, ///< More segments needed
      TP_COMPLETE,   ///< Message fully reassembled
      TP_ERROR       ///< Error (bad alignment, overflow, etc.)
    };

    //=========================================================================
    // tp_reassembler — reassembles TP segments into one original message.
    //
    //  Template parameter MaxMessageSize: max reassembled payload size.
    //
    //  Matching: caller is responsible for routing segments from the same
    //  flow (Message ID + Request ID) to the same reassembler.
    //=========================================================================
    template <size_t MaxMessageSize>
    struct tp_reassembler
    {
      tp_reassembler()
      {
        reset();
      }

      void reset()
      {
        active_         = false;
        session_id_     = 0;
        bytes_received_ = 0;
        total_length_   = 0;
        last_seen_      = false;
        header_         = someip_header();
      }

      /// Process one incoming segment.
      /// \p hdr is the SOME/IP header (with TP flag set).
      /// \p tp is the decoded TP header.
      /// \p seg_data points to the segment payload (after TP header).
      /// \p seg_len is the segment payload length.
      tp_reassembly_status_t process_segment(const someip_header& hdr, const tp_header& tp, const uint8_t* seg_data, size_t seg_len)
      {
        // Validate: segments with more_segments=true must be multiple of 16
        if (tp.more_segments && (seg_len % TP_OFFSET_ALIGNMENT) != 0)
        {
          reset();
          return TP_ERROR;
        }

        uint16_t sid = hdr.session_id;

        // New session → restart reassembly
        if (active_ && sid != session_id_)
        {
          reset();
        }

        if (!active_)
        {
          active_     = true;
          session_id_ = sid;
          header_     = hdr;
          // Clear TP flag from stored header
          header_.message_type = static_cast<message_type_t>(static_cast<uint8_t>(header_.message_type) & ~TP_FLAG);
        }

        // Check bounds
        size_t end = tp.offset + seg_len;
        if (end > MaxMessageSize)
        {
          reset();
          return TP_ERROR;
        }

        // Copy segment data into buffer
        etl::mem_copy(seg_data, seg_data + seg_len, buffer_.data() + tp.offset);

        // Track received bytes (simple: only count non-overlapping first-time bytes)
        // For simplicity we just track the high-water mark of contiguous bytes from 0.
        if (tp.offset + seg_len > bytes_received_)
        {
          bytes_received_ = tp.offset + seg_len;
        }

        // If this is the last segment, record total length
        if (!tp.more_segments)
        {
          total_length_ = end;
          last_seen_    = true;
          // Update the stored header's length to reflect reassembled payload
          header_.length = static_cast<uint32_t>(8 + total_length_);
        }

        // Check if complete
        if (last_seen_ && bytes_received_ >= total_length_)
        {
          return TP_COMPLETE;
        }

        return TP_INCOMPLETE;
      }

      bool is_active() const
      {
        return active_;
      }
      uint16_t session_id() const
      {
        return session_id_;
      }
      size_t bytes_received() const
      {
        return bytes_received_;
      }
      size_t total_length() const
      {
        return total_length_;
      }
      const someip_header& header() const
      {
        return header_;
      }
      const uint8_t* payload() const
      {
        return buffer_.data();
      }

    private:

      bool                                active_         = false;
      uint16_t                            session_id_     = 0;
      size_t                              bytes_received_ = 0;
      size_t                              total_length_   = 0;
      bool                                last_seen_      = false;
      someip_header                       header_;
      etl::array<uint8_t, MaxMessageSize> buffer_;
    };

    //=========================================================================
    // tp_reassembly_pool — manages multiple concurrent reassembly flows.
    //
    //  Template parameters:
    //    MaxFlows       — max simultaneous flows being reassembled
    //    MaxMessageSize — max reassembled payload per flow
    //=========================================================================
    template <size_t MaxFlows, size_t MaxMessageSize>
    struct tp_reassembly_pool
    {
      struct flow_slot
      {
        uint32_t message_id; ///< service_id << 16 | method_id
        uint16_t client_id;
        uint16_t session_id;
        bool     in_use;
        uint32_t age_ms; ///< Time since last segment received

        tp_reassembler<MaxMessageSize> reassembler;

        flow_slot()
          : message_id(0)
          , client_id(0)
          , session_id(0)
          , in_use(false)
          , age_ms(0)
        {
        }
      };

      etl::array<flow_slot, MaxFlows> slots;
      uint32_t                        timeout_ms; ///< Reassembly timeout (default 5000ms)

      tp_reassembly_pool()
        : timeout_ms(5000)
      {
      }

      /// Find or create a reassembly slot for the given header.
      /// Returns pointer to the reassembler, or nullptr if pool is full.
      tp_reassembler<MaxMessageSize>* find_or_create(const someip_header& hdr)
      {
        uint32_t mid = (static_cast<uint32_t>(hdr.service_id) << 16) | hdr.method_id;

        // Look for existing
        for (size_t i = 0; i < MaxFlows; ++i)
        {
          if (slots[i].in_use && slots[i].message_id == mid && slots[i].client_id == hdr.client_id && slots[i].session_id == hdr.session_id)
          {
            slots[i].age_ms = 0;
            return &slots[i].reassembler;
          }
        }

        // Allocate new slot
        for (size_t i = 0; i < MaxFlows; ++i)
        {
          if (!slots[i].in_use)
          {
            slots[i].in_use     = true;
            slots[i].message_id = mid;
            slots[i].client_id  = hdr.client_id;
            slots[i].session_id = hdr.session_id;
            slots[i].age_ms     = 0;
            slots[i].reassembler.reset();
            return &slots[i].reassembler;
          }
        }

        return nullptr; // Pool full
      }

      /// Release a slot after reassembly completes (or on error).
      void release(const someip_header& hdr)
      {
        uint32_t mid = (static_cast<uint32_t>(hdr.service_id) << 16) | hdr.method_id;
        for (size_t i = 0; i < MaxFlows; ++i)
        {
          if (slots[i].in_use && slots[i].message_id == mid && slots[i].client_id == hdr.client_id && slots[i].session_id == hdr.session_id)
          {
            slots[i].in_use = false;
            slots[i].reassembler.reset();
          }
        }
      }

      /// Advance time. Evicts flows that exceed timeout_ms.
      void tick(uint32_t elapsed_ms)
      {
        for (size_t i = 0; i < MaxFlows; ++i)
        {
          if (slots[i].in_use)
          {
            slots[i].age_ms += elapsed_ms;
            if (slots[i].age_ms >= timeout_ms)
            {
              slots[i].in_use = false;
              slots[i].reassembler.reset();
            }
          }
        }
      }
    };

  } // namespace someip
} // namespace etl

#endif // ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION

#endif // ETL_OPENSOMEIP_INCLUDED
