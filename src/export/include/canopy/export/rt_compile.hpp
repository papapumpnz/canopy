// `.canopyrt` compiler (ADR-0008): bakes evaluated LOD models into the
// runtime container. Authoring-side; the runtime loader lives in
// canopy_runtime_core and shares only the format constants.
#pragma once

#include "canopy/evaluation/evaluate.hpp"
#include "canopy/foundation/hash.hpp"

#include <filesystem>
#include <vector>

namespace canopy::exp {

struct RtCompileManifest {
    std::string rt_file;
    ContentHash rt_sha256;
    std::size_t lod_count = 0;
    std::vector<std::size_t> triangles_per_lod;
};

// `lods` are evaluated models of the same document, LOD 0 (highest detail)
// first; all must share the document. Writes `<base>.canopyrt` and
// `<base>.manifest.json`. Byte-deterministic.
Result<RtCompileManifest> write_canopyrt(const doc::Document& document,
                                         const std::vector<eval::EvaluatedModel>& lods,
                                         const std::filesystem::path& out_base);

} // namespace canopy::exp
