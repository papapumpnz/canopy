// Deterministic evaluation, bootstrap vertical slice (backlog B-030,
// 07_EVALUATION_ENGINE.md).
//
// Scope: Tree root + Branch generators, interval/absolute placement, straight
// spines with radius profiles, single-threaded deterministic scheduling.
// Output order is sorted by semantic ID (07: "gathered and sorted by semantic
// ID before final assembly"), so a parallel scheduler can slot in later
// without changing observable output.
#pragma once

#include "canopy/document/document.hpp"
#include "canopy/geometry/sweep.hpp"

#include <string>
#include <vector>

namespace canopy::eval {

struct EvaluationProfile {
    std::string name;
    double length_samples_per_meter = 12.0;
    std::uint32_t min_length_samples = 8;
    std::uint32_t max_length_samples = 96;
    std::uint32_t radial_segments_cap = 32;

    static EvaluationProfile draft();
    static EvaluationProfile preview();
    static EvaluationProfile production();
    // Known profile by name; error on unknown names.
    static Result<EvaluationProfile> by_name(std::string_view name);
};

// Timeline sample (13_WIND_GROWTH_AND_SEASONS.md): one deterministic frame.
// Same document + profile + sample → identical output on every platform.
struct TimelineSample {
    double time_s = 0.0;
    double growth = 1.0;         // normalized lifecycle, 0..1
    double season = 0.45;        // 0 spring → 1 winter
    double wind_strength = 0.0;  // 0..1
    double wind_direction_deg = 0.0;
    double gust = 0.5;           // gust depth, 0..1
};

enum class NodeKind : std::uint8_t { branch = 0, frond = 1, foliage = 2 };

struct BranchNodeGeometry {
    SemanticId semantic_id;
    SemanticId parent_semantic_id;
    Uuid generator_id;
    Uuid material_id; // nil when unassigned
    double length_m = 0.0;
    double base_radius_m = 0.0;
    geo::TriangleMesh mesh;
    // Wind/animation metadata: sway bends about the node base with a
    // quadratic stiffness falloff (zero at the base); kind and depth select
    // response amplitude (13: hierarchy level, geometry type). Anchored
    // nodes (ground-clamped roots) do not sway at all.
    Vec3 base_position{};
    NodeKind kind = NodeKind::branch;
    std::uint32_t depth = 0; // generator-graph distance from the root
    bool anchored = false;
};

struct EvaluatedModel {
    // Sorted by semantic_id: the observable, deterministic assembly order.
    std::vector<BranchNodeGeometry> nodes;
    std::vector<Diagnostic> warnings;
    // The sample this model was evaluated at (exporters use season for
    // material blending).
    TimelineSample sample;

    std::size_t total_vertices() const;
    std::size_t total_triangles() const;
    // Combined hash over (semantic_id, geometry_hash) pairs in order; the
    // cross-worker/cross-platform determinism witness.
    std::uint64_t model_hash() const;
    std::uint64_t model_topology_hash() const;
};

Result<EvaluatedModel> evaluate(const doc::Document& document, const EvaluationProfile& profile,
                                const TimelineSample& sample = {});

} // namespace canopy::eval
