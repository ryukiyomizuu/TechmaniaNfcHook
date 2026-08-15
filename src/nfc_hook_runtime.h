#pragma once

#include "event_queue.h"
#include "event_source.h"
#include "techmania_nfc_hook.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

namespace techmania::nfc {

class Runtime {
public:
    explicit Runtime(std::unique_ptr<IEventSource> source);
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    std::int32_t start();
    void stop();
    std::int32_t poll(tm_nfc_event* event_out);
    std::int32_t reader_state() const;

private:
    void accept(const NativeEvent& event);

    std::unique_ptr<IEventSource> source_;
    EventQueue queue_;
    mutable std::mutex lifecycle_mutex_;
    std::jthread worker_;
    std::atomic<bool> running_{false};
    std::atomic<std::int32_t> reader_state_{TM_NFC_READER_STOPPED};
    std::atomic<std::uint32_t> sequence_{0};
};

}
