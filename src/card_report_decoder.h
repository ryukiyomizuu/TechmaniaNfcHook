#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace techmania::nfc {

enum class CardKind {
    None,
    Mifare,
    Felica,
    Other
};

struct DecodedReport {
    bool valid = false;
    bool removed = false;
    CardKind kind = CardKind::None;
    std::array<std::uint8_t, 8> id{};
};

DecodedReport decode_cardio_report(std::span<const std::uint8_t> report);

}
