// Minimal in-house test harness (ADR-0001: no third-party deps in bootstrap).
// Usage: CANOPY_TEST(name) { CHECK(cond); CHECK_EQ(a, b); } — one executable
// per suite, registered with CTest; a nonzero exit lists the failures.
#pragma once

#include <cmath>
#include <cstdio>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace canopy::test {

struct TestCase {
    const char* name;
    std::function<void()> body;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int failure_count = 0;
inline const char* current_test = "";

struct Registrar {
    Registrar(const char* name, std::function<void()> body) {
        registry().push_back(TestCase{name, std::move(body)});
    }
};

template <typename A, typename B>
void check_eq(const A& a, const B& b, const char* expr_a, const char* expr_b, const char* file,
              int line) {
    if (!(a == b)) {
        std::ostringstream message;
        message << a;
        std::ostringstream expected;
        expected << b;
        std::fprintf(stderr, "FAIL %s: %s:%d: %s == %s (got '%s', expected '%s')\n", current_test,
                     file, line, expr_a, expr_b, message.str().c_str(), expected.str().c_str());
        ++failure_count;
    }
}

inline void check_near(double a, double b, double tolerance, const char* expr_a,
                       const char* expr_b, const char* file, int line) {
    if (!(std::fabs(a - b) <= tolerance)) {
        std::fprintf(stderr, "FAIL %s: %s:%d: %s ~= %s (got %.17g, expected %.17g, tol %.3g)\n",
                     current_test, file, line, expr_a, expr_b, a, b, tolerance);
        ++failure_count;
    }
}

inline void check(bool condition, const char* expression, const char* file, int line) {
    if (!condition) {
        std::fprintf(stderr, "FAIL %s: %s:%d: %s\n", current_test, file, line, expression);
        ++failure_count;
    }
}

inline int run_all() {
    for (const auto& test : registry()) {
        current_test = test.name;
        test.body();
    }
    if (failure_count == 0) {
        std::printf("OK (%zu tests)\n", registry().size());
        return 0;
    }
    std::fprintf(stderr, "%d failure(s) across %zu tests\n", failure_count, registry().size());
    return 1;
}

} // namespace canopy::test

#define CANOPY_TEST(name)                                                                          \
    static void canopy_test_##name();                                                              \
    static const ::canopy::test::Registrar canopy_registrar_##name{#name, canopy_test_##name};     \
    static void canopy_test_##name()

#define CHECK(condition) ::canopy::test::check((condition), #condition, __FILE__, __LINE__)
#define CHECK_EQ(a, b) ::canopy::test::check_eq((a), (b), #a, #b, __FILE__, __LINE__)
#define CHECK_NEAR(a, b, tolerance)                                                                \
    ::canopy::test::check_near((a), (b), (tolerance), #a, #b, __FILE__, __LINE__)

#define CANOPY_TEST_MAIN()                                                                         \
    int main() { return ::canopy::test::run_all(); }
