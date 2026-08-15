#include "card_report_decoder.h"
#include "test_support.h"

#include <array>

using techmania::nfc::CardKind;
using techmania::nfc::decode_cardio_report;

TEST_CASE("FeliCa report preserves its eight-byte IDm") {
    const std::array<std::uint8_t, 9> report{
        2, 0x01, 0x2E, 0x5C, 0xE0, 0xE9, 0xC6, 0x77, 0x78
    };

    const auto decoded = decode_cardio_report(report);

    REQUIRE(decoded.valid);
    REQUIRE(!decoded.removed);
    REQUIRE(decoded.kind == CardKind::Felica);
    REQUIRE(decoded.id == (std::array<std::uint8_t, 8>{
        0x01, 0x2E, 0x5C, 0xE0, 0xE9, 0xC6, 0x77, 0x78
    }));
}

TEST_CASE("MIFARE report preserves the firmware CardIO identity") {
    const std::array<std::uint8_t, 9> report{
        1, 0xE0, 0x04, 0x6A, 0xB9, 0xA5, 0x6C, 0x26, 0x81
    };

    const auto decoded = decode_cardio_report(report);

    REQUIRE(decoded.valid);
    REQUIRE(!decoded.removed);
    REQUIRE(decoded.kind == CardKind::Mifare);
    REQUIRE(decoded.id == (std::array<std::uint8_t, 8>{
        0xE0, 0x04, 0x6A, 0xB9, 0xA5, 0x6C, 0x26, 0x81
    }));
}

TEST_CASE("all-zero report is a card removal") {
    const std::array<std::uint8_t, 9> report{};

    const auto decoded = decode_cardio_report(report);

    REQUIRE(decoded.valid);
    REQUIRE(decoded.removed);
    REQUIRE(decoded.kind == CardKind::None);
}

TEST_CASE("unknown report IDs and wrong lengths are rejected") {
    const std::array<std::uint8_t, 9> unknown{
        7, 0x01, 0x2E, 0x5C, 0xE0, 0xE9, 0xC6, 0x77, 0x78
    };
    const std::array<std::uint8_t, 8> short_report{
        2, 0x01, 0x2E, 0x5C, 0xE0, 0xE9, 0xC6, 0x77
    };

    REQUIRE(!decode_cardio_report(unknown).valid);
    REQUIRE(!decode_cardio_report(short_report).valid);
}
