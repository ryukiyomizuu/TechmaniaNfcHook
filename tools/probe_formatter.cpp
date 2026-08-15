#include "probe_formatter.h"

#include <Windows.h>
#include <bcrypt.h>

#include <array>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace techmania::nfc::probe {
namespace {

std::string card_kind_name(std::int32_t kind) {
    switch (kind) {
        case TM_NFC_CARD_MIFARE: return "MIFARE";
        case TM_NFC_CARD_FELICA: return "FeliCa";
        case TM_NFC_CARD_OTHER: return "other card";
        default: return "card";
    }
}

std::string fingerprint(const std::uint8_t* id, std::size_t size) {
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD object_size = 0;
    DWORD hash_size = 0;
    DWORD copied = 0;
    std::vector<UCHAR> object;
    std::vector<UCHAR> digest;

    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM,
                                    nullptr, 0) < 0) {
        return "unavailable";
    }

    const auto close_algorithm = [&] {
        BCryptCloseAlgorithmProvider(algorithm, 0);
    };
    if (BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH,
                          reinterpret_cast<PUCHAR>(&object_size),
                          sizeof(object_size), &copied, 0) < 0 ||
        BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH,
                          reinterpret_cast<PUCHAR>(&hash_size),
                          sizeof(hash_size), &copied, 0) < 0) {
        close_algorithm();
        return "unavailable";
    }

    object.resize(object_size);
    digest.resize(hash_size);
    if (BCryptCreateHash(algorithm, &hash, object.data(), object_size,
                         nullptr, 0, 0) < 0 ||
        BCryptHashData(hash, const_cast<PUCHAR>(id),
                       static_cast<ULONG>(size), 0) < 0 ||
        BCryptFinishHash(hash, digest.data(), hash_size, 0) < 0) {
        if (hash) BCryptDestroyHash(hash);
        close_algorithm();
        return "unavailable";
    }

    BCryptDestroyHash(hash);
    close_algorithm();

    std::ostringstream output;
    output << std::hex << std::uppercase << std::setfill('0');
    for (std::size_t index = 0; index < 4 && index < digest.size(); ++index) {
        output << std::setw(2) << static_cast<unsigned>(digest[index]);
    }
    return output.str();
}

}

std::string format_event(const tm_nfc_event& event) {
    switch (event.event_type) {
        case TM_NFC_EVENT_READER_CONNECTED:
            return "reader connected";
        case TM_NFC_EVENT_READER_DISCONNECTED:
            return "reader disconnected";
        case TM_NFC_EVENT_CARD_PRESENT:
            return card_kind_name(event.card_kind) + " present fingerprint=" +
                fingerprint(event.card_id, sizeof(event.card_id));
        case TM_NFC_EVENT_CARD_REMOVED:
            return card_kind_name(event.card_kind) + " removed fingerprint=" +
                fingerprint(event.card_id, sizeof(event.card_id));
        case TM_NFC_EVENT_ERROR:
            return "reader error code=" + std::to_string(event.error_code);
        default:
            return "no event";
    }
}

}
