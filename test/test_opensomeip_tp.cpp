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

#if ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION

namespace
{
  using namespace etl::someip;

  SUITE(test_opensomeip_tp)
  {
    //-----------------------------------------------------------------------
    // tp_header: round-trip with more_segments=true
    //-----------------------------------------------------------------------
    TEST(test_tp_header_roundtrip_more)
    {
      tp_header              orig(0x12340, true); // offset must be multiple of 16
      etl::array<uint8_t, 4> buf{};
      orig.encode(buf.data());

      tp_header decoded;
      decoded.decode(buf.data());

      CHECK_EQUAL(0x12340u, decoded.offset);
      CHECK(decoded.more_segments);
    }

    //-----------------------------------------------------------------------
    // tp_header: round-trip with more_segments=false (last segment)
    //-----------------------------------------------------------------------
    TEST(test_tp_header_roundtrip_last)
    {
      tp_header              orig(0, false);
      etl::array<uint8_t, 4> buf{};
      orig.encode(buf.data());

      tp_header decoded;
      decoded.decode(buf.data());

      CHECK_EQUAL(0u, decoded.offset);
      CHECK(!decoded.more_segments);
    }

    //-----------------------------------------------------------------------
    // tp_header: verify raw wire bytes (big-endian layout)
    //-----------------------------------------------------------------------
    TEST(test_tp_header_wire_bytes)
    {
      // offset=0x00000570 (1392), more=true
      // wire = 0x00000570 | 0x01 = 0x00000571
      // big-endian: 00 00 05 71
      tp_header              h(1392, true);
      etl::array<uint8_t, 4> buf{};
      h.encode(buf.data());

      CHECK_EQUAL(0x00, buf[0]);
      CHECK_EQUAL(0x00, buf[1]);
      CHECK_EQUAL(0x05, buf[2]);
      CHECK_EQUAL(0x71, buf[3]);

      // offset=0, more=false → all zeros
      tp_header              h2(0, false);
      etl::array<uint8_t, 4> buf2{};
      h2.encode(buf2.data());
      CHECK_EQUAL(0x00, buf2[0]);
      CHECK_EQUAL(0x00, buf2[1]);
      CHECK_EQUAL(0x00, buf2[2]);
      CHECK_EQUAL(0x00, buf2[3]);
    }

    //-----------------------------------------------------------------------
    // tp_segmenter: 3000-byte payload → 3 segments (1392 + 1392 + 216)
    //-----------------------------------------------------------------------
    TEST(test_tp_segmenter_three_segments)
    {
      // Build a 3000-byte payload with known pattern
      etl::array<uint8_t, 3000> payload{};
      for (size_t i = 0; i < 3000; ++i)
      {
        payload[i] = static_cast<uint8_t>(i & 0xFF);
      }

      someip_header hdr;
      hdr.service_id        = 0x1234;
      hdr.method_id         = 0x0001;
      hdr.client_id         = 0x00FF;
      hdr.session_id        = 0x0042;
      hdr.protocol_version  = PROTOCOL_VERSION;
      hdr.interface_version = 0x01;
      hdr.message_type      = REQUEST;
      hdr.return_code       = E_OK;
      hdr.length            = static_cast<uint32_t>(8 + 3000);

      tp_segmenter<> seg;
      seg.reset(hdr, payload.data(), payload.size());

      CHECK(seg.has_next());

      // Segment 1: offset=0, more=true, data=1392 bytes
      etl::array<uint8_t, 1500> buf{};
      size_t                    n1 = seg.next_segment(buf.data(), buf.size());
      // SOME/IP header(16) + TP header(4) + 1392 = 1412
      CHECK_EQUAL(1412u, n1);

      // Decode SOME/IP header
      someip_header seg1_hdr;
      seg1_hdr.decode(buf.data());
      CHECK_EQUAL(0x0042, seg1_hdr.session_id);
      // TP flag should be set
      CHECK((static_cast<uint8_t>(seg1_hdr.message_type) & TP_FLAG) != 0);
      // Length = 8 + 4 + 1392 = 1404
      CHECK_EQUAL(1404u, seg1_hdr.length);

      // Decode TP header at offset 16
      tp_header tp1;
      tp1.decode(buf.data() + HEADER_SIZE);
      CHECK_EQUAL(0u, tp1.offset);
      CHECK(tp1.more_segments);

      // Segment 2: offset=1392, more=true
      size_t n2 = seg.next_segment(buf.data(), buf.size());
      CHECK_EQUAL(1412u, n2);

      tp_header tp2;
      tp2.decode(buf.data() + HEADER_SIZE);
      CHECK_EQUAL(1392u, tp2.offset);
      CHECK(tp2.more_segments);

      // Segment 3: offset=2784, more=false, data=216
      size_t n3 = seg.next_segment(buf.data(), buf.size());
      // 16 + 4 + 216 = 236
      CHECK_EQUAL(236u, n3);

      tp_header tp3;
      tp3.decode(buf.data() + HEADER_SIZE);
      CHECK_EQUAL(2784u, tp3.offset);
      CHECK(!tp3.more_segments);

      CHECK(!seg.has_next());
    }

    //-----------------------------------------------------------------------
    // tp_segmenter: small payload → single segment
    //-----------------------------------------------------------------------
    TEST(test_tp_segmenter_single_segment)
    {
      etl::array<uint8_t, 100> payload{};
      for (size_t i = 0; i < 100; ++i)
      {
        payload[i] = static_cast<uint8_t>(i);
      }

      someip_header hdr;
      hdr.service_id   = 0x1234;
      hdr.method_id    = 0x0001;
      hdr.session_id   = 0x0001;
      hdr.message_type = REQUEST;
      hdr.length       = static_cast<uint32_t>(8 + 100);

      tp_segmenter<> seg;
      seg.reset(hdr, payload.data(), 100);

      CHECK(seg.has_next());

      etl::array<uint8_t, 256> buf{};
      size_t                   n = seg.next_segment(buf.data(), buf.size());
      // 16 + 4 + 100 = 120
      CHECK_EQUAL(120u, n);

      tp_header tp;
      tp.decode(buf.data() + HEADER_SIZE);
      CHECK_EQUAL(0u, tp.offset);
      CHECK(!tp.more_segments);

      CHECK(!seg.has_next());
    }

    //-----------------------------------------------------------------------
    // tp_reassembler: segment → reassemble → verify payload matches
    //-----------------------------------------------------------------------
    TEST(test_tp_reassembler_happy_path)
    {
      // Original payload
      etl::array<uint8_t, 3000> payload{};
      for (size_t i = 0; i < 3000; ++i)
      {
        payload[i] = static_cast<uint8_t>(i & 0xFF);
      }

      someip_header hdr;
      hdr.service_id        = 0x1234;
      hdr.method_id         = 0x0001;
      hdr.client_id         = 0x00FF;
      hdr.session_id        = 0x0042;
      hdr.protocol_version  = PROTOCOL_VERSION;
      hdr.interface_version = 0x01;
      hdr.message_type      = REQUEST;
      hdr.return_code       = E_OK;
      hdr.length            = static_cast<uint32_t>(8 + 3000);

      // Segment
      tp_segmenter<> seg;
      seg.reset(hdr, payload.data(), payload.size());

      tp_reassembler<4096> reasm;

      etl::array<uint8_t, 1500> buf{};
      tp_reassembly_status_t    status;

      // Segment 1
      size_t        n = seg.next_segment(buf.data(), buf.size());
      someip_header s1;
      s1.decode(buf.data());
      tp_header t1;
      t1.decode(buf.data() + HEADER_SIZE);
      status = reasm.process_segment(s1, t1, buf.data() + HEADER_SIZE + TP_HEADER_SIZE, n - HEADER_SIZE - TP_HEADER_SIZE);
      CHECK_EQUAL(static_cast<uint8_t>(TP_INCOMPLETE), static_cast<uint8_t>(status));

      // Segment 2
      n = seg.next_segment(buf.data(), buf.size());
      someip_header s2;
      s2.decode(buf.data());
      tp_header t2;
      t2.decode(buf.data() + HEADER_SIZE);
      status = reasm.process_segment(s2, t2, buf.data() + HEADER_SIZE + TP_HEADER_SIZE, n - HEADER_SIZE - TP_HEADER_SIZE);
      CHECK_EQUAL(static_cast<uint8_t>(TP_INCOMPLETE), static_cast<uint8_t>(status));

      // Segment 3 (last)
      n = seg.next_segment(buf.data(), buf.size());
      someip_header s3;
      s3.decode(buf.data());
      tp_header t3;
      t3.decode(buf.data() + HEADER_SIZE);
      status = reasm.process_segment(s3, t3, buf.data() + HEADER_SIZE + TP_HEADER_SIZE, n - HEADER_SIZE - TP_HEADER_SIZE);
      CHECK_EQUAL(static_cast<uint8_t>(TP_COMPLETE), static_cast<uint8_t>(status));

      // Verify reassembled payload
      CHECK_EQUAL(3000u, reasm.total_length());
      const uint8_t* result = reasm.payload();
      for (size_t i = 0; i < 3000; ++i)
      {
        CHECK_EQUAL(payload[i], result[i]);
      }

      // Verify TP flag is cleared in reassembled header
      CHECK((static_cast<uint8_t>(reasm.header().message_type) & TP_FLAG) == 0);
    }

    //-----------------------------------------------------------------------
    // tp_reassembler: segment with more=true but non-mod-16 length → ERROR
    //-----------------------------------------------------------------------
    TEST(test_tp_reassembler_bad_alignment)
    {
      someip_header hdr;
      hdr.service_id   = 0x1234;
      hdr.method_id    = 0x0001;
      hdr.session_id   = 0x0001;
      hdr.message_type = static_cast<message_type_t>(static_cast<uint8_t>(REQUEST) | TP_FLAG);

      tp_header tp(0, true); // more_segments=true

      // 100 bytes is NOT a multiple of 16 — should fail
      etl::array<uint8_t, 100> data{};

      tp_reassembler<4096>   reasm;
      tp_reassembly_status_t status = reasm.process_segment(hdr, tp, data.data(), 100);
      CHECK_EQUAL(static_cast<uint8_t>(TP_ERROR), static_cast<uint8_t>(status));
    }

    //-----------------------------------------------------------------------
    // tp_reassembler: new session ID restarts reassembly
    //-----------------------------------------------------------------------
    TEST(test_tp_reassembler_session_reset)
    {
      someip_header hdr;
      hdr.service_id   = 0x1234;
      hdr.method_id    = 0x0001;
      hdr.session_id   = 0x0001;
      hdr.message_type = static_cast<message_type_t>(static_cast<uint8_t>(REQUEST) | TP_FLAG);

      // Feed first segment of session 1
      tp_header                 tp1(0, true);
      etl::array<uint8_t, 1392> data1{};
      for (size_t i = 0; i < 1392; ++i)
      {
        data1[i] = 0xAA;
      }

      tp_reassembler<4096>   reasm;
      tp_reassembly_status_t s = reasm.process_segment(hdr, tp1, data1.data(), 1392);
      CHECK_EQUAL(static_cast<uint8_t>(TP_INCOMPLETE), static_cast<uint8_t>(s));
      CHECK_EQUAL(0x0001, reasm.session_id());

      // Now send segment with different session — should restart
      hdr.session_id = 0x0002;
      tp_header               tp_new(0, false); // last segment of new message (small)
      etl::array<uint8_t, 48> data2{};
      for (size_t i = 0; i < 48; ++i)
      {
        data2[i] = 0xBB;
      }

      s = reasm.process_segment(hdr, tp_new, data2.data(), 48);
      CHECK_EQUAL(static_cast<uint8_t>(TP_COMPLETE), static_cast<uint8_t>(s));
      CHECK_EQUAL(0x0002, reasm.session_id());
      CHECK_EQUAL(48u, reasm.total_length());

      // Verify data is from session 2
      CHECK_EQUAL(0xBB, reasm.payload()[0]);
    }

    //-----------------------------------------------------------------------
    // tp_reassembly_pool: two interleaved flows both complete
    //-----------------------------------------------------------------------
    TEST(test_tp_pool_two_flows)
    {
      // Flow A: service 0x1234, 64-byte payload (1 segment each for simplicity)
      // Flow B: service 0x5678, 48-byte payload
      someip_header hdr_a;
      hdr_a.service_id   = 0x1234;
      hdr_a.method_id    = 0x0001;
      hdr_a.client_id    = 0x0001;
      hdr_a.session_id   = 0x0010;
      hdr_a.message_type = static_cast<message_type_t>(static_cast<uint8_t>(REQUEST) | TP_FLAG);

      someip_header hdr_b;
      hdr_b.service_id   = 0x5678;
      hdr_b.method_id    = 0x0002;
      hdr_b.client_id    = 0x0002;
      hdr_b.session_id   = 0x0020;
      hdr_b.message_type = static_cast<message_type_t>(static_cast<uint8_t>(REQUEST) | TP_FLAG);

      tp_reassembly_pool<4, 4096> pool;

      // Interleave: A segment, B segment
      etl::array<uint8_t, 64> data_a{};
      for (size_t i = 0; i < 64; ++i)
      {
        data_a[i] = 0xAA;
      }

      etl::array<uint8_t, 48> data_b{};
      for (size_t i = 0; i < 48; ++i)
      {
        data_b[i] = 0xBB;
      }

      // A: last segment
      auto* ra = pool.find_or_create(hdr_a);
      CHECK(ra != nullptr);
      tp_header tp_a(0, false);
      auto      sa = ra->process_segment(hdr_a, tp_a, data_a.data(), 64);
      CHECK_EQUAL(static_cast<uint8_t>(TP_COMPLETE), static_cast<uint8_t>(sa));

      // B: last segment
      auto* rb = pool.find_or_create(hdr_b);
      CHECK(rb != nullptr);
      tp_header tp_b(0, false);
      auto      sb = rb->process_segment(hdr_b, tp_b, data_b.data(), 48);
      CHECK_EQUAL(static_cast<uint8_t>(TP_COMPLETE), static_cast<uint8_t>(sb));

      // Verify payloads
      CHECK_EQUAL(0xAA, ra->payload()[0]);
      CHECK_EQUAL(64u, ra->total_length());
      CHECK_EQUAL(0xBB, rb->payload()[0]);
      CHECK_EQUAL(48u, rb->total_length());
    }

    //-----------------------------------------------------------------------
    // tp_reassembly_pool: timeout evicts stale flows
    //-----------------------------------------------------------------------
    TEST(test_tp_pool_timeout_eviction)
    {
      tp_reassembly_pool<2, 4096> pool;
      pool.timeout_ms = 1000;

      someip_header hdr;
      hdr.service_id   = 0x1234;
      hdr.method_id    = 0x0001;
      hdr.client_id    = 0x0001;
      hdr.session_id   = 0x0001;
      hdr.message_type = static_cast<message_type_t>(static_cast<uint8_t>(REQUEST) | TP_FLAG);

      auto* r = pool.find_or_create(hdr);
      CHECK(r != nullptr);

      // Feed one segment to make it active
      tp_header               tp(0, true);
      etl::array<uint8_t, 16> data{};
      r->process_segment(hdr, tp, data.data(), 16);

      // Tick 999ms — still alive
      pool.tick(999);
      CHECK(pool.slots[0].in_use);

      // Tick 1 more ms — evicted
      pool.tick(1);
      CHECK(!pool.slots[0].in_use);
    }
  }
} // namespace

#endif // ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION
