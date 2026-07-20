#include "canopy/geometry/spline.hpp"

#include <algorithm>
#include <cmath>

namespace canopy::geo {

namespace {

// Centripetal Catmull–Rom knot spacing exponent.
double knot_interval(const Vec3& a, const Vec3& b) {
    const double d = length(b - a);
    return std::max(std::sqrt(d), 1e-9);
}

} // namespace

Result<Spline> Spline::create(std::vector<Vec3> control_points) {
    if (control_points.empty()) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "spline requires at least one control point");
    }
    for (const auto& point : control_points) {
        if (!finite(point)) {
            return Diagnostic::error(ErrorCode::invalid_argument,
                                     "spline control point is not finite");
        }
    }
    Spline spline;
    spline.points_ = std::move(control_points);

    const std::size_t segments = spline.points_.size() > 1 ? spline.points_.size() - 1 : 0;
    spline.table_.assign(segments * kSubdivisionsPerSegment + 1, 0.0);
    double accumulated = 0.0;
    Vec3 previous = spline.points_.front();
    std::size_t index = 1;
    for (std::size_t segment = 0; segment < segments; ++segment) {
        for (std::size_t step = 1; step <= kSubdivisionsPerSegment; ++step) {
            const double u = double(step) / double(kSubdivisionsPerSegment);
            const Vec3 point = spline.segment_point(segment, u);
            accumulated += length(point - previous);
            previous = point;
            spline.table_[index++] = accumulated;
        }
    }
    spline.total_length_ = accumulated;
    return spline;
}

Vec3 Spline::segment_point(std::size_t segment, double u) const {
    const std::size_t n = points_.size();
    // Endpoint phantom points mirror the end segments.
    const Vec3& p1 = points_[segment];
    const Vec3& p2 = points_[segment + 1];
    const Vec3 p0 = segment > 0 ? points_[segment - 1] : p1 + (p1 - p2);
    const Vec3 p3 = segment + 2 < n ? points_[segment + 2] : p2 + (p2 - p1);

    // Centripetal parameterization (Yuksel et al., published formulation).
    const double t0 = 0.0;
    const double t1 = t0 + knot_interval(p0, p1);
    const double t2 = t1 + knot_interval(p1, p2);
    const double t3 = t2 + knot_interval(p2, p3);
    const double t = t1 + u * (t2 - t1);

    auto lerp_ratio = [](double a, double b, double x) {
        return (b - a) <= 1e-12 ? 0.0 : (x - a) / (b - a);
    };
    auto mix = [](const Vec3& a, const Vec3& b, double s) { return a * (1.0 - s) + b * s; };

    const Vec3 a1 = mix(p0, p1, lerp_ratio(t0, t1, t));
    const Vec3 a2 = mix(p1, p2, lerp_ratio(t1, t2, t));
    const Vec3 a3 = mix(p2, p3, lerp_ratio(t2, t3, t));
    const Vec3 b1 = mix(a1, a2, lerp_ratio(t0, t2, t));
    const Vec3 b2 = mix(a2, a3, lerp_ratio(t1, t3, t));
    return mix(b1, b2, lerp_ratio(t1, t2, t));
}

Vec3 Spline::segment_derivative(std::size_t segment, double u) const {
    // Central difference in u; adequate for tangent direction (normalized by
    // callers) and deterministic.
    const double h = 1e-4;
    const double u0 = std::clamp(u - h, 0.0, 1.0);
    const double u1 = std::clamp(u + h, 0.0, 1.0);
    return (segment_point(segment, u1) - segment_point(segment, u0)) / std::max(u1 - u0, 1e-9);
}

void Spline::locate(double t, std::size_t& segment, double& u) const {
    segment = 0;
    u = 0.0;
    if (points_.size() < 2 || total_length_ <= 0.0) {
        return;
    }
    const double target = std::clamp(t, 0.0, 1.0) * total_length_;
    const auto it = std::lower_bound(table_.begin(), table_.end(), target);
    std::size_t index = std::size_t(std::distance(table_.begin(), it));
    if (index >= table_.size()) {
        index = table_.size() - 1;
    }
    if (index == 0) {
        segment = 0;
        u = 0.0;
        return;
    }
    const double before = table_[index - 1];
    const double after = table_[index];
    const double within = after > before ? (target - before) / (after - before) : 0.0;
    const double sample = double(index - 1) + within;
    segment = std::min(std::size_t(sample / double(kSubdivisionsPerSegment)),
                       points_.size() - 2);
    u = std::clamp(sample / double(kSubdivisionsPerSegment) - double(segment), 0.0, 1.0);
}

Vec3 Spline::position_at(double t) const {
    if (points_.empty()) {
        return Vec3{0.0, 0.0, 0.0};
    }
    if (points_.size() == 1) {
        return points_.front();
    }
    std::size_t segment = 0;
    double u = 0.0;
    locate(t, segment, u);
    return segment_point(segment, u);
}

Vec3 Spline::tangent_at(double t) const {
    static constexpr Vec3 kUp{0.0, 1.0, 0.0};
    if (points_.size() < 2 || total_length_ <= 0.0) {
        return kUp;
    }
    std::size_t segment = 0;
    double u = 0.0;
    locate(t, segment, u);
    return normalize_or(segment_derivative(segment, u), kUp);
}

std::vector<double> Spline::uniform_parameters(std::size_t count) const {
    if (count <= 1) {
        return {0.0};
    }
    std::vector<double> out(count);
    for (std::size_t i = 0; i < count; ++i) {
        out[i] = double(i) / double(count - 1);
    }
    return out;
}

} // namespace canopy::geo
