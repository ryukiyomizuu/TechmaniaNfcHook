#include "probe_formatter.h"
#include "test_support.h"

#include <array>
#include <string>

using techmania::nfc::probe::format_event;

TEST_CASE("probe card output identifies family without exposing raw ID") {
    const std::array<std::uint8_t, 8> id{
        0x01,0x2E,0x5C,0xE0,0xE9,0xC6,0x77,0x78
    };
    tm_nfc_event event{};
    event.struct_size = sizeof(event);
    event.abi_version = TM_NFC_ABI_VERSION;
    event.sequence = 7;
    event.event_type = TM_NFC_EVENT_CARD_PRESENT;
    event.card_kind = TM_NFC_CARD_FELICA;
    std::copy(id.begin(), id.end(), event.card_id);

    const std::string output = format_event(event);

    REQUIRE(output.find("FeliCa") != std::string::npos);
    REQUIRE(output.find("present") != std::string::npos);
    REQUIRE(output.find("012E5CE0E9C67778") == std::string::npos);
    REQUIRE(output.find("fingerprint=") != std::string::npos);
}

TEST_CASE("probe formats reader and removal transitions") {
    tm_nfc_event event{};
    event.struct_size = sizeof(event);
    event.abi_version = TM_NFC_ABI_VERSION;
    event.event_type = TM_NFC_EVENT_READER_CONNECTED;
    REQUIRE(format_event(event) == "reader connected");

    event.event_type = TM_NFC_EVENT_CARD_REMOVED;
    event.card_kind = TM_NFC_CARD_MIFARE;
    REQUIRE(format_event(event).find("MIFARE removed") != std::string::npos);
}
