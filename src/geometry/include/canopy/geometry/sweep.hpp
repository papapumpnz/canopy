// Branch sweep meshing (backlog B-028/B-029, 09_BRANCH_MESHING.md).
//
// Bootstrap scope: uniform radial segment count per branch, circular profile,
// ring stitching, flat tip cap, bark UVs (U = angle with explicit seam,
// V = arc length / tile length), analytical sweep normals. Adaptive rings,
// junction strategies, displacement, and tangents arrive in later epics.
#pragma once

#include "canopy/foundation/diagnostics.hpp"
#include "canopy/foundation/vec.hpp"
#include "canopy/geometry/frames.hpp"

#include <cstdint>
#include <vector>

namespace canopy::geo {

// One resampled point of a branch spine (09_BRANCH_MESHING.md SpineSample).
struct SpineSample {
    Vec3 position;
    Vec3 tangent;
    double arc_length = 0.0;        // meters from branch base
    double normalized_length = 0.0; // 0 at base, 1 at tip
    double radius = 0.0;            // meters
};

struct TriangleMesh {
    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<Vec2> uvs;
    std::vector<std::uint32_t> indices; // CCW winding, outward-facing

    std::size_t vertex_count() const { return positions.size(); }
    std::size_t triangle_count() const { return indices.size() / 3; }
};

struct SweepOptions {
    std::uint32_t radial_segments = 8; // >= 3
    double uv_tile_length_m = 2.0;     // bark V tiling; > 0
    bool cap_tip = true;
};

// Sweeps rings along the samples using the given frames (one per sample).
// Requirements: >= 2 samples, frames.size() == samples.size(), radii > 0
// except the last sample may taper to >= 0 (a zero tip radius emits a point
// tip instead of a cap ring).
Result<TriangleMesh> sweep_branch(const std::vector<SpineSample>& samples,
                                  const std::vector<Frame>& frames, const SweepOptions& options);

// Mesh quality checks (09_BRANCH_MESHING.md): finite attributes, index range,
// non-degenerate triangles, bounds containment. Returns diagnostics with the
// first offending element identified.
Result<void> validate_mesh(const TriangleMesh& mesh);

// Stable topology hash: connectivity and counts only (no positions), equal
// across platforms and worker counts for identical inputs.
std::uint64_t topology_hash(const TriangleMesh& mesh);

// Content hash including quantized geometry (positions/normals/uvs rounded to
// 1e-6) for cross-platform comparison of full mesh output.
std::uint64_t geometry_hash(const TriangleMesh& mesh);

} // namespace canopy::geo
