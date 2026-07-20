#include "canopy/foundation/ids.hpp"

#include <algorithm>
#include <random>

namespace canopy {

bool Uuid::is_nil() const {
    return std::all_of(bytes.begin(), bytes.end(), [](std::uint8_t b) { return b == 0; });
}

std::string Uuid::str() const {
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string out;
    out.reserve(36);
    for (std::size_t i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            out.push_back('-');
        }
        out.push_back(kDigits[bytes[i] >> 4]);
        out.push_back(kDigits[bytes[i] & 0x0fu]);
    }
    return out;
}

std::optional<Uuid> Uuid::parse(std::string_view text) {
    if (text.size() != 36 || text[8] != '-' || text[13] != '-' || text[18] != '-' ||
        text[23] != '-') {
        return std::nullopt;
    }
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    Uuid out{};
    std::size_t byte_index = 0;
    for (std::size_t i = 0; i < 36;) {
        if (text[i] == '-') {
            ++i;
            continue;
        }
        const int hi = nibble(text[i]);
        const int lo = nibble(text[i + 1]);
        if (hi < 0 || lo < 0) {
            return std::nullopt;
        }
        out.bytes[byte_index++] = std::uint8_t((hi << 4) | lo);
        i += 2;
    }
    return out;
}

Uuid Uuid::generate() {
    // std::random_device is the OS entropy source on all supported platforms.
    // Authoring-time only; evaluation code must use named streams (ADR-0003).
    std::random_device device;
    Uuid out{};
    for (std::size_t i = 0; i < 16; i += 4) {
        const std::uint32_t word = device();
        out.bytes[i] = std::uint8_t(word & 0xffu);
        out.bytes[i + 1] = std::uint8_t((word >> 8) & 0xffu);
        out.bytes[i + 2] = std::uint8_t((word >> 16) & 0xffu);
        out.bytes[i + 3] = std::uint8_t((word >> 24) & 0xffu);
    }
    out.bytes[6] = std::uint8_t((out.bytes[6] & 0x0fu) | 0x40u); // version 4
    out.bytes[8] = std::uint8_t((out.bytes[8] & 0x3fu) | 0x80u); // variant 1
    return out;
}

std::string SemanticId::str() const {
    static constexpr char kDigits[] = "0123456789abcdef";
    std::string out;
    out.reserve(16);
    for (int shift = 60; shift >= 0; shift -= 4) {
        out.push_back(kDigits[(value >> shift) & 0x0fu]);
    }
    return out;
}

} // namespace canopy
