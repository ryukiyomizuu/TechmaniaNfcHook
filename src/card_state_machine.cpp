#include "card_state_machine.h"

namespace techmania::nfc {
namespace {

NativeEvent make_card_event(tm_nfc_event_type type, const DecodedReport& card) {
    return {.type = type, .kind = card.kind, .id = card.id};
}

bool same_card(const DecodedReport& left, const DecodedReport& right) {
    return left.kind == right.kind && left.id == right.id;
}

}

std::vector<NativeEvent> CardStateMachine::reader_connected() {
    if (connected_) return {};
    connected_ = true;
    return {{.type = TM_NFC_EVENT_READER_CONNECTED}};
}

std::vector<NativeEvent> CardStateMachine::reader_disconnected() {
    if (!connected_) return {};

    std::vector<NativeEvent> events;
    if (current_card_) {
        events.push_back(make_card_event(TM_NFC_EVENT_CARD_REMOVED, *current_card_));
        current_card_.reset();
    }
    connected_ = false;
    events.push_back({.type = TM_NFC_EVENT_READER_DISCONNECTED});
    return events;
}

std::vector<NativeEvent> CardStateMachine::observe(const DecodedReport& report) {
    if (!report.valid) return {};

    if (report.removed) {
        if (!current_card_) return {};
        const NativeEvent removed = make_card_event(
            TM_NFC_EVENT_CARD_REMOVED, *current_card_);
        current_card_.reset();
        return {removed};
    }

    if (current_card_ && same_card(*current_card_, report)) return {};

    std::vector<NativeEvent> events;
    if (current_card_) {
        events.push_back(make_card_event(TM_NFC_EVENT_CARD_REMOVED, *current_card_));
    }
    current_card_ = report;
    events.push_back(make_card_event(TM_NFC_EVENT_CARD_PRESENT, report));
    return events;
}

}
