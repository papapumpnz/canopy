#include "canopy/foundation/curve.hpp"
#include "canopy_test.hpp"

using namespace canopy;

namespace {

Curve make(std::vector<CurveKey> keys, CurveInterpolation interpolation,
           CurveBoundary boundary = CurveBoundary::clamp) {
    auto curve = Curve::create(std::move(keys), interpolation, boundary);
    CHECK(curve.ok());
    return std::move(curve).value();
}

} // namespace

CANOPY_TEST(linear_interpolation_and_clamp) {
    const Curve curve = make({{0.0, 1.0, 0, 0, {}}, {1.0, 3.0, 0, 0, {}}},
                             CurveInterpolation::linear);
    CHECK_NEAR(curve.evaluate(0.0), 1.0, 1e-12);
    CHECK_NEAR(curve.evaluate(0.5), 2.0, 1e-12);
    CHECK_NEAR(curve.evaluate(1.0), 3.0, 1e-12);
    CHECK_NEAR(curve.evaluate(-5.0), 1.0, 1e-12); // clamp
    CHECK_NEAR(curve.evaluate(9.0), 3.0, 1e-12);
}

CANOPY_TEST(constant_and_single_key) {
    const Curve constant = Curve::constant_value(4.25);
    CHECK_NEAR(constant.evaluate(-1.0), 4.25, 1e-12);
    CHECK_NEAR(constant.evaluate(100.0), 4.25, 1e-12);
    const Curve step = make({{0.0, 1.0, 0, 0, {}}, {0.5, 2.0, 0, 0, {}}},
                            CurveInterpolation::constant);
    CHECK_NEAR(step.evaluate(0.25), 1.0, 1e-12);
    CHECK_NEAR(step.evaluate(0.75), 2.0, 1e-12);
}

CANOPY_TEST(smoothstep_midpoint_and_monotonicity) {
    const Curve curve = make({{0.0, 0.0, 0, 0, {}}, {1.0, 1.0, 0, 0, {}}},
                             CurveInterpolation::smoothstep);
    CHECK_NEAR(curve.evaluate(0.5), 0.5, 1e-12);
    double previous = -1.0;
    for (int i = 0; i <= 100; ++i) {
        const double value = curve.evaluate(double(i) / 100.0);
        CHECK(value >= previous);
        previous = value;
    }
}

CANOPY_TEST(monotone_cubic_no_overshoot) {
    // Step-like data must not overshoot [0, 1] anywhere (Fritsch–Carlson).
    const Curve curve = make({{0.0, 0.0, 0, 0, {}},
                              {0.4, 0.02, 0, 0, {}},
                              {0.6, 0.98, 0, 0, {}},
                              {1.0, 1.0, 0, 0, {}}},
                             CurveInterpolation::monotone_cubic);
    for (int i = 0; i <= 1000; ++i) {
        const double value = curve.evaluate(double(i) / 1000.0);
        CHECK(value >= -1e-9 && value <= 1.0 + 1e-9);
    }
}

CANOPY_TEST(boundary_modes) {
    const Curve repeat = make({{0.0, 0.0, 0, 0, {}}, {1.0, 1.0, 0, 0, {}}},
                              CurveInterpolation::linear, CurveBoundary::repeat);
    CHECK_NEAR(repeat.evaluate(1.25), 0.25, 1e-9);
    CHECK_NEAR(repeat.evaluate(-0.25), 0.75, 1e-9);
    const Curve mirror = make({{0.0, 0.0, 0, 0, {}}, {1.0, 1.0, 0, 0, {}}},
                              CurveInterpolation::linear, CurveBoundary::mirror);
    CHECK_NEAR(mirror.evaluate(1.25), 0.75, 1e-9);
    const Curve extrapolate = make({{0.0, 0.0, 0, 0, {}}, {1.0, 2.0, 0, 0, {}}},
                                   CurveInterpolation::linear, CurveBoundary::extrapolate);
    CHECK_NEAR(extrapolate.evaluate(2.0), 4.0, 1e-9);
    CHECK_NEAR(extrapolate.evaluate(-1.0), -2.0, 1e-9);
}

CANOPY_TEST(invalid_inputs_fail_deterministically) {
    CHECK(!Curve::create({}, CurveInterpolation::linear).ok());
    CHECK(!Curve::create({{0.0, 0.0, 0, 0, {}}, {0.0, 1.0, 0, 0, {}}},
                         CurveInterpolation::linear)
               .ok()); // repeated x
    CHECK(!Curve::create({{1.0, 0.0, 0, 0, {}}, {0.0, 1.0, 0, 0, {}}},
                         CurveInterpolation::linear)
               .ok()); // decreasing x
    CHECK(!Curve::create({{0.0, 0.0 / 0.0, 0, 0, {}}}, CurveInterpolation::linear).ok());
}

CANOPY_TEST_MAIN()
