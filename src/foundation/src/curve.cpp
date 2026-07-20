#include "canopy/foundation/curve.hpp"

#include <algorithm>
#include <cmath>

namespace canopy {

Result<Curve> Curve::create(std::vector<CurveKey> keys, CurveInterpolation interpolation,
                            CurveBoundary boundary) {
    if (keys.empty()) {
        return Diagnostic::error(ErrorCode::invalid_argument, "curve requires at least one key");
    }
    for (std::size_t i = 0; i < keys.size(); ++i) {
        const auto& key = keys[i];
        if (!std::isfinite(key.x) || !std::isfinite(key.y) || !std::isfinite(key.tangent_in) ||
            !std::isfinite(key.tangent_out)) {
            return Diagnostic::error(ErrorCode::invalid_argument,
                                     "curve key " + std::to_string(i) + " has non-finite values");
        }
        if (i > 0 && !(keys[i - 1].x < key.x)) {
            return Diagnostic::error(ErrorCode::invalid_argument,
                                     "curve keys must have strictly increasing x (key " +
                                         std::to_string(i) + ")");
        }
    }

    Curve curve;
    curve.keys_ = std::move(keys);
    curve.interpolation_ = interpolation;
    curve.boundary_ = boundary;

    if (interpolation == CurveInterpolation::monotone_cubic && curve.keys_.size() >= 2) {
        // Fritsch–Carlson tangent limiting: monotone data stays monotone.
        const auto& k = curve.keys_;
        const std::size_t n = k.size();
        std::vector<double> slopes(n - 1);
        for (std::size_t i = 0; i + 1 < n; ++i) {
            slopes[i] = (k[i + 1].y - k[i].y) / (k[i + 1].x - k[i].x);
        }
        std::vector<double> tangents(n);
        tangents[0] = slopes[0];
        tangents[n - 1] = slopes[n - 2];
        for (std::size_t i = 1; i + 1 < n; ++i) {
            tangents[i] = (slopes[i - 1] * slopes[i] <= 0.0) ? 0.0
                                                             : 0.5 * (slopes[i - 1] + slopes[i]);
        }
        for (std::size_t i = 0; i + 1 < n; ++i) {
            if (slopes[i] == 0.0) {
                tangents[i] = 0.0;
                tangents[i + 1] = 0.0;
                continue;
            }
            const double a = tangents[i] / slopes[i];
            const double b = tangents[i + 1] / slopes[i];
            const double s = a * a + b * b;
            if (s > 9.0) {
                const double scale = 3.0 / std::sqrt(s);
                tangents[i] = scale * a * slopes[i];
                tangents[i + 1] = scale * b * slopes[i];
            }
        }
        curve.monotone_tangents_ = std::move(tangents);
    }
    return curve;
}

Curve Curve::constant_value(double value) {
    Curve curve;
    curve.keys_.push_back(CurveKey{0.0, value, 0.0, 0.0, {}});
    curve.interpolation_ = CurveInterpolation::constant;
    curve.boundary_ = CurveBoundary::clamp;
    return curve;
}

namespace {

double hermite(double y0, double m0, double y1, double m1, double t, double dx) {
    const double t2 = t * t;
    const double t3 = t2 * t;
    return (2.0 * t3 - 3.0 * t2 + 1.0) * y0 + (t3 - 2.0 * t2 + t) * dx * m0 +
           (-2.0 * t3 + 3.0 * t2) * y1 + (t3 - t2) * dx * m1;
}

} // namespace

double Curve::evaluate(double x) const {
    const std::size_t n = keys_.size();
    if (n == 1) {
        return keys_[0].y;
    }
    const double x_min = keys_.front().x;
    const double x_max = keys_.back().x;
    const double span = x_max - x_min;

    // Boundary handling maps x into the key domain (or extrapolates below).
    if (x < x_min || x > x_max) {
        switch (boundary_) {
        case CurveBoundary::clamp:
            return (x < x_min) ? keys_.front().y : keys_.back().y;
        case CurveBoundary::repeat: {
            double t = std::fmod(x - x_min, span);
            if (t < 0.0) {
                t += span;
            }
            x = x_min + t;
            break;
        }
        case CurveBoundary::mirror: {
            double t = std::fmod(x - x_min, 2.0 * span);
            if (t < 0.0) {
                t += 2.0 * span;
            }
            x = x_min + (t <= span ? t : 2.0 * span - t);
            break;
        }
        case CurveBoundary::extrapolate: {
            // Continue the end segment linearly using its local slope.
            if (x < x_min) {
                const double slope = (keys_[1].y - keys_[0].y) / (keys_[1].x - keys_[0].x);
                return keys_[0].y + slope * (x - x_min);
            }
            const double slope =
                (keys_[n - 1].y - keys_[n - 2].y) / (keys_[n - 1].x - keys_[n - 2].x);
            return keys_[n - 1].y + slope * (x - x_max);
        }
        }
    }

    // Find the segment: last key with key.x <= x.
    const auto it = std::upper_bound(keys_.begin(), keys_.end(), x,
                                     [](double lhs, const CurveKey& key) { return lhs < key.x; });
    std::size_t index = std::size_t(std::distance(keys_.begin(), it));
    if (index == 0) {
        return keys_.front().y;
    }
    if (index >= n) {
        index = n - 1;
    }
    const CurveKey& left = keys_[index - 1];
    const CurveKey& right = keys_[index];
    const double dx = right.x - left.x;
    const double t = std::clamp((x - left.x) / dx, 0.0, 1.0);

    switch (interpolation_) {
    case CurveInterpolation::constant:
        return left.y;
    case CurveInterpolation::linear:
        return left.y + (right.y - left.y) * t;
    case CurveInterpolation::smoothstep: {
        const double s = t * t * (3.0 - 2.0 * t);
        return left.y + (right.y - left.y) * s;
    }
    case CurveInterpolation::cubic_hermite:
        return hermite(left.y, left.tangent_out, right.y, right.tangent_in, t, dx);
    case CurveInterpolation::monotone_cubic: {
        const double m0 = monotone_tangents_[index - 1];
        const double m1 = monotone_tangents_[index];
        return hermite(left.y, m0, right.y, m1, t, dx);
    }
    }
    return left.y;
}

} // namespace canopy
