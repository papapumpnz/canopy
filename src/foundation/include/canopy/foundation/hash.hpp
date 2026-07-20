// SHA-256 content hashing (ADR-0002). Thread safety: Sha256 instances are
// independent; free functions are reentrant.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace canopy {

struct ContentHash {
    std::array<std::uint8_t, 32> bytes{};

    // Lowercase hex, 64 chars.
    std::string hex() const;
    static ContentHash from_hex(std::string_view hex_digits); // all-zero on malformed input
    auto operator<=>(const ContentHash&) const = default;

    // First 8 bytes interpreted little-endian; used for PRNG seeding (ADR-0003).
    std::uint64_t low64() const;
};

class Sha256 {
public:
    Sha256();
    void update(const void* data, std::size_t size);
    void update(std::string_view text) { update(text.data(), text.size()); }
    ContentHash finish();

private:
    void process_block(const std::uint8_t* block);

    std::array<std::uint32_t, 8> state_{};
    std::array<std::uint8_t, 64> buffer_{};
    std::uint64_t total_bytes_ = 0;
    std::size_t buffered_ = 0;
};

ContentHash sha256(std::string_view bytes);

} // namespace canopy
