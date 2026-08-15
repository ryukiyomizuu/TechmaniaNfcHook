#include "probe_formatter.h"
#include "techmania_nfc_hook.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace {

int parse_duration(int argc, char** argv) {
    if (argc == 3 && std::string(argv[1]) == "--seconds") {
        try {
            const int value = std::stoi(argv[2]);
            if (value > 0 && value <= 3600) return value;
        } catch (...) {
        }
        std::cerr << "--seconds must be from 1 to 3600\n";
        return -1;
    }
    if (argc != 1) {
        std::cerr << "usage: TechmaniaNfcProbe [--seconds N]\n";
        return -1;
    }
    return 30;
}

}

int main(int argc, char** argv) {
    const int duration_seconds = parse_duration(argc, argv);
    if (duration_seconds < 0) return 64;
    if (tm_nfc_get_abi_version() != TM_NFC_ABI_VERSION) {
        std::cerr << "NFC hook ABI mismatch\n";
        return 3;
    }
    if (tm_nfc_start() != TM_NFC_OK) {
        std::cerr << "NFC hook failed to start\n";
        return 4;
    }

    std::cout << "TECHMANIA NFC hook probe; raw card IDs are redacted\n";
    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::seconds(duration_seconds);
    while (std::chrono::steady_clock::now() < deadline) {
        tm_nfc_event event{};
        event.struct_size = sizeof(event);
        event.abi_version = TM_NFC_ABI_VERSION;
        const int result = tm_nfc_poll(&event);
        if (result < 0) {
            std::cerr << "poll failed code=" << result << '\n';
            tm_nfc_stop();
            return 5;
        }
        if (result == 1) {
            std::cout << "#" << event.sequence << ' '
                      << techmania::nfc::probe::format_event(event) << '\n';
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    tm_nfc_stop();
    return 0;
}
