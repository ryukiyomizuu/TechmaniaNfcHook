#include "nfc_hook_runtime.h"
#include "test_support.h"

#include <array>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

using namespace std::chrono_literals;
using techmania::nfc::CardKind;
using techmania::nfc::IEventSource;
using techmania::nfc::NativeEvent;
using techmania::nfc::Runtime;

namespace {

class ScriptedEventSource final : public IEventSource {
public:
    explicit ScriptedEventSource(std::vector<NativeEvent> events)
        : events_(std::move(events)) {}

    void run(std::stop_token, const Sink& sink) override {
        for (const auto& event : events_) sink(event);
    }

private:
    std::vector<NativeEvent> events_;
};

tm_nfc_event empty_event() {
    tm_nfc_event event{};
    event.struct_size = sizeof(event);
    event.abi_version = TM_NFC_ABI_VERSION;
    return event;
}

bool poll_until(Runtime& runtime, tm_nfc_event& event) {
    for (int attempt = 0; attempt < 100; ++attempt) {
        if (runtime.poll(&event) == 1) return true;
        std::this_thread::sleep_for(2ms);
    }
    return false;
}

}

TEST_CASE("public event ABI has the documented size and version") {
    REQUIRE(sizeof(tm_nfc_event) == 60);
    REQUIRE(TM_NFC_ABI_VERSION == 0x00010000u);
}

TEST_CASE("runtime start and stop are idempotent") {
    Runtime runtime(std::make_unique<ScriptedEventSource>(
        std::vector<NativeEvent>{}));

    REQUIRE(runtime.start() == TM_NFC_OK);
    REQUIRE(runtime.start() == TM_NFC_OK);
    runtime.stop();
    runtime.stop();
    REQUIRE(runtime.reader_state() == TM_NFC_READER_STOPPED);
}

TEST_CASE("runtime poll rejects null and incompatible caller structures") {
    Runtime runtime(std::make_unique<ScriptedEventSource>(
        std::vector<NativeEvent>{}));
    auto event = empty_event();

    REQUIRE(runtime.poll(nullptr) == TM_NFC_ERROR_INVALID_ARGUMENT);
    event.struct_size -= 1;
    REQUIRE(runtime.poll(&event) == TM_NFC_ERROR_INVALID_ARGUMENT);
    event = empty_event();
    event.abi_version = 0;
    REQUIRE(runtime.poll(&event) == TM_NFC_ERROR_ABI_MISMATCH);
}

TEST_CASE("runtime copies ordered native events into caller-owned ABI data") {
    const std::array<std::uint8_t, 8> id{
        0x01,0x2E,0x5C,0xE0,0xE9,0xC6,0x77,0x78
    };
    Runtime runtime(std::make_unique<ScriptedEventSource>(
        std::vector<NativeEvent>{
            {.type = TM_NFC_EVENT_READER_CONNECTED},
            {.type = TM_NFC_EVENT_CARD_PRESENT, .kind = CardKind::Felica, .id = id},
            {.type = TM_NFC_EVENT_CARD_REMOVED, .kind = CardKind::Felica, .id = id}
        }));
    REQUIRE(runtime.start() == TM_NFC_OK);

    auto event = empty_event();
    REQUIRE(poll_until(runtime, event));
    REQUIRE(event.event_type == TM_NFC_EVENT_READER_CONNECTED);
    REQUIRE(poll_until(runtime, event));
    REQUIRE(event.event_type == TM_NFC_EVENT_CARD_PRESENT);
    REQUIRE(event.card_kind == TM_NFC_CARD_FELICA);
    REQUIRE(std::equal(id.begin(), id.end(), event.card_id));
    const auto present_sequence = event.sequence;
    REQUIRE(poll_until(runtime, event));
    REQUIRE(event.event_type == TM_NFC_EVENT_CARD_REMOVED);
    REQUIRE(event.sequence > present_sequence);
    runtime.stop();
}
