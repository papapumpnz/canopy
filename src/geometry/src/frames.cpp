#include "canopy/geometry/frames.hpp"

#include <cmath>

namespace canopy::geo {

Vec3 stable_normal_for_tangent(const Vec3& unit_tangent) {
    // Least-aligned cardinal axis; ties resolve in fixed X, Y, Z order.
    const double ax = std::fabs(unit_tangent.x);
    const double ay = std::fabs(unit_tangent.y);
    const double az = std::fabs(unit_tangent.z);
    Vec3 axis;
    if (ax <= ay && ax <= az) {
        axis = Vec3{1.0, 0.0, 0.0};
    } else if (ay <= az) {
        axis = Vec3{0.0, 1.0, 0.0};
    } else {
        axis = Vec3{0.0, 0.0, 1.0};
    }
    const Vec3 projected = axis - unit_tangent * dot(axis, unit_tangent);
    return normalize_or(projected, Vec3{1.0, 0.0, 0.0});
}

Result<std::vector<Frame>> parallel_transport(const std::vector<Vec3>& positions,
                                              const std::vector<Vec3>& tangents,
                                              std::optional<Vec3> initial_normal) {
    if (positions.empty() || positions.size() != tangents.size()) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "parallel_transport requires equal non-empty positions/tangents");
    }
    const std::size_t n = positions.size();
    std::vector<Frame> frames(n);

    static constexpr Vec3 kUp{0.0, 1.0, 0.0};
    Vec3 t0 = normalize_or(tangents[0], kUp);
    Vec3 n0;
    if (initial_normal.has_value()) {
        const Vec3 projected = *initial_normal - t0 * dot(*initial_normal, t0);
        n0 = normalize_or(projected, stable_normal_for_tangent(t0));
    } else {
        n0 = stable_normal_for_tangent(t0);
    }
    frames[0] = Frame{t0, n0, cross(t0, n0)};

    for (std::size_t i = 0; i + 1 < n; ++i) {
        const Frame& current = frames[i];
        const Vec3 next_tangent = normalize_or(tangents[i + 1], current.tangent);

        // Double-reflection step.
        const Vec3 delta = positions[i + 1] - positions[i];
        const double c1 = dot(delta, delta);
        Vec3 normal_l;
        Vec3 tangent_l;
        if (c1 <= 1e-24) {
            // Coincident samples: transport by tangent rotation only.
            normal_l = current.normal;
            tangent_l = current.tangent;
        } else {
            normal_l = current.normal - delta * (2.0 / c1 * dot(delta, current.normal));
            tangent_l = current.tangent - delta * (2.0 / c1 * dot(delta, current.tangent));
        }
        const Vec3 v2 = next_tangent - tangent_l;
        const double c2 = dot(v2, v2);
        Vec3 next_normal =
            c2 <= 1e-24 ? normal_l : normal_l - v2 * (2.0 / c2 * dot(v2, normal_l));
        // Re-orthonormalize to contain drift.
        next_normal = next_normal - next_tangent * dot(next_normal, next_tangent);
        next_normal = normalize_or(next_normal, stable_normal_for_tangent(next_tangent));
        frames[i + 1] = Frame{next_tangent, next_normal, cross(next_tangent, next_normal)};
    }

    for (const Frame& frame : frames) {
        if (!finite(frame.tangent) || !finite(frame.normal) || !finite(frame.binormal)) {
            return Diagnostic::error(ErrorCode::geometry_invalid,
                                     "parallel transport produced non-finite frame");
        }
    }
    return frames;
}

} // namespace canopy::geo
