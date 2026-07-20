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

struct BranchNodeGeometry {
    SemanticId semantic_id;
    SemanticId parent_semantic_id;
    Uuid generator_id;
    Uuid material_id; // nil when unassigned
    double length_m = 0.0;
    double base_radius_m = 0.0;
    geo::TriangleMesh mesh;
};

struct EvaluatedModel {
    // Sorted by semantic_id: the observable, deterministic assembly order.
    std::vector<BranchNodeGeometry> nodes;
    std::vector<Diagnostic> warnings;

    std::size_t total_vertices() const;
    std::size_t total_triangles() const;
    // Combined hash over (semantic_id, geometry_hash) pairs in order; the
    // cross-worker/cross-platform determinism witness.
    std::uint64_t model_hash() const;
    std::uint64_t model_topology_hash() const;
};

Result<EvaluatedModel> evaluate(const doc::Document& document, const EvaluationProfile& profile);

} // namespace canopy::eval
