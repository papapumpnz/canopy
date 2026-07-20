// Rotation-minimizing parallel-transport frames (backlog B-027,
// 09_BRANCH_MESHING.md "Frame transport").
//
// Determinism: the initial normal comes from the caller (parent surface
// radial, user orientation) or the stable fallback — the least-aligned world
// cardinal axis; never pointer or thread order. Degenerate tangents reuse the
// previous frame instead of producing NaNs.
#pragma once

#include "canopy/foundation/diagnostics.hpp"
#include "canopy/foundation/vec.hpp"

#include <optional>
#include <vector>

namespace canopy::geo {

struct Frame {
    Vec3 tangent{0.0, 1.0, 0.0};
    Vec3 normal{1.0, 0.0, 0.0};
    Vec3 binormal{0.0, 0.0, 1.0};
};

// Stable fallback normal for a tangent: the world cardinal axis least aligned
// with it, projected to the tangent plane and normalized.
Vec3 stable_normal_for_tangent(const Vec3& unit_tangent);

// Transports an initial frame along the sample tangents using the
// double-reflection rotation-minimizing method (Wang et al. 2008, published
// algorithm). positions/tangents must be equal-length and non-empty; tangents
// need not be normalized. If initial_normal is absent (or parallel to the
// first tangent) the stable fallback is used.
Result<std::vector<Frame>> parallel_transport(const std::vector<Vec3>& positions,
                                              const std::vector<Vec3>& tangents,
                                              std::optional<Vec3> initial_normal);

} // namespace canopy::geo
