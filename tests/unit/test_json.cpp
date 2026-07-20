#include "canopy/foundation/json.hpp"
#include "canopy_test.hpp"

using namespace canopy;
using canopy::json::Value;

static std::string canonical(std::string_view text) {
    auto parsed = json::parse(text);
    CHECK(parsed.ok());
    if (!parsed.ok()) {
        return {};
    }
    auto written = json::write_canonical(parsed.value());
    CHECK(written.ok());
    return written.ok() ? written.value() : std::string{};
}

CANOPY_TEST(canonical_sorts_keys_and_is_stable) {
    const std::string a = canonical(R"({"b": 1, "a": {"z": true, "y": null}})");
    const std::string b = canonical("{\n  \"a\": {\"y\": null, \"z\": true},\"b\":1}");
    CHECK_EQ(a, b);
    CHECK_EQ(a, std::string("{\n  \"a\": {\n    \"y\": null,\n    \"z\": true\n  },\n  \"b\": 1\n}\n"));
    // Repeated canonicalization is byte-identical (B-013).
    CHECK_EQ(canonical(a), a);
}

CANOPY_TEST(number_formatting) {
    CHECK_EQ(json::format_double(0.5), std::string("0.5"));
    CHECK_EQ(json::format_double(1.0), std::string("1"));
    CHECK_EQ(json::format_double(-3.0), std::string("-3"));
    CHECK_EQ(json::format_int(42), std::string("42"));
    // Shortest round-trip: value survives parse → format.
    const std::string text = json::format_double(0.1);
    auto parsed = json::parse(text);
    CHECK(parsed.ok() && parsed.value().as_number() == 0.1);
}

CANOPY_TEST(parse_rejects_malformed_input) {
    CHECK(!json::parse("{").ok());
    CHECK(!json::parse("[1,]").ok());
    CHECK(!json::parse("{\"a\": 1, \"a\": 2}").ok()); // duplicate key
    CHECK(!json::parse("01").ok());                     // leading zero
    CHECK(!json::parse("\"\\x\"").ok());               // bad escape
    CHECK(!json::parse("\"unterminated").ok());
    CHECK(!json::parse("[1] trailing").ok());
    CHECK(!json::parse("\"\\ud800\"").ok()); // unpaired surrogate
    CHECK(!json::parse("nul").ok());
}

CANOPY_TEST(parse_error_reports_stable_code_and_location) {
    auto result = json::parse("{\n  \"a\": zz\n}", "test.json");
    CHECK(!result.ok());
    if (!result.ok()) {
        CHECK_EQ(std::string(error_code_id(result.error().code)), std::string("CANOPY-E2002"));
        CHECK_EQ(result.error().location.file, std::string("test.json"));
        CHECK_EQ(result.error().location.line, 2u);
    }
}

CANOPY_TEST(string_escapes_roundtrip) {
    const std::string text = R"({"s": "line\n\ttab \"quote\" é 🌳"})";
    auto parsed = json::parse(text);
    CHECK(parsed.ok());
    if (parsed.ok()) {
        const auto* s = parsed.value().find("s");
        CHECK(s != nullptr && s->is_string());
        // 🌳 is one supplementary code point (4-byte UTF-8).
        CHECK(s->as_string().find("\xF0\x9F\x8C\xB3") != std::string::npos);
        auto rewritten = json::write_canonical(parsed.value());
        CHECK(rewritten.ok());
        auto reparsed = json::parse(rewritten.value());
        CHECK(reparsed.ok() && reparsed.value() == parsed.value());
    }
}

CANOPY_TEST(non_finite_numbers_never_serialize) {
    json::Object object;
    object.emplace("bad", 0.0 / 0.0);
    CHECK(!json::write_canonical(Value(std::move(object))).ok());
}

CANOPY_TEST(int64_boundaries) {
    auto parsed = json::parse("[9223372036854775807, -9223372036854775808]");
    CHECK(parsed.ok());
    if (parsed.ok()) {
        CHECK(parsed.value().as_array()[0].is_int());
        CHECK_EQ(parsed.value().as_array()[0].as_int(), INT64_MAX);
    }
    // Integer exceeding int64 falls back to double, not an error.
    auto huge = json::parse("18446744073709551616");
    CHECK(huge.ok() && huge.value().is_double());
}

CANOPY_TEST(depth_limit_enforced) {
    std::string deep;
    for (int i = 0; i < 200; ++i) {
        deep += "[";
    }
    deep += "1";
    for (int i = 0; i < 200; ++i) {
        deep += "]";
    }
    CHECK(!json::parse(deep).ok());
}

CANOPY_TEST_MAIN()
