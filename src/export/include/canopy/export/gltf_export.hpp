// glTF 2.0 binary (GLB) export (ADR-0007).
//
// Writes `<base>.glb` and `<base>.manifest.json`. One mesh, one primitive per
// referenced material (UUID order); float32 attributes, uint32 indices; Y-up
// right-handed meters (glTF's native convention). Deterministic: identical
// inputs produce byte-identical GLB files.
#pragma once

#include "canopy/evaluation/evaluate.hpp"
#include "canopy/export/obj_export.hpp"
#include "canopy/foundation/hash.hpp"

#include <filesystem>

namespace canopy::exp {

struct GltfManifest {
    std::string glb_file;
    ContentHash glb_sha256;
    std::uint64_t model_hash = 0;
    std::size_t primitive_count = 0;
    std::size_t vertex_count = 0;
    std::size_t triangle_count = 0;
};

Result<GltfManifest> write_glb(const doc::Document& document, const eval::EvaluatedModel& model,
                               const ExportPreset& preset,
                               const std::filesystem::path& out_base);

} // namespace canopy::exp
