#include "canopy/geometry/polygon.hpp"

#include <cmath>

namespace canopy::geo {

namespace {

double cross2(const Vec2& o, const Vec2& a, const Vec2& b) {
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

bool point_in_triangle(const Vec2& p, const Vec2& a, const Vec2& b, const Vec2& c) {
    const double d1 = cross2(a, b, p);
    const double d2 = cross2(b, c, p);
    const double d3 = cross2(c, a, p);
    const bool has_negative = (d1 < 0.0) || (d2 < 0.0) || (d3 < 0.0);
    const bool has_positive = (d1 > 0.0) || (d2 > 0.0) || (d3 > 0.0);
    return !(has_negative && has_positive);
}

} // namespace

Result<std::vector<std::uint32_t>> triangulate_polygon(const std::vector<Vec2>& outline) {
    const std::size_t n = outline.size();
    if (n < 3) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "polygon requires at least 3 vertices");
    }
    for (const Vec2& v : outline) {
        if (!std::isfinite(v.x) || !std::isfinite(v.y)) {
            return Diagnostic::error(ErrorCode::invalid_argument,
                                     "polygon vertex is not finite");
        }
    }

    // Signed area decides winding; work on an index list in CCW order.
    double signed_area = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const Vec2& a = outline[i];
        const Vec2& b = outline[(i + 1) % n];
        signed_area += a.x * b.y - b.x * a.y;
    }
    if (std::fabs(signed_area) < 1e-12) {
        return Diagnostic::error(ErrorCode::invalid_argument, "polygon has zero area");
    }
    std::vector<std::uint32_t> indices(n);
    for (std::size_t i = 0; i < n; ++i) {
        indices[i] = std::uint32_t(signed_area > 0.0 ? i : n - 1 - i);
    }

    std::vector<std::uint32_t> triangles;
    triangles.reserve((n - 2) * 3);
    std::size_t guard = 0;
    const std::size_t guard_limit = n * n + 16;
    std::size_t i = 0;
    while (indices.size() > 3) {
        if (++guard > guard_limit) {
            return Diagnostic::error(ErrorCode::geometry_invalid,
                                     "polygon is not simple (no clippable ear found)");
        }
        const std::size_t count = indices.size();
        const std::uint32_t ia = indices[(i + count - 1) % count];
        const std::uint32_t ib = indices[i % count];
        const std::uint32_t ic = indices[(i + 1) % count];
        const Vec2& a = outline[ia];
        const Vec2& b = outline[ib];
        const Vec2& c = outline[ic];
        bool is_ear = cross2(a, b, c) > 1e-14; // strictly convex corner (CCW)
        if (is_ear) {
            for (const std::uint32_t other : indices) {
                if (other == ia || other == ib || other == ic) {
                    continue;
                }
                if (point_in_triangle(outline[other], a, b, c)) {
                    is_ear = false;
                    break;
                }
            }
        }
        if (is_ear) {
            triangles.insert(triangles.end(), {ia, ib, ic});
            indices.erase(indices.begin() + std::ptrdiff_t(i % count));
            guard = 0;
        } else {
            ++i;
        }
        if (i >= indices.size()) {
            i = 0;
        }
    }
    triangles.insert(triangles.end(), {indices[0], indices[1], indices[2]});
    return triangles;
}

} // namespace canopy::geo
