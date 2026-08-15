#include "card_state_machine.h"
#include "test_support.h"

#include <array>
#include <vector>

using techmania::nfc::CardKind;
using techmania::nfc::CardStateMachine;
using techmania::nfc::DecodedReport;
using techmania::nfc::NativeEvent;

namespace {

DecodedReport card(CardKind kind, std::array<std::uint8_t, 8> id) {
    return {.valid = true, .removed = false, .kind = kind, .id = id};
}

std::vector<tm_nfc_event_type> types(const std::vector<NativeEvent>& events) {
    std::vector<tm_nfc_event_type> result;
    for (const auto& event : events) result.push_back(event.type);
    return result;
}

const auto felica_a = card(CardKind::Felica,
    {0x01,0x2E,0x5C,0xE0,0xE9,0xC6,0x77,0x78});
const auto mifare_b = card(CardKind::Mifare,
    {0xE0,0x04,0x6A,0xB9,0xA5,0x6C,0x26,0x81});

}

TEST_CASE("reader connection and disconnection are transition events") {
    CardStateMachine state;

    REQUIRE(types(state.reader_connected()) ==
        std::vector<tm_nfc_event_type>{TM_NFC_EVENT_READER_CONNECTED});
    REQUIRE(state.reader_connected().empty());
    REQUIRE(types(state.reader_disconnected()) ==
        std::vector<tm_nfc_event_type>{TM_NFC_EVENT_READER_DISCONNECTED});
    REQUIRE(state.reader_disconnected().empty());
}

TEST_CASE("held card emits once and a card swap removes before presenting") {
    CardStateMachine state;
    state.reader_connected();

    REQUIRE(types(state.observe(felica_a)) ==
        std::vector<tm_nfc_event_type>{TM_NFC_EVENT_CARD_PRESENT});
    REQUIRE(state.observe(felica_a).empty());
    REQUIRE(types(state.observe(mifare_b)) ==
        (std::vector<tm_nfc_event_type>{
            TM_NFC_EVENT_CARD_REMOVED,
            TM_NFC_EVENT_CARD_PRESENT
        }));
}

TEST_CASE("zero report removes once and malformed report changes nothing") {
    CardStateMachine state;
    state.reader_connected();
    state.observe(felica_a);

    REQUIRE(state.observe(DecodedReport{}).empty());
    REQUIRE(types(state.observe({.valid = true, .removed = true})) ==
        std::vector<tm_nfc_event_type>{TM_NFC_EVENT_CARD_REMOVED});
    REQUIRE(state.observe({.valid = true, .removed = true}).empty());
}

TEST_CASE("reader unplug removes a held card before disconnecting") {
    CardStateMachine state;
    state.reader_connected();
    state.observe(felica_a);

    REQUIRE(types(state.reader_disconnected()) ==
        (std::vector<tm_nfc_event_type>{
            TM_NFC_EVENT_CARD_REMOVED,
            TM_NFC_EVENT_READER_DISCONNECTED
        }));
}
