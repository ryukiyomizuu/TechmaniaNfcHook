#pragma once

#include <exception>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace test_support {

struct TestCase {
    const char* name;
    void (*run)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const char* name, void (*run)()) {
        registry().push_back({name, run});
    }
};

inline void require(bool condition, const char* expression, const char* file, int line) {
    if (condition) return;
    std::ostringstream message;
    message << file << ':' << line << ": requirement failed: " << expression;
    throw std::runtime_error(message.str());
}

}

#define TM_TEST_JOIN_INNER(left, right) left##right
#define TM_TEST_JOIN(left, right) TM_TEST_JOIN_INNER(left, right)
#define TEST_CASE(name) \
    static void TM_TEST_JOIN(test_function_, __LINE__)(); \
    static ::test_support::Registrar TM_TEST_JOIN(test_registrar_, __LINE__)( \
        name, &TM_TEST_JOIN(test_function_, __LINE__)); \
    static void TM_TEST_JOIN(test_function_, __LINE__)()
#define REQUIRE(expression) \
    ::test_support::require(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
