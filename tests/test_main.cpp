#include "test_support.h"

#include <iostream>

int main() {
    int failures = 0;
    for (const auto& test : test_support::registry()) {
        try {
            test.run();
            std::cout << "PASS " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "FAIL " << test.name << ": " << error.what() << '\n';
        }
    }
    std::cout << test_support::registry().size() << " tests, "
              << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
