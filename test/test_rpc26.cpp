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
#include "etl/rpc26.h"
#include "etl/string.h"
#include "etl/vector.h"

#if ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION

//---------------------------------------------------------------------------
// Calculator service — the shared interface definition.
// Both caller and callee know this type. Its public non-special member
// functions define the RPC API.
//---------------------------------------------------------------------------
struct Calculator
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
// Simple struct types used to test reflection-based serialization.
//---------------------------------------------------------------------------
struct Point2D
{
  int x;
  int y;
};

struct Colour
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

/// A struct with a nested struct field — tests recursive serialization.
struct ColouredPoint
{
  Point2D position;
  Colour  colour;
};

/// A struct whose naive sizeof includes padding (char + int).
struct PaddedStruct
{
  char tag;
  int  value;
};

//---------------------------------------------------------------------------
// Enum type for serialization tests.
//---------------------------------------------------------------------------
enum class Direction : uint8_t
{
  NORTH = 0,
  EAST  = 1,
  SOUTH = 2,
  WEST  = 3
};

//---------------------------------------------------------------------------
// Struct containing an etl::array.
//---------------------------------------------------------------------------
struct ArrayHolder
{
  etl::array<int, 3> values;
};

//---------------------------------------------------------------------------
// Struct containing an etl::vector (dynamic-length container).
//---------------------------------------------------------------------------
struct VectorHolder
{
  etl::vector<int, 8> items;
};

//---------------------------------------------------------------------------
// Struct containing an etl::string.
//---------------------------------------------------------------------------
struct StringHolder
{
  etl::string<16> name;
};

//---------------------------------------------------------------------------
// Complex struct mixing containers, primitives, and nested structs.
//---------------------------------------------------------------------------
struct SensorReading
{
  uint16_t               sensor_id;
  Direction              direction;
  etl::array<int16_t, 4> samples;
  etl::string<8>         label;
};

//---------------------------------------------------------------------------
// ContainerService — an RPC service that uses container arguments.
//---------------------------------------------------------------------------
struct ContainerService
{
  /// Sum all elements of a vector.
  int sum_vector(VectorHolder vh)
  {
    int total = 0;
    for (size_t i = 0; i < vh.items.size(); ++i)
    {
      total += vh.items[i];
    }
    return total;
  }

  /// Concatenate two strings.
  StringHolder concat(StringHolder a, StringHolder b)
  {
    StringHolder result;
    result.name = a.name;
    result.name.append(b.name);
    return result;
  }

  /// Return a sensor reading (complex mixed struct).
  SensorReading make_reading(uint16_t id, Direction dir)
  {
    SensorReading r;
    r.sensor_id = id;
    r.direction = dir;
    r.samples   = etl::array<int16_t, 4>{{10, 20, 30, 40}};
    r.label.assign("test");
    return r;
  }
};

//---------------------------------------------------------------------------
// GeometryService — an RPC service that uses struct arguments and returns.
//---------------------------------------------------------------------------
struct GeometryService
{
  /// Returns the sum of two 2D points.
  Point2D add_points(Point2D a, Point2D b)
  {
    return Point2D{a.x + b.x, a.y + b.y};
  }

  /// Scales a point by an integer factor.
  Point2D scale(Point2D p, int factor)
  {
    return Point2D{p.x * factor, p.y * factor};
  }

  /// Returns a coloured point (tests nested struct return).
  ColouredPoint make_coloured(Point2D pos, Colour col)
  {
    return ColouredPoint{pos, col};
  }
};

//---------------------------------------------------------------------------
// Test configuration constants.
//---------------------------------------------------------------------------
static constexpr size_t MAX_PAYLOAD = 128; // bytes per message payload
static constexpr size_t QUEUE_DEPTH = 8;   // FIFO depth
static constexpr size_t MAX_PENDING = 4;   // max in-flight calls

//---------------------------------------------------------------------------
// Type aliases for convenience.
//---------------------------------------------------------------------------
using Channel = etl::rpc26::rpc_channel<MAX_PAYLOAD, QUEUE_DEPTH>;
using Server  = etl::rpc26::rpc_server<Calculator, MAX_PAYLOAD, QUEUE_DEPTH>;
using Client  = etl::rpc26::rpc_client<Calculator, MAX_PAYLOAD, QUEUE_DEPTH, MAX_PENDING>;

using GeoChannel = etl::rpc26::rpc_channel<MAX_PAYLOAD, QUEUE_DEPTH>;
using GeoServer  = etl::rpc26::rpc_server<GeometryService, MAX_PAYLOAD, QUEUE_DEPTH>;
using GeoClient  = etl::rpc26::rpc_client<GeometryService, MAX_PAYLOAD, QUEUE_DEPTH, MAX_PENDING>;

