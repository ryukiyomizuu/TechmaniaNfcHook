#pragma once

#include "native_event.h"

#include <cstddef>
#include <deque>
#include <mutex>

namespace techmania::nfc {

class EventQueue {
public:
    explicit EventQueue(std::size_t capacity = 64);

    void push(const NativeEvent& event);
    bool try_pop(NativeEvent& event);
    std::size_t size() const;

private:
    std::size_t capacity_;
    mutable std::mutex mutex_;
    std::deque<NativeEvent> events_;
};

}
