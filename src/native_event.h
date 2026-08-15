#pragma once

#include "card_report_decoder.h"
#include "techmania_nfc_hook.h"

#include <array>
#include <cstdint>

namespace techmania::nfc {

struct NativeEvent {
    tm_nfc_event_type type = TM_NFC_EVENT_NONE;
    CardKind kind = CardKind::None;
    std::array<std::uint8_t, 8> id{};
    std::int32_t error_code = 0;
};

inline bool is_card_transition(const NativeEvent& event) {
    return event.type == TM_NFC_EVENT_CARD_PRESENT ||
           event.type == TM_NFC_EVENT_CARD_REMOVED;
}

}
