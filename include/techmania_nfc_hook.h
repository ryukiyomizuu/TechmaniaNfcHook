#pragma once

#include <stdint.h>

#define TM_NFC_ABI_VERSION 0x00010000u

#if defined(_WIN32)
#define TM_NFC_CALL __cdecl
#if defined(TM_NFC_BUILD_DLL)
#define TM_NFC_API __declspec(dllexport)
#else
#define TM_NFC_API __declspec(dllimport)
#endif
#else
#define TM_NFC_CALL
#define TM_NFC_API
#endif

typedef enum tm_nfc_result {
    TM_NFC_OK = 0,
    TM_NFC_ERROR_INVALID_ARGUMENT = -1,
    TM_NFC_ERROR_ABI_MISMATCH = -2,
    TM_NFC_ERROR_INTERNAL = -3
} tm_nfc_result;

typedef enum tm_nfc_reader_state {
    TM_NFC_READER_STOPPED = 0,
    TM_NFC_READER_SEARCHING = 1,
    TM_NFC_READER_CONNECTED = 2,
    TM_NFC_READER_ERROR = 3
} tm_nfc_reader_state;

typedef enum tm_nfc_event_type {
    TM_NFC_EVENT_NONE = 0,
    TM_NFC_EVENT_READER_CONNECTED = 1,
    TM_NFC_EVENT_READER_DISCONNECTED = 2,
    TM_NFC_EVENT_CARD_PRESENT = 3,
    TM_NFC_EVENT_CARD_REMOVED = 4,
    TM_NFC_EVENT_ERROR = 5
} tm_nfc_event_type;

typedef enum tm_nfc_card_kind {
    TM_NFC_CARD_NONE = 0,
    TM_NFC_CARD_MIFARE = 1,
    TM_NFC_CARD_FELICA = 2,
    TM_NFC_CARD_OTHER = 3
} tm_nfc_card_kind;

#pragma pack(push, 1)
typedef struct tm_nfc_event {
    uint32_t struct_size;
    uint32_t abi_version;
    uint32_t sequence;
    int32_t event_type;
    int32_t card_kind;
    uint8_t card_id[8];
    int32_t error_code;
    uint8_t reserved[28];
} tm_nfc_event;
#pragma pack(pop)

#if defined(__cplusplus)
static_assert(sizeof(tm_nfc_event) == 60, "tm_nfc_event ABI size changed");
extern "C" {
#endif

TM_NFC_API uint32_t TM_NFC_CALL tm_nfc_get_abi_version(void);
TM_NFC_API int32_t TM_NFC_CALL tm_nfc_start(void);
TM_NFC_API void TM_NFC_CALL tm_nfc_stop(void);
TM_NFC_API int32_t TM_NFC_CALL tm_nfc_poll(tm_nfc_event* event_out);
TM_NFC_API int32_t TM_NFC_CALL tm_nfc_get_reader_state(void);

#if defined(__cplusplus)
}
#endif
