#include "windows_cardio_reader.h"

#include "card_report_decoder.h"
#include "card_state_machine.h"

#include <Windows.h>
#include <hidsdi.h>
#include <setupapi.h>

#include <array>
#include <cstdint>
#include <memory>
#include <stop_token>
#include <vector>

namespace techmania::nfc {
namespace {

constexpr USHORT kVendorId = 0xCAFF;
constexpr USHORT kProductId = 0x400E;
constexpr USAGE kUsagePage = 0xFFCA;
constexpr USAGE kUsage = 0x0001;
constexpr DWORD kRetryDelayMs = 250;

struct HandleCloser {
    void operator()(void* handle) const {
        if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
            CloseHandle(static_cast<HANDLE>(handle));
        }
    }
};

using UniqueHandle = std::unique_ptr<void, HandleCloser>;

struct DeviceConnection {
    UniqueHandle handle;
    USHORT input_report_length = 0;
};

bool matches_cardio(HANDLE handle, USHORT& input_report_length) {
    HIDD_ATTRIBUTES attributes{};
    attributes.Size = sizeof(attributes);
    if (!HidD_GetAttributes(handle, &attributes) ||
        attributes.VendorID != kVendorId || attributes.ProductID != kProductId) {
        return false;
    }

    PHIDP_PREPARSED_DATA preparsed = nullptr;
    if (!HidD_GetPreparsedData(handle, &preparsed)) return false;

    HIDP_CAPS caps{};
    const NTSTATUS status = HidP_GetCaps(preparsed, &caps);
    HidD_FreePreparsedData(preparsed);
    if (status != HIDP_STATUS_SUCCESS || caps.UsagePage != kUsagePage ||
        caps.Usage != kUsage || caps.InputReportByteLength < 9) {
        return false;
    }

    input_report_length = caps.InputReportByteLength;
    return true;
}

DeviceConnection open_cardio() {
    GUID hid_guid{};
    HidD_GetHidGuid(&hid_guid);
    HDEVINFO devices = SetupDiGetClassDevsW(
        &hid_guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (devices == INVALID_HANDLE_VALUE) return {};

    DeviceConnection result;
    SP_DEVICE_INTERFACE_DATA interface_data{};
    interface_data.cbSize = sizeof(interface_data);

    for (DWORD index = 0;
         SetupDiEnumDeviceInterfaces(devices, nullptr, &hid_guid, index,
                                     &interface_data);
         ++index) {
        DWORD required_size = 0;
        SetupDiGetDeviceInterfaceDetailW(
            devices, &interface_data, nullptr, 0, &required_size, nullptr);
        if (required_size < sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W)) continue;

        std::vector<std::uint8_t> detail_buffer(required_size);
        auto* detail = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(
            detail_buffer.data());
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
        if (!SetupDiGetDeviceInterfaceDetailW(
                devices, &interface_data, detail, required_size, nullptr, nullptr)) {
            continue;
        }

        HANDLE raw = CreateFileW(detail->DevicePath, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED, nullptr);
        if (raw == INVALID_HANDLE_VALUE) continue;

        UniqueHandle handle(raw);
        USHORT input_report_length = 0;
        if (!matches_cardio(raw, input_report_length)) continue;

        result.handle = std::move(handle);
        result.input_report_length = input_report_length;
        break;
    }

    SetupDiDestroyDeviceInfoList(devices);
    return result;
}

enum class ReadResult { Report, Disconnected, Stop };

ReadResult read_report(HANDLE device, HANDLE stop_event, USHORT report_length,
                       std::vector<std::uint8_t>& report) {
    report.assign(report_length, 0);
    OVERLAPPED overlapped{};
    UniqueHandle completed(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    if (!completed) return ReadResult::Disconnected;
    overlapped.hEvent = static_cast<HANDLE>(completed.get());

    DWORD bytes_read = 0;
    if (!ReadFile(device, report.data(), report_length, &bytes_read, &overlapped)) {
        const DWORD error = GetLastError();
        if (error != ERROR_IO_PENDING) return ReadResult::Disconnected;

        const std::array<HANDLE, 2> waits{
            stop_event, static_cast<HANDLE>(completed.get())
        };
        const DWORD wait = WaitForMultipleObjects(
            static_cast<DWORD>(waits.size()), waits.data(), FALSE, INFINITE);
        if (wait == WAIT_OBJECT_0) {
            CancelIoEx(device, &overlapped);
            GetOverlappedResult(device, &overlapped, &bytes_read, TRUE);
            return ReadResult::Stop;
        }
        if (wait != WAIT_OBJECT_0 + 1 ||
            !GetOverlappedResult(device, &overlapped, &bytes_read, FALSE)) {
            return ReadResult::Disconnected;
        }
    }

    report.resize(bytes_read);
    return ReadResult::Report;
}

void emit(const IEventSource::Sink& sink,
          const std::vector<NativeEvent>& events) {
    for (const auto& event : events) sink(event);
}

}

void WindowsCardIoReader::run(std::stop_token stop, const Sink& sink) {
    UniqueHandle stop_event(CreateEventW(nullptr, TRUE, FALSE, nullptr));
    if (!stop_event) {
        sink({.type = TM_NFC_EVENT_ERROR,
              .error_code = static_cast<std::int32_t>(GetLastError())});
        return;
    }

    const HANDLE stop_handle = static_cast<HANDLE>(stop_event.get());
    std::stop_callback stop_callback(stop, [stop_handle] {
        SetEvent(stop_handle);
    });
    CardStateMachine state;

    while (!stop.stop_requested()) {
        DeviceConnection connection = open_cardio();
        if (!connection.handle) {
            if (WaitForSingleObject(stop_handle, kRetryDelayMs) == WAIT_OBJECT_0) {
                break;
            }
            continue;
        }

        emit(sink, state.reader_connected());
        std::vector<std::uint8_t> report;
        while (!stop.stop_requested()) {
            const ReadResult result = read_report(
                static_cast<HANDLE>(connection.handle.get()), stop_handle,
                connection.input_report_length, report);
            if (result == ReadResult::Stop) break;
            if (result == ReadResult::Disconnected) {
                emit(sink, state.reader_disconnected());
                break;
            }
            emit(sink, state.observe(decode_cardio_report(report)));
        }
    }

    emit(sink, state.reader_disconnected());
}

}
