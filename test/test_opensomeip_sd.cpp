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

  SUITE(test_opensomeip_sd)
  {
    //-----------------------------------------------------------------------
    // sd_entry: OfferService round-trip encode/decode
    //-----------------------------------------------------------------------
    TEST(test_sd_entry_offer_roundtrip)
    {
      sd_entry orig;
      orig.type              = SD_OFFER_SERVICE;
      orig.index_1st_options = 0;
      orig.index_2nd_options = 0;
      orig.num_opt_1         = 1;
      orig.num_opt_2         = 0;
      orig.service_id        = 0x1234;
      orig.instance_id       = 0x0001;
      orig.major_version     = 0x02;
      orig.ttl               = 3;
      orig.minor_version     = 0x00000005;

      etl::array<uint8_t, 16> buf{};
      orig.encode(buf.data());

      sd_entry decoded;
      decoded.decode(buf.data());

      CHECK_EQUAL(static_cast<uint8_t>(SD_OFFER_SERVICE), static_cast<uint8_t>(decoded.type));
      CHECK_EQUAL(0, decoded.index_1st_options);
      CHECK_EQUAL(0, decoded.index_2nd_options);
      CHECK_EQUAL(1, decoded.num_opt_1);
      CHECK_EQUAL(0, decoded.num_opt_2);
      CHECK_EQUAL(0x1234, decoded.service_id);
      CHECK_EQUAL(0x0001, decoded.instance_id);
      CHECK_EQUAL(0x02, decoded.major_version);
      CHECK_EQUAL(3u, decoded.ttl);
      CHECK_EQUAL(0x00000005u, decoded.minor_version);
      CHECK(!decoded.is_stop());
    }

    //-----------------------------------------------------------------------
    // sd_entry: SubscribeEventgroup round-trip
    //-----------------------------------------------------------------------
    TEST(test_sd_entry_subscribe_roundtrip)
    {
      sd_entry orig;
      orig.type              = SD_SUBSCRIBE_EVENTGROUP;
      orig.index_1st_options = 2;
      orig.index_2nd_options = 3;
      orig.num_opt_1         = 1;
      orig.num_opt_2         = 1;
      orig.service_id        = 0xABCD;
      orig.instance_id       = 0x0002;
      orig.major_version     = 0x01;
      orig.ttl               = SD_TTL_FOREVER;
      orig.counter           = 5;
      orig.eventgroup_id     = 0x0010;

      etl::array<uint8_t, 16> buf{};
      orig.encode(buf.data());

      sd_entry decoded;
      decoded.decode(buf.data());

      CHECK_EQUAL(static_cast<uint8_t>(SD_SUBSCRIBE_EVENTGROUP), static_cast<uint8_t>(decoded.type));
      CHECK_EQUAL(2, decoded.index_1st_options);
      CHECK_EQUAL(3, decoded.index_2nd_options);
      CHECK_EQUAL(1, decoded.num_opt_1);
      CHECK_EQUAL(1, decoded.num_opt_2);
      CHECK_EQUAL(0xABCD, decoded.service_id);
      CHECK_EQUAL(0x0002, decoded.instance_id);
      CHECK_EQUAL(0x01, decoded.major_version);
      CHECK_EQUAL(SD_TTL_FOREVER, decoded.ttl);
      CHECK_EQUAL(5, decoded.counter);
      CHECK_EQUAL(0x0010, decoded.eventgroup_id);
    }

    //-----------------------------------------------------------------------
    // sd_entry: StopOffer (TTL == 0)
    //-----------------------------------------------------------------------
    TEST(test_sd_entry_stop_offer)
    {
      sd_entry e;
      e.type          = SD_OFFER_SERVICE;
      e.service_id    = 0x1234;
      e.instance_id   = 0x0001;
      e.major_version = 1;
      e.ttl           = 0;
      e.minor_version = 0;

      CHECK(e.is_stop());

      etl::array<uint8_t, 16> buf{};
      e.encode(buf.data());

      sd_entry decoded;
      decoded.decode(buf.data());
      CHECK(decoded.is_stop());
      CHECK_EQUAL(0u, decoded.ttl);
    }

    //-----------------------------------------------------------------------
    // sd_ipv4_option: endpoint round-trip
    //-----------------------------------------------------------------------
    TEST(test_sd_ipv4_option_roundtrip)
    {
      sd_ipv4_option orig;
      orig.type         = SD_OPT_IPV4_ENDPOINT;
      orig.ipv4_address = etl::array<uint8_t, 4>{192, 168, 1, 42};
      orig.protocol     = SD_L4_UDP;
      orig.port         = 30490;

      etl::array<uint8_t, 12> buf{};
      orig.encode(buf.data());

      // Verify wire: length=0x0009, type=0x04
      CHECK_EQUAL(0x00, buf[0]);
      CHECK_EQUAL(0x09, buf[1]);
      CHECK_EQUAL(0x04, buf[2]);

      sd_ipv4_option decoded;
      decoded.decode(buf.data());

      CHECK_EQUAL(static_cast<uint8_t>(SD_OPT_IPV4_ENDPOINT), static_cast<uint8_t>(decoded.type));
      CHECK_EQUAL(192, decoded.ipv4_address[0]);
      CHECK_EQUAL(168, decoded.ipv4_address[1]);
      CHECK_EQUAL(1, decoded.ipv4_address[2]);
      CHECK_EQUAL(42, decoded.ipv4_address[3]);
      CHECK_EQUAL(static_cast<uint8_t>(SD_L4_UDP), static_cast<uint8_t>(decoded.protocol));
      CHECK_EQUAL(30490, decoded.port);
    }

    //-----------------------------------------------------------------------
    // sd_message: encode with entries + options, then decode
    //-----------------------------------------------------------------------
    TEST(test_sd_message_roundtrip)
    {
      sd_message<4, 4> msg;
      msg.header.set_reboot(true);
      msg.header.set_unicast(true);

      // Add an offer entry pointing to option index 0
      sd_entry offer;
      offer.type              = SD_OFFER_SERVICE;
      offer.index_1st_options = 0;
      offer.num_opt_1         = 1;
      offer.service_id        = 0x1234;
      offer.instance_id       = 0x0001;
      offer.major_version     = 1;
      offer.ttl               = 5;
      offer.minor_version     = 0;
      msg.add_entry(offer);

      // Add an IPv4 endpoint option
      sd_ipv4_option opt;
      opt.type         = SD_OPT_IPV4_ENDPOINT;
      opt.ipv4_address = etl::array<uint8_t, 4>{10, 0, 0, 1};
      opt.protocol     = SD_L4_TCP;
      opt.port         = 9999;
      msg.add_ipv4_option(opt);

      // Encode
      etl::array<uint8_t, 256> payload{};
      size_t                   len = msg.encode_payload(payload.data(), payload.size());

      // Expected: flags(4) + entries_len(4) + 1 entry(16) + options_len(4) + 1 option(12) = 40
      CHECK_EQUAL(40u, len);

      // Decode
      sd_message<4, 4> decoded;
      bool             ok = decoded.decode_payload(payload.data(), len);
      CHECK(ok);

      CHECK(decoded.header.reboot());
      CHECK(decoded.header.unicast());
      CHECK(!decoded.header.explicit_init_data());

      CHECK_EQUAL(1u, decoded.num_entries);
      CHECK_EQUAL(static_cast<uint8_t>(SD_OFFER_SERVICE), static_cast<uint8_t>(decoded.entries[0].type));
      CHECK_EQUAL(0x1234, decoded.entries[0].service_id);
      CHECK_EQUAL(0x0001, decoded.entries[0].instance_id);
      CHECK_EQUAL(5u, decoded.entries[0].ttl);

      CHECK_EQUAL(1u, decoded.num_ipv4_options);
      CHECK_EQUAL(static_cast<uint8_t>(SD_OPT_IPV4_ENDPOINT), static_cast<uint8_t>(decoded.ipv4_options[0].type));
      CHECK_EQUAL(10, decoded.ipv4_options[0].ipv4_address[0]);
      CHECK_EQUAL(0, decoded.ipv4_options[0].ipv4_address[1]);
      CHECK_EQUAL(0, decoded.ipv4_options[0].ipv4_address[2]);
      CHECK_EQUAL(1, decoded.ipv4_options[0].ipv4_address[3]);
      CHECK_EQUAL(9999, decoded.ipv4_options[0].port);
    }

    //-----------------------------------------------------------------------
    // Builder helpers
    //-----------------------------------------------------------------------
    TEST(test_make_find_entry)
    {
      sd_entry e = make_find_entry(0x1234);
      CHECK_EQUAL(static_cast<uint8_t>(SD_FIND_SERVICE), static_cast<uint8_t>(e.type));
      CHECK_EQUAL(0x1234, e.service_id);
      CHECK_EQUAL(SD_INSTANCE_ANY, e.instance_id);
      CHECK_EQUAL(SD_MAJOR_VERSION_ANY, e.major_version);
      CHECK_EQUAL(SD_MINOR_VERSION_ANY, e.minor_version);
      CHECK_EQUAL(3u, e.ttl);
    }

    TEST(test_make_offer_entry)
    {
      sd_entry e = make_offer_entry(0x1234, 0x0001, 1, 0, SD_TTL_FOREVER);
      CHECK_EQUAL(static_cast<uint8_t>(SD_OFFER_SERVICE), static_cast<uint8_t>(e.type));
      CHECK_EQUAL(0x1234, e.service_id);
      CHECK_EQUAL(0x0001, e.instance_id);
      CHECK_EQUAL(SD_TTL_FOREVER, e.ttl);
    }

    TEST(test_make_subscribe_entry)
    {
      sd_entry e = make_subscribe_entry(0x1234, 0x0001, 1, 0x0010, SD_TTL_FOREVER, 3);
      CHECK_EQUAL(static_cast<uint8_t>(SD_SUBSCRIBE_EVENTGROUP), static_cast<uint8_t>(e.type));
      CHECK_EQUAL(0x0010, e.eventgroup_id);
      CHECK_EQUAL(3, e.counter);
    }

    TEST(test_make_subscribe_ack_entry)
    {
      sd_entry e = make_subscribe_ack_entry(0x1234, 0x0001, 1, 0x0010, SD_TTL_FOREVER, 3);
      CHECK_EQUAL(static_cast<uint8_t>(SD_SUBSCRIBE_EVENTGROUP_ACK), static_cast<uint8_t>(e.type));
      CHECK_EQUAL(0x0010, e.eventgroup_id);
    }

    TEST(test_make_ipv4_endpoint)
    {
      etl::array<uint8_t, 4> addr = {192, 168, 0, 1};
      sd_ipv4_option         opt  = make_ipv4_endpoint(addr, SD_L4_TCP, 30500);
      CHECK_EQUAL(static_cast<uint8_t>(SD_OPT_IPV4_ENDPOINT), static_cast<uint8_t>(opt.type));
      CHECK_EQUAL(192, opt.ipv4_address[0]);
      CHECK_EQUAL(static_cast<uint8_t>(SD_L4_TCP), static_cast<uint8_t>(opt.protocol));
      CHECK_EQUAL(30500, opt.port);
    }

    TEST(test_make_ipv4_multicast)
    {
      etl::array<uint8_t, 4> addr = {239, 0, 0, 1};
      sd_ipv4_option         opt  = make_ipv4_multicast(addr, 30490);
      CHECK_EQUAL(static_cast<uint8_t>(SD_OPT_IPV4_MULTICAST), static_cast<uint8_t>(opt.type));
      CHECK_EQUAL(static_cast<uint8_t>(SD_L4_UDP), static_cast<uint8_t>(opt.protocol));
      CHECK_EQUAL(30490, opt.port);
    }

    //-----------------------------------------------------------------------
    // Server state machine: phase transitions
    //-----------------------------------------------------------------------
    TEST(test_sd_server_phase_transitions)
    {
      sd_service_instance svc;
      svc.service_id         = 0x1234;
      svc.instance_id        = 0x0001;
      svc.major_version      = 1;
      svc.minor_version      = 0;
      svc.initial_delay_ms   = 100;
      svc.repetition_base_ms = 200;
      svc.repetition_max     = 3;
      svc.main_cycle_ms      = 1000;

      CHECK_EQUAL(static_cast<uint8_t>(SD_PHASE_DOWN), static_cast<uint8_t>(svc.phase));
      CHECK(!svc.needs_offer());

      // Start
      svc.start();
      CHECK_EQUAL(static_cast<uint8_t>(SD_PHASE_INITIAL_WAIT), static_cast<uint8_t>(svc.phase));
      CHECK(!svc.needs_offer()); // timer not yet elapsed

      // Tick past initial delay
      svc.tick(100);
      CHECK(svc.needs_offer());
      svc.mark_offered();
      CHECK_EQUAL(static_cast<uint8_t>(SD_PHASE_REPETITION), static_cast<uint8_t>(svc.phase));

      // First repetition: 200ms
      CHECK(!svc.needs_offer());
      svc.tick(200);
      CHECK(svc.needs_offer());
      svc.mark_offered(); // rep_count=2, backoff=400

      // Second repetition: 400ms
      svc.tick(400);
      CHECK(svc.needs_offer());
      svc.mark_offered(); // rep_count=3 >= max → MAIN
      CHECK_EQUAL(static_cast<uint8_t>(SD_PHASE_MAIN), static_cast<uint8_t>(svc.phase));

      // Main cycle: 1000ms
      CHECK(!svc.needs_offer());
      svc.tick(1000);
      CHECK(svc.needs_offer());
      svc.mark_offered();
      CHECK_EQUAL(static_cast<uint8_t>(SD_PHASE_MAIN), static_cast<uint8_t>(svc.phase));
    }

    TEST(test_sd_server_make_offer)
    {
      sd_service_instance svc;
      svc.service_id    = 0xABCD;
      svc.instance_id   = 0x0002;
      svc.major_version = 3;
      svc.minor_version = 7;
      svc.ttl           = 5;

      sd_entry e = svc.make_offer();
      CHECK_EQUAL(static_cast<uint8_t>(SD_OFFER_SERVICE), static_cast<uint8_t>(e.type));
      CHECK_EQUAL(0xABCD, e.service_id);
      CHECK_EQUAL(5u, e.ttl);

      sd_entry stop = svc.make_stop_offer();
      CHECK(stop.is_stop());
    }

    //-----------------------------------------------------------------------
    // Client state machine: find + offer processing
    //-----------------------------------------------------------------------
    TEST(test_sd_client_find_and_offer)
    {
      sd_client_instance<4> cli;
      cli.service_id         = 0x1234;
      cli.initial_delay_ms   = 50;
      cli.repetition_base_ms = 100;
      cli.repetition_max     = 2;

      cli.start();
      CHECK_EQUAL(static_cast<uint8_t>(SD_CLIENT_INITIAL_WAIT), static_cast<uint8_t>(cli.phase));
      CHECK(!cli.needs_find());

      // Tick past initial delay
      cli.tick(50);
      CHECK(cli.needs_find());
      cli.mark_find_sent();
      CHECK_EQUAL(static_cast<uint8_t>(SD_CLIENT_REPETITION), static_cast<uint8_t>(cli.phase));

      // Receive an offer before next find
      sd_entry offer   = make_offer_entry(0x1234, 0x0001, 1, 0, 5);
      bool     matched = cli.process_offer(offer);
      CHECK(matched);
      CHECK_EQUAL(static_cast<uint8_t>(SD_CLIENT_SERVICE_READY), static_cast<uint8_t>(cli.phase));
      CHECK(cli.is_available());
      CHECK_EQUAL(1u, cli.num_found);
      CHECK_EQUAL(0x0001, cli.found[0].instance_id);
    }

    TEST(test_sd_client_stop_offer)
    {
      sd_client_instance<4> cli;
      cli.service_id = 0x1234;
      cli.start();
      cli.tick(500); // past default initial delay
      cli.mark_find_sent();

      // Receive offer
      sd_entry offer = make_offer_entry(0x1234, 0x0001, 1, 0, 10);
      cli.process_offer(offer);
      CHECK(cli.is_available());

      // Receive StopOffer
      sd_entry stop = make_offer_entry(0x1234, 0x0001, 1, 0, 0);
      cli.process_offer(stop);
      CHECK(!cli.is_available());
      // Should go back to searching
      CHECK_EQUAL(static_cast<uint8_t>(SD_CLIENT_REPETITION), static_cast<uint8_t>(cli.phase));
    }

    TEST(test_sd_client_ttl_expiry)
    {
      sd_client_instance<4> cli;
      cli.service_id = 0x1234;
      cli.start();
      cli.tick(500);
      cli.mark_find_sent();

      // Offer with TTL=2 seconds → 2000ms
      sd_entry offer = make_offer_entry(0x1234, 0x0001, 1, 0, 2);
      cli.process_offer(offer);
      CHECK(cli.is_available());

      // Tick 1999ms — still available
      cli.tick(1999);
      CHECK(cli.is_available());

      // Tick 1 more ms — expired
      cli.tick(1);
      CHECK(!cli.is_available());
    }

    //-----------------------------------------------------------------------
    // Reboot detection
    //-----------------------------------------------------------------------
    TEST(test_reboot_detector_first_contact)
    {
      sd_reboot_detector<4> det;
      // First contact is never a reboot
      bool rebooted = det.check(1, 0x0001, true);
      CHECK(!rebooted);
    }

    TEST(test_reboot_detector_normal_increment)
    {
      sd_reboot_detector<4> det;
      det.check(1, 0x0001, true);
      // Normal session increment — no reboot
      bool rebooted = det.check(1, 0x0002, true);
      CHECK(!rebooted);
    }

    TEST(test_reboot_detector_session_reset)
    {
      sd_reboot_detector<4> det;
      det.check(1, 0x0005, true);
      // Session goes backward with reboot flag still true → reboot
      bool rebooted = det.check(1, 0x0001, true);
      CHECK(rebooted);
    }

    TEST(test_reboot_detector_flag_transition)
    {
      sd_reboot_detector<4> det;
      det.check(1, 0x0001, false);
      // false → true transition = reboot
      bool rebooted = det.check(1, 0x0001, true);
      CHECK(rebooted);
    }

    TEST(test_reboot_detector_forget)
    {
      sd_reboot_detector<4> det;
      det.check(1, 0x0005, true);
      det.forget(1);
      // After forget, next contact is first contact again
      bool rebooted = det.check(1, 0x0001, true);
      CHECK(!rebooted);
    }
  }
} // namespace

#endif // ETL_HAS_REFLECTION && ETL_HAS_PARAMETER_REFLECTION
