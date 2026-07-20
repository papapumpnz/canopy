// Named deterministic random streams (ADR-0003, backlog B-018).
//
// Every observable random decision draws from a stream identified by a
// StreamKey. Keys are derived by SHA-256 over a domain-separated tuple so that
// adding streams, reordering evaluation, or changing worker counts can never
// shift existing values. There is no global generator.
//
// Thread safety: RandomStream is a value type; concurrent use of one instance
// is a bug (each work item derives its own stream).
#pragma once

#include "canopy/foundation/hash.hpp"
#include "canopy/foundation/ids.hpp"

#include <cstdint>
#include <string_view>

namespace canopy {

struct StreamKey {
    std::uint64_t state = 0;
};

// algorithm_version increments only via ADR — it shifts every downstream value.
inline constexpr std::uint32_t kRandomAlgorithmVersion = 1;

StreamKey derive_stream_key(std::uint64_t document_seed, const Uuid& generator_id,
                            SemanticId semantic_parent_id, std::string_view property_key,
                            std::string_view purpose_tag,
                            std::uint32_t algorithm_version = kRandomAlgorithmVersion);

// SplitMix64 (Steele, Lea, Flood 2014; public-domain constants). Chosen for
// exact cross-platform behavior and O(1) keyed construction.
class RandomStream {
public:
    explicit RandomStream(StreamKey key) : state_(key.state) {}

    std::uint64_t next_u64();
    // 53-bit mantissa / 2^53 → uniform in [0, 1).
    double next_double();
    // Uniform in [minimum, maximum]; degenerate range returns minimum.
    double uniform(double minimum, double maximum);
    // Uniform integer in [0, count); count == 0 returns 0.
    std::uint64_t uniform_index(std::uint64_t count);

private:
    std::uint64_t state_ = 0;
};

} // namespace canopy
