#include "canopy/foundation/random.hpp"

namespace canopy {

StreamKey derive_stream_key(std::uint64_t document_seed, const Uuid& generator_id,
                            SemanticId semantic_parent_id, std::string_view property_key,
                            std::string_view purpose_tag, std::uint32_t algorithm_version) {
    Sha256 hasher;
    // Fixed-width little-endian fields with a leading domain string; text
    // fields are length-prefixed so tuples can never collide by concatenation.
    hasher.update(std::string_view("canopy-rng-v1"));
    auto put_u64 = [&hasher](std::uint64_t value) {
        std::uint8_t bytes[8];
        for (int i = 0; i < 8; ++i) {
            bytes[i] = std::uint8_t((value >> (8 * i)) & 0xffu);
        }
        hasher.update(bytes, sizeof bytes);
    };
    auto put_text = [&hasher, &put_u64](std::string_view text) {
        put_u64(text.size());
        hasher.update(text);
    };
    put_u64(document_seed);
    hasher.update(generator_id.bytes.data(), generator_id.bytes.size());
    put_u64(semantic_parent_id.value);
    put_text(property_key);
    put_text(purpose_tag);
    put_u64(algorithm_version);
    return StreamKey{hasher.finish().low64()};
}

std::uint64_t RandomStream::next_u64() {
    state_ += 0x9e3779b97f4a7c15ull;
    std::uint64_t z = state_;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
    return z ^ (z >> 31);
}

double RandomStream::next_double() {
    return double(next_u64() >> 11) * 0x1.0p-53;
}

double RandomStream::uniform(double minimum, double maximum) {
    if (!(maximum > minimum)) {
        return minimum;
    }
    return minimum + (maximum - minimum) * next_double();
}

std::uint64_t RandomStream::uniform_index(std::uint64_t count) {
    if (count == 0) {
        return 0;
    }
    // Debiased modulo via rejection sampling; deterministic given the stream.
    const std::uint64_t threshold = (0ull - count) % count;
    while (true) {
        const std::uint64_t value = next_u64();
        if (value >= threshold) {
            return value % count;
        }
    }
}

} // namespace canopy
