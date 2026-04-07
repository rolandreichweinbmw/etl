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

#include "etl/array.h"
#include "etl/opensomeip.h"
#include "etl/string.h"
#include "etl/vector.h"

#if ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION

//---------------------------------------------------------------------------
// Calculator service — shared interface for RPC tests.
//---------------------------------------------------------------------------
struct SomeipCalculator
{
  int add(int a, int b)
  {
    return a + b;
  }

  int multiply(int a, int b)
  {
    return a * b;
  }
};

//---------------------------------------------------------------------------
// Simple struct for reflection-based serialization tests.
//---------------------------------------------------------------------------
struct SomeipPoint2D
{
  int x;
  int y;
};

//---------------------------------------------------------------------------
// GeometryService — service with struct args and returns.
//---------------------------------------------------------------------------
struct SomeipGeometryService
{
  SomeipPoint2D add_points(SomeipPoint2D a, SomeipPoint2D b)
  {
    return SomeipPoint2D{a.x + b.x, a.y + b.y};
  }
};

//---------------------------------------------------------------------------
// Service that tracks fire-and-forget invocations.
//---------------------------------------------------------------------------
struct SomeipSideEffectService
{
  int last_value = 0;

  void store(int v)
  {
    last_value = v;
  }

  int retrieve()
  {
    return last_value;
  }
};

//---------------------------------------------------------------------------
// Test constants.
//---------------------------------------------------------------------------
static constexpr size_t   MAX_PAYLOAD     = 256;
static constexpr uint16_t TEST_SERVICE_ID = 0x1234;
static constexpr uint16_t TEST_CLIENT_ID  = 0x0001;
static constexpr uint8_t  TEST_IFACE_VER  = 1;

//---------------------------------------------------------------------------
// Convenience aliases.
//---------------------------------------------------------------------------
using Loopback   = etl::someip::loopback_transport<4096>;
using CalcServer = etl::someip::someip_server<SomeipCalculator, MAX_PAYLOAD>;
using CalcClient = etl::someip::someip_client<SomeipCalculator, MAX_PAYLOAD, 8>;
using GeoServer  = etl::someip::someip_server<SomeipGeometryService, MAX_PAYLOAD>;
using GeoClient  = etl::someip::someip_client<SomeipGeometryService, MAX_PAYLOAD, 8>;
using SideServer = etl::someip::someip_server<SomeipSideEffectService, MAX_PAYLOAD>;
using SideClient = etl::someip::someip_client<SomeipSideEffectService, MAX_PAYLOAD, 8>;

