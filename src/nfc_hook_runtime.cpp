#include "nfc_hook_runtime.h"

#include <algorithm>
#include <cstring>
#include <utility>

namespace techmania::nfc {
namespace {

std::int32_t public_kind(CardKind kind) {
    switch (kind) {
        case CardKind::Mifare: return TM_NFC_CARD_MIFARE;
        case CardKind::Felica: return TM_NFC_CARD_FELICA;
        case CardKind::Other: return TM_NFC_CARD_OTHER;
        case CardKind::None: return TM_NFC_CARD_NONE;
    }
    return TM_NFC_CARD_NONE;
}

}

Runtime::Runtime(std::unique_ptr<IEventSource> source)
    : source_(std::move(source)) {}

Runtime::~Runtime() {
    stop();
}

std::int32_t Runtime::start() {
    std::lock_guard lock(lifecycle_mutex_);
    if (running_.load()) return TM_NFC_OK;
    if (!source_) return TM_NFC_ERROR_INTERNAL;

    if (worker_.joinable()) worker_.join();

    reader_state_.store(TM_NFC_READER_SEARCHING);
    running_.store(true);
    worker_ = std::jthread([this](std::stop_token stop_token) {
        try {
            source_->run(stop_token, [this](const NativeEvent& event) {
                accept(event);
            });
        } catch (...) {
            accept({.type = TM_NFC_EVENT_ERROR,
                    .error_code = TM_NFC_ERROR_INTERNAL});
        }

        if (!stop_token.stop_requested()) {
            reader_state_.store(TM_NFC_READER_ERROR);
        }
        running_.store(false);
    });
    return TM_NFC_OK;
}

void Runtime::stop() {
    std::jthread worker;
    {
        std::lock_guard lock(lifecycle_mutex_);
        if (worker_.joinable()) {
            worker_.request_stop();
            worker = std::move(worker_);
        }
    }
    if (worker.joinable()) worker.join();
    running_.store(false);
    reader_state_.store(TM_NFC_READER_STOPPED);
}

std::int32_t Runtime::poll(tm_nfc_event* event_out) {
    if (!event_out || event_out->struct_size != sizeof(tm_nfc_event)) {
        return TM_NFC_ERROR_INVALID_ARGUMENT;
    }
    if (event_out->abi_version != TM_NFC_ABI_VERSION) {
        return TM_NFC_ERROR_ABI_MISMATCH;
    }

    NativeEvent event;
    if (!queue_.try_pop(event)) return 0;

    tm_nfc_event output{};
    output.struct_size = sizeof(output);
    output.abi_version = TM_NFC_ABI_VERSION;
    output.sequence = sequence_.fetch_add(1) + 1;
    output.event_type = event.type;
    output.card_kind = public_kind(event.kind);
    std::copy(event.id.begin(), event.id.end(), output.card_id);
    output.error_code = event.error_code;
    *event_out = output;
    return 1;
}

std::int32_t Runtime::reader_state() const {
    return reader_state_.load();
}

void Runtime::accept(const NativeEvent& event) {
    if (event.type == TM_NFC_EVENT_READER_CONNECTED) {
        reader_state_.store(TM_NFC_READER_CONNECTED);
    } else if (event.type == TM_NFC_EVENT_READER_DISCONNECTED) {
        reader_state_.store(TM_NFC_READER_SEARCHING);
    } else if (event.type == TM_NFC_EVENT_ERROR) {
        reader_state_.store(TM_NFC_READER_ERROR);
    }
    queue_.push(event);
}

}
