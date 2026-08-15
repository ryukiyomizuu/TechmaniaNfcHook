cmake_minimum_required(VERSION 3.25)
project(TechmaniaNfcHook VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_executable(techmania_nfc_tests
    src/card_report_decoder.cpp
    tests/test_main.cpp
    tests/test_card_report_decoder.cpp
)

target_include_directories(techmania_nfc_tests PRIVATE
    include
    src
    tests
)

if(MSVC)
    target_compile_options(techmania_nfc_tests PRIVATE /W4 /WX /permissive-)
endif()

add_test(NAME techmania_nfc_tests COMMAND techmania_nfc_tests)
