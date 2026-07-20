// Merge-by-material geometry preparation shared by runtime-facing exporters
// (glTF, .canopyrt — ADR-0007/0008): one primitive per referenced material in
// UUID order, vertices appended in semantic-node order, float32 attributes,
// season-blended colors.
#pragma once

#include "canopy/evaluation/evaluate.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace canopy::exp {

struct MergedPrimitive {
    Uuid material_id; // nil for the default material
    std::string material_name;
    std::array<double, 4> color{0.6, 0.5, 0.4, 1.0};
    bool double_sided = false;
    std::vector<float> positions; // xyz
    std::vector<float> normals;   // xyz, unit
    std::vector<float> uvs;       // uv
    std::vector<std::uint32_t> indices;
    std::array<double, 3> min_position{1e30, 1e30, 1e30};
    std::array<double, 3> max_position{-1e30, -1e30, -1e30};

    std::size_t vertex_count() const { return positions.size() / 3; }
};

// Deterministic; empty model produces an empty vector.
std::vector<MergedPrimitive> merge_by_material(const doc::Document& document,
                                               const eval::EvaluatedModel& model);

} // namespace canopy::exp
