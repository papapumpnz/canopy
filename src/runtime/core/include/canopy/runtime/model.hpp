// Runtime model: the loaded, validated form of a `.canopyrt` file
// (17_RUNTIME_SDK_AND_FOREST.md bootstrap slice). Depends on canopy_foundation
// only — no authoring code. Loading validates every section (bounds,
// checksums, cross-references) before use; a loaded RtModel is immutable and
// safe to share across threads.
#pragma once

#include "canopy/foundation/diagnostics.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace canopy::rt {

struct RtMaterial {
    std::string name;
    std::array<double, 4> color{0.6, 0.5, 0.4, 1.0};
    bool two_sided = false;
};

struct RtPrimitive {
    std::uint32_t material = 0;     // index into RtModel::materials
    std::uint32_t index_offset = 0; // element offset into the LOD index buffer
    std::uint32_t index_count = 0;
};

struct RtLod {
    // Interleaved float32: position xyz, normal xyz, uv — 8 floats per vertex.
    std::vector<float> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<RtPrimitive> primitives;

    std::size_t vertex_count() const { return vertices.size() / 8; }
    std::size_t triangle_count() const { return indices.size() / 3; }
};

struct RtBounds {
    std::array<double, 3> minimum{0, 0, 0};
    std::array<double, 3> maximum{0, 0, 0};

    double radius() const;
};

struct RtModel {
    std::string name;
    std::string document_id;
    std::string source_model_hash; // authoring model hash at compile time
    RtBounds bounds;
    std::vector<RtMaterial> materials;
    std::vector<RtLod> lods; // LOD 0 = highest detail
};

Result<RtModel> load_model_bytes(std::string_view bytes, std::string source_name = {});
Result<RtModel> load_model(const std::filesystem::path& path);

} // namespace canopy::rt
