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
    // Texture URIs (project-relative; empty = none) and whether the material
    // silhouettes through an alpha mask (card foliage). ADR-0009.
    std::string base_color_texture;
    std::string normal_texture;
    bool alpha_masked = false;
    std::vector<float> positions; // xyz
    std::vector<float> normals;   // xyz, unit
    std::vector<float> uvs;       // uv
    // MikkTSpace-style UV-gradient tangents, xyzw (w = handedness).
    std::vector<float> tangents;
    // Wind vertex channels (13 game-wind semantics, ADR-0009): per-vertex
    // sway pivot and oscillator parameters (amplitude_rad, phase, depth,
    // kind) matching the authoring wind exactly.
    std::vector<float> wind_anchor; // xyz
    std::vector<float> wind_params; // amplitude, phase, depth, kind
    std::vector<std::uint32_t> indices;
    std::array<double, 3> min_position{1e30, 1e30, 1e30};
    std::array<double, 3> max_position{-1e30, -1e30, -1e30};

    std::size_t vertex_count() const { return positions.size() / 3; }
};

// Deterministic; empty model produces an empty vector.
std::vector<MergedPrimitive> merge_by_material(const doc::Document& document,
                                               const eval::EvaluatedModel& model);

} // namespace canopy::exp
