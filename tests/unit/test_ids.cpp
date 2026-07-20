#include "canopy/foundation/ids.hpp"
#include "canopy_test.hpp"

using namespace canopy;

CANOPY_TEST(uuid_parse_format_roundtrip) {
    const std::string text = "37f7d1ee-c8a8-4d3e-8ba9-3c3a77e40533";
    auto parsed = Uuid::parse(text);
    CHECK(parsed.has_value());
    if (parsed) {
        CHECK_EQ(parsed->str(), text);
        CHECK(!parsed->is_nil());
    }
    // Uppercase input parses; output is always lowercase.
    auto upper = Uuid::parse("37F7D1EE-C8A8-4D3E-8BA9-3C3A77E40533");
    CHECK(upper.has_value() && upper->str() == text);
}

CANOPY_TEST(uuid_rejects_malformed) {
    CHECK(!Uuid::parse("").has_value());
    CHECK(!Uuid::parse("37f7d1ee-c8a8-4d3e-8ba9-3c3a77e4053").has_value());   // short
    CHECK(!Uuid::parse("37f7d1ee-c8a8-4d3e-8ba9-3c3a77e405331").has_value()); // long
    CHECK(!Uuid::parse("37f7d1eexc8a8-4d3e-8ba9-3c3a77e40533").has_value());  // bad dash
    CHECK(!Uuid::parse("37f7d1ee-c8a8-4d3e-8ba9-3c3a77e4053g").has_value());  // bad digit
}

CANOPY_TEST(uuid_generate_is_v4_and_unique) {
    const Uuid a = Uuid::generate();
    const Uuid b = Uuid::generate();
    CHECK(!a.is_nil());
    CHECK(!(a == b));
    CHECK_EQ(int(a.bytes[6] >> 4), 4);       // version
    CHECK_EQ(int(a.bytes[8] >> 6), 2);       // variant
}

CANOPY_TEST(semantic_id_formats_as_16_hex) {
    CHECK_EQ(SemanticId{0}.str(), std::string("0000000000000000"));
    CHECK_EQ(SemanticId{0xdeadbeef01020304ull}.str(), std::string("deadbeef01020304"));
}

CANOPY_TEST_MAIN()