using CntChannel = etl::rpc26::rpc_channel<MAX_PAYLOAD, QUEUE_DEPTH>;
using CntServer  = etl::rpc26::rpc_server<ContainerService, MAX_PAYLOAD, QUEUE_DEPTH>;
using CntClient  = etl::rpc26::rpc_client<ContainerService, MAX_PAYLOAD, QUEUE_DEPTH, MAX_PENDING>;

using ContChannel = etl::rpc26::rpc_channel<MAX_PAYLOAD, QUEUE_DEPTH>;
using ContServer  = etl::rpc26::rpc_server<ContainerService, MAX_PAYLOAD, QUEUE_DEPTH>;
using ContClient  = etl::rpc26::rpc_client<ContainerService, MAX_PAYLOAD, QUEUE_DEPTH, MAX_PENDING>;

namespace
{
  SUITE(test_rpc26)
  {
    //*************************************************************************
    /// Test: basic async add call through the RPC framework.
    ///
    /// Simulates the RTOS pattern:
    ///   1. Caller thread: invoke add(3, 4), get a future.
    ///   2. Callee thread: server.process() picks up request, calls impl.
    ///   3. Caller thread: client.poll() picks up response, fulfils future.
    ///   4. Caller reads future.get() == 7.
    //*************************************************************************
    TEST(test_add_async)
    {
      // Shared channel (would be in shared memory for cross-process).
      Channel channel;

      // Callee side: implementation + server.
      Calculator calc_impl;
      Server     server(channel, calc_impl);

      // Caller side: client.
      Client client(channel);

      // --- Caller thread: initiate async call ---
      // Use reflection to automatically deduce func_index, return type,
      // and parameter types from the member function.
      // The shared_state is embedded inside the client's pending slot.
      auto fut = client.call<^^Calculator::add>(3, 4);

      // Future is valid but not yet ready.
      CHECK(fut.valid());
      CHECK(!fut.is_ready());

      // --- Callee thread: process the request ---
      bool processed = server.process();
      CHECK(processed);

      // --- Caller thread: poll for responses ---
      bool received = client.poll();
      CHECK(received);

      // Now the future should be ready with the result.
      CHECK(fut.is_ready());
      CHECK_EQUAL(7, fut.get());
    }

    //*************************************************************************
    /// Test: multiply call.
    //*************************************************************************
    TEST(test_multiply_async)
    {
      Channel    channel;
      Calculator calc_impl;
      Server     server(channel, calc_impl);
      Client     client(channel);

      auto fut = client.call<^^Calculator::multiply>(5, 6);

      CHECK(fut.valid());
      CHECK(!fut.is_ready());

      // Process on callee side.
      server.process();

      // Poll on caller side.
      client.poll();

      CHECK(fut.is_ready());
      CHECK_EQUAL(30, fut.get());
    }

    //*************************************************************************
    /// Test: multiple in-flight calls.
    //*************************************************************************
    TEST(test_multiple_inflight)
    {
      Channel    channel;
      Calculator calc_impl;
      Server     server(channel, calc_impl);
      Client     client(channel);

      auto fut_add = client.call<^^Calculator::add>(10, 20);
      auto fut_mul = client.call<^^Calculator::multiply>(3, 7);

      CHECK(!fut_add.is_ready());
      CHECK(!fut_mul.is_ready());

      // Process both on callee side (two calls to process).
      server.process();
      server.process();

      // Poll both on caller side.
      client.poll();
      client.poll();

      CHECK(fut_add.is_ready());
      CHECK_EQUAL(30, fut_add.get());

      CHECK(fut_mul.is_ready());
      CHECK_EQUAL(21, fut_mul.get());
    }

    //*************************************************************************
    /// Test: poll returns false when no response available.
    //*************************************************************************
    TEST(test_poll_empty)
    {
      Channel channel;
      Client  client(channel);

      CHECK(!client.poll());
    }

    //*************************************************************************
    /// Test: process returns false when no request available.
    //*************************************************************************
    TEST(test_process_empty)
    {
      Channel    channel;
      Calculator calc_impl;
      Server     server(channel, calc_impl);

      CHECK(!server.process());
    }

    //*************************************************************************
    /// Test: serialization round-trip.
    //*************************************************************************
    TEST(test_serialization_roundtrip)
    {
      uint8_t buffer[64] = {};
      int     a          = 42;
      int     b          = -7;
      double  c          = 3.14;

      size_t written = etl::rpc26::serialize_args(buffer, a, b, c);
      CHECK_EQUAL(sizeof(int) + sizeof(int) + sizeof(double), written);

      auto result = etl::rpc26::detail::deserialize_args<int, int, double>(buffer);
      CHECK_EQUAL(42, etl::get<0>(result));
      CHECK_EQUAL(-7, etl::get<1>(result));
      CHECK_CLOSE(3.14, etl::get<2>(result), 0.001);
    }

    //*************************************************************************
    /// Test: serialize/deserialize a simple struct (Point2D).
    //*************************************************************************
    TEST(test_struct_serialization_roundtrip)
    {
      uint8_t buffer[64] = {};
      Point2D original{100, -200};

      size_t written = etl::rpc26::detail::serialize_one(buffer, 0, original);

      // Should be exactly 2 * sizeof(int), no padding.
      CHECK_EQUAL(2 * sizeof(int), written);

      Point2D restored{};
      size_t  read = etl::rpc26::detail::deserialize_one(buffer, 0, restored);
      CHECK_EQUAL(written, read);
      CHECK_EQUAL(100, restored.x);
      CHECK_EQUAL(-200, restored.y);
    }

    //*************************************************************************
    /// Test: serialize/deserialize a nested struct (ColouredPoint).
    //*************************************************************************
    TEST(test_nested_struct_serialization)
    {
      uint8_t       buffer[64] = {};
      ColouredPoint original;
      original.position.x = 10;
      original.position.y = 20;
      original.colour.r   = 255;
      original.colour.g   = 128;
      original.colour.b   = 0;

      size_t written = etl::rpc26::detail::serialize_one(buffer, 0, original);

      // Point2D: 2 * sizeof(int).  Colour: 3 * sizeof(uint8_t).
      size_t expected = 2 * sizeof(int) + 3 * sizeof(uint8_t);
      CHECK_EQUAL(expected, written);

      ColouredPoint restored{};
      size_t        read = etl::rpc26::detail::deserialize_one(buffer, 0, restored);
      CHECK_EQUAL(written, read);
      CHECK_EQUAL(10, restored.position.x);
      CHECK_EQUAL(20, restored.position.y);
      CHECK_EQUAL(255, restored.colour.r);
      CHECK_EQUAL(128, restored.colour.g);
      CHECK_EQUAL(0, restored.colour.b);
    }

    //*************************************************************************
    /// Test: struct with padding — verify tight packing (no padding bytes
    /// in wire format).
    //*************************************************************************
    TEST(test_padded_struct_tight_serialization)
    {
      uint8_t      buffer[64] = {};
      PaddedStruct original;
      original.tag   = 'A';
      original.value = 12345;

      size_t written = etl::rpc26::detail::serialize_one(buffer, 0, original);

      // Tight: sizeof(char) + sizeof(int) = 1 + 4 = 5.
      // NOT sizeof(PaddedStruct) which may be 8 due to alignment padding.
      size_t tight_size = sizeof(char) + sizeof(int);
      CHECK_EQUAL(tight_size, written);

      PaddedStruct restored{};
      etl::rpc26::detail::deserialize_one(buffer, 0, restored);
      CHECK_EQUAL('A', restored.tag);
      CHECK_EQUAL(12345, restored.value);
    }

    //*************************************************************************
    /// Test: serialized_size_of computes tight size.
    //*************************************************************************
    TEST(test_serialized_size_of)
    {
      int i = 0;
      CHECK_EQUAL(sizeof(int), etl::rpc26::detail::serialized_size_of(i));

      Point2D pt{0, 0};
      CHECK_EQUAL(2 * sizeof(int), etl::rpc26::detail::serialized_size_of(pt));

      Colour col{0, 0, 0};
      CHECK_EQUAL(3 * sizeof(uint8_t), etl::rpc26::detail::serialized_size_of(col));

      ColouredPoint cp{};
      size_t        expected_cp = 2 * sizeof(int) + 3 * sizeof(uint8_t);
      CHECK_EQUAL(expected_cp, etl::rpc26::detail::serialized_size_of(cp));

      PaddedStruct ps{};
      CHECK_EQUAL(sizeof(char) + sizeof(int), etl::rpc26::detail::serialized_size_of(ps));
    }

    //*************************************************************************
    /// Test: serialize multiple args including a struct.
    //*************************************************************************
    TEST(test_serialize_args_with_struct)
    {
      uint8_t buffer[64] = {};
      int     flag       = 42;
      Point2D pt{10, 20};

      size_t written = etl::rpc26::serialize_args(buffer, flag, pt);
      CHECK_EQUAL(sizeof(int) + 2 * sizeof(int), written);

      // Deserialize manually: first an int, then a Point2D.
      auto args = etl::rpc26::detail::deserialize_args<int, Point2D>(buffer);
      CHECK_EQUAL(42, etl::get<0>(args));
      CHECK_EQUAL(10, etl::get<1>(args).x);
      CHECK_EQUAL(20, etl::get<1>(args).y);
    }

    //*************************************************************************
    /// Test: RPC call with struct argument and struct return (add_points).
    //*************************************************************************
    TEST(test_rpc_struct_add_points)
    {
      GeoChannel      channel;
      GeometryService geo_impl;
      GeoServer       server(channel, geo_impl);
      GeoClient       client(channel);

      // add_points is index 0 of GeometryService.
      Point2D a{3, 4};
      Point2D b{10, 20};
      auto    fut = client.call<^^GeometryService::add_points>(a, b);

      CHECK(fut.valid());
      CHECK(!fut.is_ready());

      server.process();
      client.poll();

      CHECK(fut.is_ready());
      Point2D result = fut.get();
      CHECK_EQUAL(13, result.x);
      CHECK_EQUAL(24, result.y);
    }

    //*************************************************************************
    /// Test: RPC call with mixed struct + primitive args (scale).
    //*************************************************************************
    TEST(test_rpc_struct_scale)
    {
      GeoChannel      channel;
      GeometryService geo_impl;
      GeoServer       server(channel, geo_impl);
      GeoClient       client(channel);

      // scale is index 1 of GeometryService.
      Point2D p{5, 7};
      auto    fut = client.call<^^GeometryService::scale>(p, 3);

      server.process();
      client.poll();

      CHECK(fut.is_ready());
      Point2D result = fut.get();
      CHECK_EQUAL(15, result.x);
      CHECK_EQUAL(21, result.y);
    }

    //*************************************************************************
    /// Test: RPC call returning a nested struct (make_coloured).
    //*************************************************************************
    TEST(test_rpc_nested_struct_return)
    {
      GeoChannel      channel;
      GeometryService geo_impl;
      GeoServer       server(channel, geo_impl);
      GeoClient       client(channel);

      // make_coloured is index 2 of GeometryService.
      Point2D pos{42, 99};
      Colour  col{255, 128, 0};
      auto    fut = client.call<^^GeometryService::make_coloured>(pos, col);

      server.process();
      client.poll();

      CHECK(fut.is_ready());
      ColouredPoint result = fut.get();
      CHECK_EQUAL(42, result.position.x);
      CHECK_EQUAL(99, result.position.y);
      CHECK_EQUAL(255, result.colour.r);
      CHECK_EQUAL(128, result.colour.g);
      CHECK_EQUAL(0, result.colour.b);
    }

    //*************************************************************************
    /// Test: enum serialization roundtrip.
    //*************************************************************************
    TEST(test_enum_serialization)
    {
      uint8_t   buffer[64] = {};
      Direction original   = Direction::SOUTH;

      size_t written = etl::rpc26::detail::serialize_one(buffer, 0, original);
      CHECK_EQUAL(sizeof(uint8_t), written);

      Direction restored = Direction::NORTH;
      etl::rpc26::detail::deserialize_one(buffer, 0, restored);
      CHECK_EQUAL(static_cast<int>(Direction::SOUTH), static_cast<int>(restored));
    }

    //*************************************************************************
    /// Test: etl::array serialization roundtrip.
    //*************************************************************************
    TEST(test_etl_array_serialization)
    {
      uint8_t            buffer[64] = {};
      etl::array<int, 3> original   = {{10, 20, 30}};

      size_t written = etl::rpc26::detail::serialize_one(buffer, 0, original);
      CHECK_EQUAL(3 * sizeof(int), written);

      etl::array<int, 3> restored = {{0, 0, 0}};
      size_t             read     = etl::rpc26::detail::deserialize_one(buffer, 0, restored);
      CHECK_EQUAL(written, read);
      CHECK_EQUAL(10, restored[0]);
      CHECK_EQUAL(20, restored[1]);
      CHECK_EQUAL(30, restored[2]);
    }

    //*************************************************************************
    /// Test: etl::array serialized_size_of.
    //*************************************************************************
    TEST(test_etl_array_size)
    {
      etl::array<int, 3> arr = {{0, 0, 0}};
      CHECK_EQUAL(3 * sizeof(int), etl::rpc26::detail::serialized_size_of(arr));
    }

    //*************************************************************************
    /// Test: C-style fixed array serialization roundtrip.
    //*************************************************************************
    TEST(test_c_array_serialization)
    {
      uint8_t buffer[64]  = {};
      int     original[4] = {1, 2, 3, 4};

      size_t written = etl::rpc26::detail::serialize_one(buffer, 0, original);
      CHECK_EQUAL(4 * sizeof(int), written);

      int    restored[4] = {0, 0, 0, 0};
      size_t read        = etl::rpc26::detail::deserialize_one(buffer, 0, restored);
      CHECK_EQUAL(written, read);
      CHECK_EQUAL(1, restored[0]);
      CHECK_EQUAL(2, restored[1]);
      CHECK_EQUAL(3, restored[2]);
      CHECK_EQUAL(4, restored[3]);
    }

    //*************************************************************************
    /// Test: etl::vector serialization roundtrip (length-prefixed).
    //*************************************************************************
    TEST(test_etl_vector_serialization)
    {
      uint8_t             buffer[64] = {};
      etl::vector<int, 8> original;
      original.push_back(100);
      original.push_back(200);
      original.push_back(300);

      size_t written = etl::rpc26::detail::serialize_one(buffer, 0, original);
      // uint16_t prefix + 3 ints
      CHECK_EQUAL(sizeof(uint16_t) + 3 * sizeof(int), written);

      etl::vector<int, 8> restored;
      size_t              read = etl::rpc26::detail::deserialize_one(buffer, 0, restored);
      CHECK_EQUAL(written, read);
      CHECK_EQUAL(3U, restored.size());
      CHECK_EQUAL(100, restored[0]);
      CHECK_EQUAL(200, restored[1]);
      CHECK_EQUAL(300, restored[2]);
    }

    //*************************************************************************
    /// Test: etl::vector serialized_size_of.
    //*************************************************************************
    TEST(test_etl_vector_size)
    {
      etl::vector<int, 8> v;
      v.push_back(1);
      v.push_back(2);
      CHECK_EQUAL(sizeof(uint16_t) + 2 * sizeof(int), etl::rpc26::detail::serialized_size_of(v));
    }

    //*************************************************************************
    /// Test: etl::string serialization roundtrip.
    //*************************************************************************
    TEST(test_etl_string_serialization)
    {
      uint8_t         buffer[64] = {};
      etl::string<16> original("Hello");

      size_t written = etl::rpc26::detail::serialize_one(buffer, 0, original);
      // uint16_t prefix + 5 chars
      CHECK_EQUAL(sizeof(uint16_t) + 5, written);

      etl::string<16> restored;
      size_t          read = etl::rpc26::detail::deserialize_one(buffer, 0, restored);
      CHECK_EQUAL(written, read);
      CHECK_EQUAL(5U, restored.size());
      CHECK_EQUAL('H', restored[0]);
      CHECK_EQUAL('e', restored[1]);
      CHECK_EQUAL('l', restored[2]);
      CHECK_EQUAL('l', restored[3]);
      CHECK_EQUAL('o', restored[4]);
    }

    //*************************************************************************
    /// Test: etl::string serialized_size_of.
    //*************************************************************************
    TEST(test_etl_string_size)
    {
      etl::string<16> s("abc");
      CHECK_EQUAL(sizeof(uint16_t) + 3, etl::rpc26::detail::serialized_size_of(s));
    }

    //*************************************************************************
    /// Test: SensorReading — complex struct with enum, array, string.
    //*************************************************************************
    TEST(test_sensor_reading_serialization)
    {
      uint8_t       buffer[128] = {};
      SensorReading original;
      original.sensor_id = 42;
      original.direction = Direction::WEST;
      original.samples   = etl::array<int16_t, 4>{{100, -200, 300, -400}};
      original.label.assign("temp");

      size_t written = etl::rpc26::detail::serialize_one(buffer, 0, original);

      // Expected: uint16_t(2) + uint8_t(1) + 4*int16_t(8) + uint16_t(2) + 4 chars(4) = 17
      size_t expected = sizeof(uint16_t) + sizeof(uint8_t) + 4 * sizeof(int16_t) + sizeof(uint16_t) + 4;
      CHECK_EQUAL(expected, written);

      SensorReading restored;
      size_t        read = etl::rpc26::detail::deserialize_one(buffer, 0, restored);
      CHECK_EQUAL(written, read);
      CHECK_EQUAL(42, restored.sensor_id);
      CHECK_EQUAL(static_cast<int>(Direction::WEST), static_cast<int>(restored.direction));
      CHECK_EQUAL(100, restored.samples[0]);
      CHECK_EQUAL(-200, restored.samples[1]);
      CHECK_EQUAL(300, restored.samples[2]);
      CHECK_EQUAL(-400, restored.samples[3]);
      CHECK_EQUAL(4U, restored.label.size());
      CHECK_EQUAL('t', restored.label[0]);
      CHECK_EQUAL('e', restored.label[1]);
      CHECK_EQUAL('m', restored.label[2]);
      CHECK_EQUAL('p', restored.label[3]);
    }

    //*************************************************************************
    /// Test: RPC with etl::vector argument (sum_vector).
    //*************************************************************************
    TEST(test_rpc_sum_vector)
    {
      CntChannel       channel;
      ContainerService impl;
      CntServer        server(channel, impl);
      CntClient        client(channel);

      VectorHolder vh;
      vh.items.push_back(10);
      vh.items.push_back(20);
      vh.items.push_back(30);

      // sum_vector is index 0 of ContainerService.
      auto fut = client.call<^^ContainerService::sum_vector>(vh);

      server.process();
      client.poll();

      CHECK(fut.is_ready());
      CHECK_EQUAL(60, fut.get());
    }

    //*************************************************************************
    /// Test: RPC with etl::string argument and return (concat).
    //*************************************************************************
    TEST(test_rpc_concat_strings)
    {
      CntChannel       channel;
      ContainerService impl;
      CntServer        server(channel, impl);
      CntClient        client(channel);

      StringHolder a;
      a.name.assign("Hel");
      StringHolder b;
      b.name.assign("lo");

      // concat is index 1 of ContainerService.
      auto fut = client.call<^^ContainerService::concat>(a, b);

      server.process();
      client.poll();

      CHECK(fut.is_ready());
      StringHolder result = fut.get();
      CHECK_EQUAL(5U, result.name.size());
      CHECK_EQUAL('H', result.name[0]);
      CHECK_EQUAL('e', result.name[1]);
      CHECK_EQUAL('l', result.name[2]);
      CHECK_EQUAL('l', result.name[3]);
      CHECK_EQUAL('o', result.name[4]);
    }

    //*************************************************************************
    /// Test: RPC returning complex mixed struct (make_reading).
    //*************************************************************************
    TEST(test_rpc_make_reading)
    {
      CntChannel       channel;
      ContainerService impl;
      CntServer        server(channel, impl);
      CntClient        client(channel);

      // make_reading is index 2 of ContainerService.
      auto fut = client.call<^^ContainerService::make_reading>(static_cast<uint16_t>(7), Direction::EAST);

      server.process();
      client.poll();

      CHECK(fut.is_ready());
      SensorReading result = fut.get();
      CHECK_EQUAL(7, result.sensor_id);
      CHECK_EQUAL(static_cast<int>(Direction::EAST), static_cast<int>(result.direction));
      CHECK_EQUAL(10, result.samples[0]);
      CHECK_EQUAL(20, result.samples[1]);
      CHECK_EQUAL(30, result.samples[2]);
      CHECK_EQUAL(40, result.samples[3]);
      CHECK_EQUAL(4U, result.label.size());
      CHECK_EQUAL('t', result.label[0]);
    }

    //*************************************************************************
    /// Test: rpc_router — two clients, one server.
    ///
    /// Two clients send requests to the same Calculator server through
    /// a router. Responses must reach the correct client.
    //*************************************************************************
    TEST(test_router_two_clients_one_server)
    {
      using Router = etl::rpc26::rpc_router<MAX_PAYLOAD, QUEUE_DEPTH, 2, 1>;

      Router router;

      // Connect two clients and one server (service_id = 0).
      uint8_t cid_a    = 0;
      uint8_t cid_b    = 0;
      auto&   chan_a   = router.connect_client(cid_a);
      auto&   chan_b   = router.connect_client(cid_b);
      auto&   srv_chan = router.connect_server(0);

      Calculator calc_impl;
      Server     server(srv_chan, calc_impl);

      // Clients use service_id = 0, their assigned client_id.
      Client client_a(chan_a, 0, cid_a);
      Client client_b(chan_b, 0, cid_b);

      // Client A: add(10, 20), Client B: multiply(3, 7).
      auto fut_a = client_a.call<^^Calculator::add>(10, 20);
      auto fut_b = client_b.call<^^Calculator::multiply>(3, 7);

      CHECK(!fut_a.is_ready());
      CHECK(!fut_b.is_ready());

      // Route requests: client channels -> server channel.
      router.route();

      // Server processes both requests.
      server.process();
      server.process();

      // Route responses: server channel -> client channels.
      router.route();

      // Each client polls its own channel.
      client_a.poll();
      client_b.poll();

      CHECK(fut_a.is_ready());
      CHECK_EQUAL(30, fut_a.get());

      CHECK(fut_b.is_ready());
      CHECK_EQUAL(21, fut_b.get());
    }

    //*************************************************************************
    /// Test: rpc_router — one client, two servers (different services).
    ///
    /// A single client talks to both a Calculator (service 0) and a
    /// GeometryService (service 1) through the same router.
    //*************************************************************************
    TEST(test_router_one_client_two_servers)
    {
      using Router = etl::rpc26::rpc_router<MAX_PAYLOAD, QUEUE_DEPTH, 1, 2>;

      Router router;

      uint8_t cid         = 0;
      auto&   client_chan = router.connect_client(cid);
      auto&   calc_chan   = router.connect_server(0); // service_id = 0
      auto&   geo_chan    = router.connect_server(1); // service_id = 1

      Calculator calc_impl;
      Server     calc_server(calc_chan, calc_impl);

      GeometryService geo_impl;
      GeoServer       geo_server(geo_chan, geo_impl);

      // Client targeting Calculator (service 0).
      Client calc_client(client_chan, 0, cid);

      auto fut_add = calc_client.call<^^Calculator::add>(100, 200);

      router.route();
      calc_server.process();
      router.route();
      calc_client.poll();

      CHECK(fut_add.is_ready());
      CHECK_EQUAL(300, fut_add.get());

      // Same client channel but targeting GeometryService (service 1).
      // We need a client typed for GeometryService on the same channel.
      GeoClient geo_client(client_chan, 1, cid);

      Point2D pa{5, 10};
      Point2D pb{3, 7};
      auto    fut_geo = geo_client.call<^^GeometryService::add_points>(pa, pb);

      router.route();
      geo_server.process();
      router.route();
      geo_client.poll();

      CHECK(fut_geo.is_ready());
      Point2D result = fut_geo.get();
      CHECK_EQUAL(8, result.x);
      CHECK_EQUAL(17, result.y);
    }

    //*************************************************************************
    /// Test: rpc_router — two clients, two servers (full N:M).
    ///
    /// Both clients send to both servers through the same router.
    //*************************************************************************
    TEST(test_router_two_clients_two_servers)
    {
      using Router = etl::rpc26::rpc_router<MAX_PAYLOAD, QUEUE_DEPTH, 2, 2>;

      Router router;

      uint8_t cid_a = 0, cid_b = 0;
      auto&   chan_a    = router.connect_client(cid_a);
      auto&   chan_b    = router.connect_client(cid_b);
      auto&   calc_chan = router.connect_server(0);
      auto&   geo_chan  = router.connect_server(1);

      Calculator calc_impl;
      Server     calc_server(calc_chan, calc_impl);

      GeometryService geo_impl;
      GeoServer       geo_server(geo_chan, geo_impl);

      // Client A uses Calculator (service 0).
      Client client_a_calc(chan_a, 0, cid_a);
      auto   fut_a = client_a_calc.call<^^Calculator::multiply>(4, 5);

      // Client B uses GeometryService (service 1).
      GeoClient client_b_geo(chan_b, 1, cid_b);
      Point2D   p1{1, 2}, p2{3, 4};
      auto      fut_b = client_b_geo.call<^^GeometryService::add_points>(p1, p2);

      // Route, process, route, poll.
      router.route();
      calc_server.process();
      geo_server.process();
      router.route();
      client_a_calc.poll();
      client_b_geo.poll();

      CHECK(fut_a.is_ready());
      CHECK_EQUAL(20, fut_a.get());

      CHECK(fut_b.is_ready());
      Point2D r = fut_b.get();
      CHECK_EQUAL(4, r.x);
      CHECK_EQUAL(6, r.y);
    }

    //*************************************************************************
    /// Test: rpc_router route() returns 0 when idle.
    //*************************************************************************
    TEST(test_router_idle)
    {
      using Router = etl::rpc26::rpc_router<MAX_PAYLOAD, QUEUE_DEPTH, 1, 1>;
      Router router;
      CHECK_EQUAL(0U, router.route());
    }

    //*************************************************************************
    /// Test: rpc_mpmc_router — two clients, one server.
    //*************************************************************************
    TEST(test_mpmc_router_two_clients_one_server)
    {
      using Router = etl::rpc26::rpc_mpmc_router<MAX_PAYLOAD, QUEUE_DEPTH, 2, 1>;

      Router router;

      uint8_t cid_a    = 0;
      uint8_t cid_b    = 0;
      auto&   chan_a   = router.connect_client(cid_a);
      auto&   chan_b   = router.connect_client(cid_b);
      auto&   srv_chan = router.connect_server(0);

      Calculator calc_impl;
      Server     server(srv_chan, calc_impl);

      Client client_a(chan_a, 0, cid_a);
      Client client_b(chan_b, 0, cid_b);

      auto fut_a = client_a.call<^^Calculator::add>(10, 20);
      auto fut_b = client_b.call<^^Calculator::multiply>(3, 7);

      CHECK(!fut_a.is_ready());
      CHECK(!fut_b.is_ready());

      router.route();
      server.process();
      server.process();
      router.route();

      client_a.poll();
      client_b.poll();

      CHECK(fut_a.is_ready());
      CHECK_EQUAL(30, fut_a.get());

      CHECK(fut_b.is_ready());
      CHECK_EQUAL(21, fut_b.get());
    }

    //*************************************************************************
    /// Test: rpc_mpmc_router — one client, two servers.
    //*************************************************************************
    TEST(test_mpmc_router_one_client_two_servers)
    {
      using Router = etl::rpc26::rpc_mpmc_router<MAX_PAYLOAD, QUEUE_DEPTH, 1, 2>;

      Router router;

      uint8_t cid         = 0;
      auto&   client_chan = router.connect_client(cid);
      auto&   calc_chan   = router.connect_server(0);
      auto&   geo_chan    = router.connect_server(1);

      Calculator calc_impl;
      Server     calc_server(calc_chan, calc_impl);

      GeometryService geo_impl;
      GeoServer       geo_server(geo_chan, geo_impl);

      Client calc_client(client_chan, 0, cid);

      auto fut_add = calc_client.call<^^Calculator::add>(100, 200);

      router.route();
      calc_server.process();
      router.route();
      calc_client.poll();

      CHECK(fut_add.is_ready());
      CHECK_EQUAL(300, fut_add.get());

      GeoClient geo_client(client_chan, 1, cid);

      Point2D pa{5, 10};
      Point2D pb{3, 7};
      auto    fut_geo = geo_client.call<^^GeometryService::add_points>(pa, pb);

      router.route();
      geo_server.process();
      router.route();
      geo_client.poll();

      CHECK(fut_geo.is_ready());
      Point2D result = fut_geo.get();
      CHECK_EQUAL(8, result.x);
      CHECK_EQUAL(17, result.y);
    }

    //*************************************************************************
    /// Test: rpc_mpmc_router — two clients, two servers (full N:M).
    //*************************************************************************
    TEST(test_mpmc_router_two_clients_two_servers)
    {
      using Router = etl::rpc26::rpc_mpmc_router<MAX_PAYLOAD, QUEUE_DEPTH, 2, 2>;

      Router router;

      uint8_t cid_a = 0, cid_b = 0;
      auto&   chan_a    = router.connect_client(cid_a);
      auto&   chan_b    = router.connect_client(cid_b);
      auto&   calc_chan = router.connect_server(0);
      auto&   geo_chan  = router.connect_server(1);

      Calculator calc_impl;
      Server     calc_server(calc_chan, calc_impl);

      GeometryService geo_impl;
      GeoServer       geo_server(geo_chan, geo_impl);

      Client client_a_calc(chan_a, 0, cid_a);
      auto   fut_a = client_a_calc.call<^^Calculator::multiply>(4, 5);

      GeoClient client_b_geo(chan_b, 1, cid_b);
      Point2D   p1{1, 2}, p2{3, 4};
      auto      fut_b = client_b_geo.call<^^GeometryService::add_points>(p1, p2);

      router.route();
      calc_server.process();
      geo_server.process();
      router.route();
      client_a_calc.poll();
      client_b_geo.poll();

      CHECK(fut_a.is_ready());
      CHECK_EQUAL(20, fut_a.get());

      CHECK(fut_b.is_ready());
      Point2D r = fut_b.get();
      CHECK_EQUAL(4, r.x);
      CHECK_EQUAL(6, r.y);
    }

    //*************************************************************************
    /// Test: rpc_mpmc_router route() returns 0 when idle.
    //*************************************************************************
    TEST(test_mpmc_router_idle)
    {
      using Router = etl::rpc26::rpc_mpmc_router<MAX_PAYLOAD, QUEUE_DEPTH, 1, 1>;
      Router router;
      CHECK_EQUAL(0U, router.route());
    }
  }
} // namespace

#endif // ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION
