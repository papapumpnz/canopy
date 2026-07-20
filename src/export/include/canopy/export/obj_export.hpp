// OBJ diagnostic export (backlog B-029/B-030, 16_EXPORT_PIPELINE groundwork).
//
// Writes `<base>.obj`, `<base>.mtl` and `<base>.manifest.json`. The OBJ is an
// open industry format written deterministically: nodes in semantic-ID order,
// canonical numeric formatting, one `g`/`usemtl` group per branch node keyed
// by semantic ID. The manifest records counts and content hashes so an
// independent parser can verify the artifact.
#pragma once

#include "canopy/evaluation/evaluate.hpp"
#include "canopy/foundation/hash.hpp"

#include <filesystem>

namespace canopy::exp {

struct ExportPreset {
    std::string format = "obj";       // "obj" | "gltf"
    std::string profile = "production";
    bool write_normals = true;
    bool write_uvs = true;
    // gltf only (ADR-0009): bake production/preview/draft as
    // <base>.lod0/1/2.glb plus a combined manifest with switch distances.
    bool bake_lods = false;

    static Result<ExportPreset> load(const std::filesystem::path& preset_path);
};

struct ExportManifest {
    std::string obj_file;
    std::string mtl_file;
    ContentHash obj_sha256;
    std::uint64_t model_hash = 0;
    std::uint64_t model_topology_hash = 0;
    std::size_t node_count = 0;
    std::size_t vertex_count = 0;
    std::size_t triangle_count = 0;
};

// Writes the OBJ/MTL/manifest set for `model`; `out_base` is the path without
// extension. Returns the manifest that was written.
Result<ExportManifest> write_obj(const doc::Document& document, const eval::EvaluatedModel& model,
                                 const ExportPreset& preset,
                                 const std::filesystem::path& out_base);

} // namespace canopy::exp
