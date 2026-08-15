#include "nfc_hook_runtime.h"
#include "windows_cardio_reader.h"

#include <memory>

namespace {

techmania::nfc::Runtime& runtime_instance() {
    static techmania::nfc::Runtime runtime(
        std::make_unique<techmania::nfc::WindowsCardIoReader>());
    return runtime;
}

}

extern "C" TM_NFC_API uint32_t TM_NFC_CALL tm_nfc_get_abi_version(void) {
    return TM_NFC_ABI_VERSION;
}

extern "C" TM_NFC_API int32_t TM_NFC_CALL tm_nfc_start(void) {
    return runtime_instance().start();
}

extern "C" TM_NFC_API void TM_NFC_CALL tm_nfc_stop(void) {
    runtime_instance().stop();
}

extern "C" TM_NFC_API int32_t TM_NFC_CALL tm_nfc_poll(
    tm_nfc_event* event_out) {
    return runtime_instance().poll(event_out);
}

extern "C" TM_NFC_API int32_t TM_NFC_CALL tm_nfc_get_reader_state(void) {
    return runtime_instance().reader_state();
}
