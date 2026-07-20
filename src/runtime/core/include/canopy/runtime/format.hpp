// `.canopyrt` v1 container constants (ADR-0008). Shared by the authoring-side
// compiler and the runtime loader; the runtime never includes authoring code.
#pragma once

#include <cstdint>

namespace canopy::rt {

inline constexpr char kMagic[8] = {'C', 'N', 'P', 'Y', 'R', 'T', '\0', '\0'};
inline constexpr std::uint32_t kVersion = 1;
inline constexpr std::uint32_t kEndianProbe = 0x01020304;

enum class SectionType : std::uint32_t {
    metadata = 1,  // canonical JSON
    materials = 2, // canonical JSON
    lods = 3,      // canonical JSON descriptor
    vertices = 4,  // interleaved float32 pos3/normal3/uv2, subtype = LOD index
    indices = 5,   // uint32, subtype = LOD index
};

// Header: magic[8], u32 version, u32 endian probe, u64 feature flags,
// u32 section count, u32 reserved. Then section_count table entries.
inline constexpr std::size_t kHeaderSize = 8 + 4 + 4 + 8 + 4 + 4;
// Table entry: u32 type, u32 subtype, u64 offset, u64 length, u64 sha256_low64.
inline constexpr std::size_t kTableEntrySize = 4 + 4 + 8 + 8 + 8;
inline constexpr std::size_t kVertexStrideBytes = 8 * 4; // pos3 + normal3 + uv2

} // namespace canopy::rt
