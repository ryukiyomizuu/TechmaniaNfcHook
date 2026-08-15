#pragma once

#include "card_report_decoder.h"
#include "native_event.h"

#include <optional>
#include <vector>

namespace techmania::nfc {

class CardStateMachine {
public:
    std::vector<NativeEvent> reader_connected();
    std::vector<NativeEvent> reader_disconnected();
    std::vector<NativeEvent> observe(const DecodedReport& report);

private:
    bool connected_ = false;
    std::optional<DecodedReport> current_card_;
};

}