namespace
{
  SUITE(test_opensomeip)
  {
    //*************************************************************************
    /// Test: someip_header encode/decode round-trip.
    //*************************************************************************
    TEST(test_header_round_trip)
    {
      etl::someip::someip_header h;
      h.service_id        = 0x1234;
      h.method_id         = 0x0001;
      h.length            = 8 + 10; // 8 header-after-length + 10 payload
      h.client_id         = 0xABCD;
      h.session_id        = 0x0042;
      h.protocol_version  = 0x01;
      h.interface_version = 0x03;
      h.message_type      = etl::someip::RESPONSE;
      h.return_code       = etl::someip::E_OK;

      uint8_t wire[16];
      h.encode(wire);

      // Verify big-endian byte order for multi-byte fields.
      CHECK_EQUAL(0x12, wire[0]);  // service_id high
      CHECK_EQUAL(0x34, wire[1]);  // service_id low
      CHECK_EQUAL(0x00, wire[2]);  // method_id high
      CHECK_EQUAL(0x01, wire[3]);  // method_id low
      CHECK_EQUAL(0x00, wire[4]);  // length byte 3 (MSB)
      CHECK_EQUAL(0x00, wire[5]);  // length byte 2
      CHECK_EQUAL(0x00, wire[6]);  // length byte 1
      CHECK_EQUAL(18, wire[7]);    // length byte 0 (LSB) = 8+10
      CHECK_EQUAL(0xAB, wire[8]);  // client_id high
      CHECK_EQUAL(0xCD, wire[9]);  // client_id low
      CHECK_EQUAL(0x00, wire[10]); // session_id high
      CHECK_EQUAL(0x42, wire[11]); // session_id low
      CHECK_EQUAL(0x01, wire[12]); // protocol_version
      CHECK_EQUAL(0x03, wire[13]); // interface_version
      CHECK_EQUAL(0x80, wire[14]); // message_type = RESPONSE
      CHECK_EQUAL(0x00, wire[15]); // return_code = E_OK

      // Decode back and verify.
      etl::someip::someip_header h2;
      h2.decode(wire);
      CHECK_EQUAL(h.service_id, h2.service_id);
      CHECK_EQUAL(h.method_id, h2.method_id);
      CHECK_EQUAL(h.length, h2.length);
      CHECK_EQUAL(h.client_id, h2.client_id);
      CHECK_EQUAL(h.session_id, h2.session_id);
      CHECK_EQUAL(h.protocol_version, h2.protocol_version);
      CHECK_EQUAL(h.interface_version, h2.interface_version);
      CHECK_EQUAL(h.message_type, h2.message_type);
      CHECK_EQUAL(h.return_code, h2.return_code);
    }

    //*************************************************************************
    /// Test: header payload_size() and set_payload_size().
    //*************************************************************************
    TEST(test_header_payload_size)
    {
      etl::someip::someip_header h;
      h.set_payload_size(100);
      CHECK_EQUAL(108u, h.length);
      CHECK_EQUAL(100u, h.payload_size());

      h.set_payload_size(0);
      CHECK_EQUAL(8u, h.length);
      CHECK_EQUAL(0u, h.payload_size());
    }

    //*************************************************************************
    /// Test: someip_message encode/decode round-trip with payload.
    //*************************************************************************
    TEST(test_message_encode_decode)
    {
      etl::someip::someip_message<64> msg;
      msg.header.service_id        = 0x4321;
      msg.header.method_id         = 0x0005;
      msg.header.client_id         = 0x0010;
      msg.header.session_id        = 0x0001;
      msg.header.protocol_version  = etl::someip::PROTOCOL_VERSION;
      msg.header.interface_version = 2;
      msg.header.message_type      = etl::someip::REQUEST;
      msg.header.return_code       = etl::someip::E_OK;

      // Write some payload bytes.
      msg.payload[0]   = 0xDE;
      msg.payload[1]   = 0xAD;
      msg.payload[2]   = 0xBE;
      msg.payload[3]   = 0xEF;
      msg.payload_used = 4;

      // Encode to wire.
      uint8_t wire[128];
      size_t  wire_len = msg.encode(wire);
      CHECK_EQUAL(size_t(16 + 4), wire_len);

      // Decode back.
      etl::someip::someip_message<64> msg2;
      bool                            ok = msg2.decode(wire, wire_len);
      CHECK(ok);
      CHECK_EQUAL(msg.header.service_id, msg2.header.service_id);
      CHECK_EQUAL(msg.header.method_id, msg2.header.method_id);
      CHECK_EQUAL(4u, msg2.payload_used);
      CHECK_EQUAL(0xDE, msg2.payload[0]);
      CHECK_EQUAL(0xAD, msg2.payload[1]);
      CHECK_EQUAL(0xBE, msg2.payload[2]);
      CHECK_EQUAL(0xEF, msg2.payload[3]);
    }

    //*************************************************************************
    /// Test: message decode rejects too-short buffer.
    //*************************************************************************
    TEST(test_message_decode_too_short)
    {
      uint8_t                         wire[10] = {};
      etl::someip::someip_message<64> msg;
      CHECK(!msg.decode(wire, 10)); // < 16 bytes header
    }

    //*************************************************************************
    /// Test: session_id_generator wraps 0xFFFF -> 0x0001, never 0.
    //*************************************************************************
    TEST(test_session_id_wrapping)
    {
      etl::someip::session_id_generator gen;

      // First ID is 1.
      CHECK_EQUAL(1u, gen.next());
      CHECK_EQUAL(2u, gen.next());

      // Fast-forward to near wrap.
      for (uint32_t i = 3; i < 0xFFFF; ++i)
      {
        gen.next();
      }

      // Should be 0xFFFF now.
      CHECK_EQUAL(0xFFFFu, gen.next());

      // Wraps to 1, not 0.
      CHECK_EQUAL(1u, gen.next());
      CHECK_EQUAL(2u, gen.next());
    }

    //*************************************************************************
    /// Test: loopback_transport send/receive.
    //*************************************************************************
    TEST(test_loopback_transport)
    {
      Loopback lb;

      CHECK_EQUAL(0u, lb.available());

      uint8_t send_data[] = {1, 2, 3, 4, 5};
      CHECK(lb.send(send_data, 5));
      CHECK_EQUAL(5u, lb.available());

      uint8_t recv_buf[16];
      size_t  received = 0;
      CHECK(lb.receive(recv_buf, sizeof(recv_buf), received));
      CHECK_EQUAL(5u, received);
      CHECK_EQUAL(1, recv_buf[0]);
      CHECK_EQUAL(5, recv_buf[4]);

      // After draining, nothing left.
      CHECK_EQUAL(0u, lb.available());
      CHECK(!lb.receive(recv_buf, sizeof(recv_buf), received));

      // Reset works.
      lb.send(send_data, 3);
      CHECK_EQUAL(3u, lb.available());
      lb.reset();
      CHECK_EQUAL(0u, lb.available());
    }

    //*************************************************************************
    /// Test: basic request/response round-trip (Calculator::add).
    ///
    /// Pattern:
    ///   1. client.call<^^SomeipCalculator::add>(3, 4) → sends REQUEST
    ///   2. server.process() → receives, dispatches, sends RESPONSE
    ///   3. client.poll() → receives RESPONSE, fulfils future
    ///   4. future.get() == 7
    //*************************************************************************
    TEST(test_request_response_add)
    {
      Loopback         transport;
      SomeipCalculator calc_impl;

      CalcServer server(transport, calc_impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      CalcClient client(transport, TEST_SERVICE_ID, TEST_CLIENT_ID, TEST_IFACE_VER);

      // 1. Client sends request.
      auto fut = client.call<^^SomeipCalculator::add>(3, 4);
      CHECK(fut.valid());
      CHECK(!fut.is_ready());

      // 2. Server processes request and sends response back.
      bool processed = server.process();
      CHECK(processed);

      // 3. Client polls response.
      bool received = client.poll();
      CHECK(received);

      // 4. Future should be fulfilled.
      CHECK(fut.is_ready());
      CHECK_EQUAL(7, fut.get());
    }

    //*************************************************************************
    /// Test: multiply method.
    //*************************************************************************
    TEST(test_request_response_multiply)
    {
      Loopback         transport;
      SomeipCalculator calc_impl;

      CalcServer server(transport, calc_impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      CalcClient client(transport, TEST_SERVICE_ID, TEST_CLIENT_ID, TEST_IFACE_VER);

      auto fut = client.call<^^SomeipCalculator::multiply>(6, 7);
      server.process();
      client.poll();

      CHECK(fut.is_ready());
      CHECK_EQUAL(42, fut.get());
    }

    //*************************************************************************
    /// Test: struct arguments and return via GeometryService.
    //*************************************************************************
    TEST(test_struct_args_return)
    {
      Loopback              transport;
      SomeipGeometryService geo_impl;

      GeoServer server(transport, geo_impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      GeoClient client(transport, TEST_SERVICE_ID, TEST_CLIENT_ID, TEST_IFACE_VER);

      SomeipPoint2D a{10, 20};
      SomeipPoint2D b{3, 7};

      auto fut = client.call<^^SomeipGeometryService::add_points>(a, b);
      server.process();
      client.poll();

      CHECK(fut.is_ready());
      SomeipPoint2D result = fut.get();
      CHECK_EQUAL(13, result.x);
      CHECK_EQUAL(27, result.y);
    }

    //*************************************************************************
    /// Test: multiple sequential calls.
    //*************************************************************************
    TEST(test_multiple_sequential_calls)
    {
      Loopback         transport;
      SomeipCalculator calc_impl;

      CalcServer server(transport, calc_impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      CalcClient client(transport, TEST_SERVICE_ID, TEST_CLIENT_ID, TEST_IFACE_VER);

      // Call add, then multiply, sequentially.
      auto f1 = client.call<^^SomeipCalculator::add>(10, 20);
      server.process();
      client.poll();
      CHECK_EQUAL(30, f1.get());

      auto f2 = client.call<^^SomeipCalculator::multiply>(5, 5);
      server.process();
      client.poll();
      CHECK_EQUAL(25, f2.get());
    }

    //*************************************************************************
    /// Test: fire & forget — REQUEST_NO_RETURN, no response.
    //*************************************************************************
    TEST(test_fire_and_forget)
    {
      Loopback                transport;
      SomeipSideEffectService impl;

      SideServer server(transport, impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      SideClient client(transport, TEST_SERVICE_ID, TEST_CLIENT_ID, TEST_IFACE_VER);

      CHECK_EQUAL(0, impl.last_value);

      // Fire & forget: store(42).
      client.fire_and_forget<^^SomeipSideEffectService::store>(42);

      // Server processes — dispatches store(42), no response sent.
      bool processed = server.process();
      CHECK(processed);
      CHECK_EQUAL(42, impl.last_value);

      // Nothing in transport for client to read.
      CHECK_EQUAL(0u, transport.available());
    }

    //*************************************************************************
    /// Test: server publishes notification, client receives it.
    //*************************************************************************
    TEST(test_notification)
    {
      Loopback         transport;
      SomeipCalculator calc_impl;

      CalcServer server(transport, calc_impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      CalcClient client(transport, TEST_SERVICE_ID, TEST_CLIENT_ID, TEST_IFACE_VER);

      // Server publishes a notification with an int value.
      int notification_value = 999;
      server.notify(uint16_t(0x0001), notification_value);

      // Client receives notification.
      int      received_value = 0;
      uint16_t method_id      = 0;
      bool     got            = client.receive_notification(received_value, method_id);
      CHECK(got);
      CHECK_EQUAL(999, received_value);
      // method_id should have EVENT_METHOD_ID_BIT set.
      CHECK_EQUAL(0x8001u, method_id);
    }

    //*************************************************************************
    /// Test: server returns E_WRONG_PROTOCOL_VERSION.
    //*************************************************************************
    TEST(test_error_wrong_protocol_version)
    {
      Loopback transport;

      // Manually craft a request with wrong protocol version.
      etl::someip::someip_message<MAX_PAYLOAD> msg;
      msg.header.service_id        = TEST_SERVICE_ID;
      msg.header.method_id         = 0;
      msg.header.client_id         = TEST_CLIENT_ID;
      msg.header.session_id        = 1;
      msg.header.protocol_version  = 0xFF; // WRONG
      msg.header.interface_version = TEST_IFACE_VER;
      msg.header.message_type      = etl::someip::REQUEST;
      msg.header.return_code       = etl::someip::E_OK;
      msg.payload_used             = 0;
      msg.header.set_payload_size(0);

      uint8_t wire[512];
      size_t  len = msg.encode(wire);
      transport.send(wire, len);

      SomeipCalculator calc_impl;
      CalcServer       server(transport, calc_impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      server.process();

      // Server should have sent an error response.
      etl::someip::someip_message<MAX_PAYLOAD> resp;
      size_t                                   recv_len = 0;
      CHECK(transport.receive(wire, sizeof(wire), recv_len));
      CHECK(resp.decode(wire, recv_len));
      CHECK_EQUAL(etl::someip::EXCEPTION, resp.header.message_type);
      CHECK_EQUAL(etl::someip::E_WRONG_PROTOCOL_VERSION, resp.header.return_code);
    }

    //*************************************************************************
    /// Test: server returns E_WRONG_INTERFACE_VERSION.
    //*************************************************************************
    TEST(test_error_wrong_interface_version)
    {
      Loopback transport;

      etl::someip::someip_message<MAX_PAYLOAD> msg;
      msg.header.service_id        = TEST_SERVICE_ID;
      msg.header.method_id         = 0;
      msg.header.client_id         = TEST_CLIENT_ID;
      msg.header.session_id        = 1;
      msg.header.protocol_version  = etl::someip::PROTOCOL_VERSION;
      msg.header.interface_version = 0xFF; // WRONG
      msg.header.message_type      = etl::someip::REQUEST;
      msg.header.return_code       = etl::someip::E_OK;
      msg.payload_used             = 0;
      msg.header.set_payload_size(0);

      uint8_t wire[512];
      size_t  len = msg.encode(wire);
      transport.send(wire, len);

      SomeipCalculator calc_impl;
      CalcServer       server(transport, calc_impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      server.process();

      etl::someip::someip_message<MAX_PAYLOAD> resp;
      size_t                                   recv_len = 0;
      CHECK(transport.receive(wire, sizeof(wire), recv_len));
      CHECK(resp.decode(wire, recv_len));
      CHECK_EQUAL(etl::someip::EXCEPTION, resp.header.message_type);
      CHECK_EQUAL(etl::someip::E_WRONG_INTERFACE_VERSION, resp.header.return_code);
    }

    //*************************************************************************
    /// Test: server returns E_UNKNOWN_SERVICE.
    //*************************************************************************
    TEST(test_error_unknown_service)
    {
      Loopback transport;

      etl::someip::someip_message<MAX_PAYLOAD> msg;
      msg.header.service_id        = 0xFFFF; // Wrong service
      msg.header.method_id         = 0;
      msg.header.client_id         = TEST_CLIENT_ID;
      msg.header.session_id        = 1;
      msg.header.protocol_version  = etl::someip::PROTOCOL_VERSION;
      msg.header.interface_version = TEST_IFACE_VER;
      msg.header.message_type      = etl::someip::REQUEST;
      msg.header.return_code       = etl::someip::E_OK;
      msg.payload_used             = 0;
      msg.header.set_payload_size(0);

      uint8_t wire[512];
      size_t  len = msg.encode(wire);
      transport.send(wire, len);

      SomeipCalculator calc_impl;
      CalcServer       server(transport, calc_impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      server.process();

      etl::someip::someip_message<MAX_PAYLOAD> resp;
      size_t                                   recv_len = 0;
      CHECK(transport.receive(wire, sizeof(wire), recv_len));
      CHECK(resp.decode(wire, recv_len));
      CHECK_EQUAL(etl::someip::EXCEPTION, resp.header.message_type);
      CHECK_EQUAL(etl::someip::E_UNKNOWN_SERVICE, resp.header.return_code);
    }

    //*************************************************************************
    /// Test: server returns E_UNKNOWN_METHOD for bad method_id.
    //*************************************************************************
    TEST(test_error_unknown_method)
    {
      Loopback transport;

      etl::someip::someip_message<MAX_PAYLOAD> msg;
      msg.header.service_id        = TEST_SERVICE_ID;
      msg.header.method_id         = 0x7FFF; // No such method
      msg.header.client_id         = TEST_CLIENT_ID;
      msg.header.session_id        = 1;
      msg.header.protocol_version  = etl::someip::PROTOCOL_VERSION;
      msg.header.interface_version = TEST_IFACE_VER;
      msg.header.message_type      = etl::someip::REQUEST;
      msg.header.return_code       = etl::someip::E_OK;
      msg.payload_used             = 0;
      msg.header.set_payload_size(0);

      uint8_t wire[512];
      size_t  len = msg.encode(wire);
      transport.send(wire, len);

      SomeipCalculator calc_impl;
      CalcServer       server(transport, calc_impl, TEST_SERVICE_ID, TEST_IFACE_VER);
      server.process();

      etl::someip::someip_message<MAX_PAYLOAD> resp;
      size_t                                   recv_len = 0;
      CHECK(transport.receive(wire, sizeof(wire), recv_len));
      CHECK(resp.decode(wire, recv_len));
      CHECK_EQUAL(etl::someip::EXCEPTION, resp.header.message_type);
      CHECK_EQUAL(etl::someip::E_UNKNOWN_METHOD, resp.header.return_code);
    }

    //*************************************************************************
    /// Test: malformed message (length < 8) is silently dropped.
    //*************************************************************************
    TEST(test_malformed_message_dropped)
    {
      Loopback transport;

      // Craft a wire buffer with length field = 4 (< 8).
      uint8_t wire[16] = {};
      // Service ID
      wire[0] = 0x12;
      wire[1] = 0x34;
      // Method ID
      wire[2] = 0x00;
      wire[3] = 0x01;
      // Length = 4 (too short, must be >= 8).
      wire[4] = 0x00;
      wire[5] = 0x00;
      wire[6] = 0x00;
      wire[7] = 0x04;
      // Remaining bytes don't matter.
      wire[12] = etl::someip::PROTOCOL_VERSION;
      wire[14] = etl::someip::REQUEST;

      transport.send(wire, 16);

      SomeipCalculator calc_impl;
      CalcServer       server(transport, calc_impl, TEST_SERVICE_ID, TEST_IFACE_VER);

      // process() should return false (message dropped, not processed).
      bool processed = server.process();
      CHECK(!processed);

      // No error response sent.
      CHECK_EQUAL(0u, transport.available());
    }
  }
} // namespace

#endif // ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION
