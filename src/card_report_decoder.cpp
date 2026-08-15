#include "card_report_decoder.h"

#include <algorithm>

namespace techmania::nfc {

DecodedReport decode_cardio_report(std::span<const std::uint8_t> report) {
    if (report.size() != 9) {
        return {};
    }

    if (std::all_of(report.begin(), report.end(), [](std::uint8_t value) {
            return value == 0;
        })) {
        return {.valid = true, .removed = true};
    }

    CardKind kind = CardKind::None;
    if (report[0] == 1) {
        kind = CardKind::Mifare;
    } else if (report[0] == 2) {
        kind = CardKind::Felica;
    } else {
        return {};
    }

    DecodedReport result{.valid = true, .removed = false, .kind = kind};
    std::copy_n(report.begin() + 1, result.id.size(), result.id.begin());
    return result;
}

}
