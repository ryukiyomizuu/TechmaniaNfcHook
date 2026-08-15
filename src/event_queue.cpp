#include "event_queue.h"

#include <algorithm>

namespace techmania::nfc {
namespace {

bool is_reader_state(tm_nfc_event_type type) {
    return type == TM_NFC_EVENT_READER_CONNECTED ||
           type == TM_NFC_EVENT_READER_DISCONNECTED;
}

}

EventQueue::EventQueue(std::size_t capacity)
    : capacity_(std::max<std::size_t>(capacity, 1)) {}

void EventQueue::push(const NativeEvent& event) {
    std::lock_guard lock(mutex_);

    if (!events_.empty() && is_reader_state(event.type) &&
        events_.back().type == event.type) {
        return;
    }

    if (events_.size() >= capacity_) {
        const auto discardable = std::find_if(events_.begin(), events_.end(),
            [](const NativeEvent& queued) {
                return !is_card_transition(queued);
            });
        if (discardable != events_.end()) {
            events_.erase(discardable);
        } else {
            events_.pop_front();
        }
    }

    events_.push_back(event);
}

bool EventQueue::try_pop(NativeEvent& event) {
    std::lock_guard lock(mutex_);
    if (events_.empty()) return false;
    event = events_.front();
    events_.pop_front();
    return true;
}

std::size_t EventQueue::size() const {
    std::lock_guard lock(mutex_);
    return events_.size();
}

}
