// Piecewise curve foundation (backlog B-026).
//
// A Spline interpolates ordered control points with centripetal Catmull–Rom
// segments (C1, no self-oscillation on repeated points) and exposes
// arc-length-parameterized queries. Degenerate inputs (single point, zero
// length) are valid and answer conservatively rather than producing NaNs.
//
// Units: meters. Determinism: all queries are pure functions of the control
// points; the arc-length table uses a fixed per-segment subdivision count.
#pragma once

#include "canopy/foundation/diagnostics.hpp"
#include "canopy/foundation/vec.hpp"

#include <vector>

namespace canopy::geo {

class Spline {
public:
    // Empty spline: zero length, queries answer origin / +Y.
    Spline() = default;

    // Control points must be finite; at least one required.
    static Result<Spline> create(std::vector<Vec3> control_points);

    double total_length() const { return total_length_; }

    // t is normalized arc length in [0, 1], clamped outside.
    Vec3 position_at(double t) const;
    // Unit tangent; for degenerate (zero-length) splines returns +Y.
    Vec3 tangent_at(double t) const;

    // count >= 2 positions at uniform normalized arc length (includes both
    // endpoints). count <= 1 returns the start point once.
    std::vector<double> uniform_parameters(std::size_t count) const;

    const std::vector<Vec3>& control_points() const { return points_; }

private:
    Vec3 segment_point(std::size_t segment, double u) const;
    Vec3 segment_derivative(std::size_t segment, double u) const;
    // Maps normalized arc length to (segment, local u).
    void locate(double t, std::size_t& segment, double& u) const;

    std::vector<Vec3> points_;
    // Cumulative arc length at fixed subdivisions: table_[i] is the length up
    // to sample i; samples are per-segment uniform in u.
    std::vector<double> table_;
    double total_length_ = 0.0;
    static constexpr std::size_t kSubdivisionsPerSegment = 16;
};

} // namespace canopy::geo
