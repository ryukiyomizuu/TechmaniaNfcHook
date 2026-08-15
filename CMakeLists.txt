cmake_minimum_required(VERSION 3.25)
project(TechmaniaNfcHook VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(techmania_nfc_core STATIC
    src/card_report_decoder.cpp
    src/card_state_machine.cpp
    src/event_queue.cpp
    src/nfc_hook_runtime.cpp
)

target_include_directories(techmania_nfc_core PUBLIC
    include
    src
)

add_library(TechmaniaNfcHook SHARED
    src/nfc_hook_exports.cpp
    src/windows_cardio_reader.cpp
)

target_include_directories(TechmaniaNfcHook PRIVATE
    include
    src
)

target_compile_definitions(TechmaniaNfcHook PRIVATE
    TM_NFC_BUILD_DLL
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(TechmaniaNfcHook PRIVATE
    techmania_nfc_core
    hid
    setupapi
)

add_library(techmania_nfc_probe_support STATIC
    tools/probe_formatter.cpp
)

target_include_directories(techmania_nfc_probe_support PUBLIC
    include
    tools
)

target_link_libraries(techmania_nfc_probe_support PUBLIC bcrypt)

add_executable(TechmaniaNfcProbe
    tools/nfc_hook_probe.cpp
)

target_link_libraries(TechmaniaNfcProbe PRIVATE
    TechmaniaNfcHook
    techmania_nfc_probe_support
)

add_executable(techmania_nfc_tests
    tests/test_main.cpp
    tests/test_card_report_decoder.cpp
    tests/test_card_state_machine.cpp
    tests/test_event_queue.cpp
    tests/test_hook_runtime.cpp
    tests/test_probe_format.cpp
)

target_include_directories(techmania_nfc_tests PRIVATE
    tests
    tools
)

target_link_libraries(techmania_nfc_tests PRIVATE
    techmania_nfc_core
    techmania_nfc_probe_support
)

if(MSVC)
    foreach(target techmania_nfc_core TechmaniaNfcHook
                   techmania_nfc_probe_support TechmaniaNfcProbe
                   techmania_nfc_tests)
        target_compile_options(${target} PRIVATE /W4 /WX /permissive-)
    endforeach()
endif()

add_test(NAME techmania_nfc_tests COMMAND techmania_nfc_tests)
