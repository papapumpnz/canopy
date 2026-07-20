#include "canopy/foundation/hash.hpp"
#include "canopy_test.hpp"

using namespace canopy;

// FIPS 180-4 published test vectors.
CANOPY_TEST(sha256_empty) {
    CHECK_EQ(sha256("").hex(),
             std::string("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
}

CANOPY_TEST(sha256_abc) {
    CHECK_EQ(sha256("abc").hex(),
             std::string("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
}

CANOPY_TEST(sha256_two_blocks) {
    CHECK_EQ(sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq").hex(),
             std::string("248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"));
}

CANOPY_TEST(sha256_incremental_matches_oneshot) {
    Sha256 hasher;
    hasher.update("abc");
    hasher.update("dbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
    CHECK_EQ(hasher.finish().hex(),
             sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq").hex());
}

CANOPY_TEST(sha256_exactly_one_block_padding_boundary) {
    // 55, 56 and 64 byte messages cross the padding boundaries.
    const std::string bytes55(55, 'a');
    const std::string bytes56(56, 'a');
    const std::string bytes64(64, 'a');
    CHECK_EQ(sha256(bytes55).hex().size(), std::size_t{64});
    CHECK(sha256(bytes55).hex() != sha256(bytes56).hex());
    CHECK(sha256(bytes56).hex() != sha256(bytes64).hex());
    // Incremental one-byte updates equal one-shot for each boundary size.
    for (const auto& bytes : {bytes55, bytes56, bytes64}) {
        Sha256 hasher;
        for (const char c : bytes) {
            hasher.update(&c, 1);
        }
        CHECK_EQ(hasher.finish().hex(), sha256(bytes).hex());
    }
}

CANOPY_TEST(bulk_and_chunked_updates_agree) {
    // Exercises the bulk multi-block path (hardware SHA when available)
    // against buffered small-chunk updates across awkward boundaries.
    std::string payload(1 << 20, '\0');
    for (std::size_t i = 0; i < payload.size(); ++i) {
        payload[i] = char((i * 131 + 7) & 0xff);
    }
    const ContentHash oneshot = sha256(payload);
    for (const std::size_t chunk : {std::size_t{1}, std::size_t{63}, std::size_t{64},
                                    std::size_t{100}, std::size_t{65536}}) {
        Sha256 hasher;
        for (std::size_t offset = 0; offset < payload.size(); offset += chunk) {
            hasher.update(payload.data() + offset,
                          std::min(chunk, payload.size() - offset));
        }
        CHECK(hasher.finish() == oneshot);
    }
}

CANOPY_TEST(content_hash_hex_roundtrip) {
    const ContentHash hash = sha256("roundtrip");
    CHECK(ContentHash::from_hex(hash.hex()) == hash);
    CHECK(ContentHash::from_hex("xyz") == ContentHash{});
    CHECK(ContentHash::from_hex(std::string(63, 'a')) == ContentHash{});
}

CANOPY_TEST_MAIN()
