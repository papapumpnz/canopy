// Curve evaluation (backlog B-019, data model "Curves").
//
// A curve maps a normalized or physical X domain to Y through typed keys.
// Interpolation and boundary behavior are explicit per curve. Evaluation is
// deterministic and allocation-free after construction.
#pragma once

#include "canopy/foundation/diagnostics.hpp"

#include <string>
#include <vector>

namespace canopy {

enum class CurveInterpolation : std::uint8_t {
    constant,      // value of the left key
    linear,
    smoothstep,    // 3t^2 - 2t^3 blend between keys
    cubic_hermite, // uses per-key tangents
    monotone_cubic // Fritsch–Carlson, no overshoot
};

enum class CurveBoundary : std::uint8_t {
    clamp,       // hold end value
    repeat,      // wrap X into the domain
    mirror,      // reflect X into the domain
    extrapolate  // continue the end segment linearly
};

struct CurveKey {
    double x = 0.0;
    double y = 0.0;
    // Hermite tangents (dy/dx); ignored by other interpolation modes.
    double tangent_in = 0.0;
    double tangent_out = 0.0;
    // Stable key identity for merge tools; empty allowed in bootstrap.
    std::string id;
};

class Curve {
public:
    // Keys must be finite with strictly increasing x; at least one key.
    static Result<Curve> create(std::vector<CurveKey> keys,
                                CurveInterpolation interpolation = CurveInterpolation::linear,
                                CurveBoundary boundary = CurveBoundary::clamp);

    // Convenience: the constant curve y(x) = value.
    static Curve constant_value(double value);

    double evaluate(double x) const;

    const std::vector<CurveKey>& keys() const { return keys_; }
    CurveInterpolation interpolation() const { return interpolation_; }
    CurveBoundary boundary() const { return boundary_; }

private:
    Curve() = default;

    std::vector<CurveKey> keys_;
    std::vector<double> monotone_tangents_; // precomputed for monotone_cubic
    CurveInterpolation interpolation_ = CurveInterpolation::linear;
    CurveBoundary boundary_ = CurveBoundary::clamp;
};

} // namespace canopy
