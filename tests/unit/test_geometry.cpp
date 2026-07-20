#include "canopy/geometry/frames.hpp"
#include "canopy/geometry/spline.hpp"
#include "canopy/geometry/sweep.hpp"
#include "canopy_test.hpp"

using namespace canopy;
using namespace canopy::geo;

CANOPY_TEST(straight_spline_arc_length) {
    auto spline = Spline::create({{0, 0, 0}, {0, 1, 0}, {0, 2, 0}, {0, 4, 0}});
    CHECK(spline.ok());
    if (spline.ok()) {
        CHECK_NEAR(spline.value().total_length(), 4.0, 1e-6);
        const Vec3 mid = spline.value().position_at(0.5);
        CHECK_NEAR(mid.y, 2.0, 1e-3);
        CHECK_NEAR(mid.x, 0.0, 1e-9);
        const Vec3 tangent = spline.value().tangent_at(0.5);
        CHECK_NEAR(tangent.y, 1.0, 1e-6);
    }
}

CANOPY_TEST(degenerate_splines_are_safe) {
    auto single = Spline::create({{1, 2, 3}});
    CHECK(single.ok());
    if (single.ok()) {
        CHECK_NEAR(single.value().total_length(), 0.0, 1e-12);
        CHECK_NEAR(single.value().position_at(0.7).x, 1.0, 1e-12);
        CHECK_NEAR(single.value().tangent_at(0.3).y, 1.0, 1e-12); // +Y fallback
    }
    // Repeated identical points: zero length, no NaNs.
    auto repeated = Spline::create({{1, 1, 1}, {1, 1, 1}, {1, 1, 1}});
    CHECK(repeated.ok());
    if (repeated.ok()) {
        CHECK(finite(repeated.value().position_at(0.5)));
        CHECK(finite(repeated.value().tangent_at(0.5)));
    }
    CHECK(!Spline::create({}).ok());
    CHECK(!Spline::create({{0.0 / 0.0, 0, 0}}).ok());
}

CANOPY_TEST(frames_orthonormal_on_curved_path) {
    std::vector<Vec3> positions;
    std::vector<Vec3> tangents;
    for (int i = 0; i <= 50; ++i) {
        const double t = double(i) / 50.0;
        // Quarter turn: tangent rotates 90 degrees.
        positions.push_back({std::sin(t * 1.5707), 1.0 - std::cos(t * 1.5707), 0.0});
        tangents.push_back({std::cos(t * 1.5707), std::sin(t * 1.5707), 0.0});
    }
    auto frames = parallel_transport(positions, tangents, std::nullopt);
    CHECK(frames.ok());
    if (frames.ok()) {
        for (const Frame& frame : frames.value()) {
            CHECK_NEAR(length(frame.tangent), 1.0, 1e-6);
            CHECK_NEAR(length(frame.normal), 1.0, 1e-6);
            CHECK_NEAR(dot(frame.tangent, frame.normal), 0.0, 1e-6);
            CHECK_NEAR(dot(frame.normal, frame.binormal), 0.0, 1e-6);
        }
        // Rotation-minimizing: the normal must not spin around the tangent.
        // Compare consecutive normals: angle change stays small.
        for (std::size_t i = 1; i < frames.value().size(); ++i) {
            CHECK(dot(frames.value()[i].normal, frames.value()[i - 1].normal) > 0.99);
        }
    }
}

CANOPY_TEST(frames_handle_reversal_without_nans) {
    // Tangent reverses direction: degenerate case from B-027 acceptance.
    std::vector<Vec3> positions = {{0, 0, 0}, {0, 1, 0}, {0, 0.5, 0}};
    std::vector<Vec3> tangents = {{0, 1, 0}, {0, 1, 0}, {0, -1, 0}};
    auto frames = parallel_transport(positions, tangents, std::nullopt);
    CHECK(frames.ok());
}

CANOPY_TEST(sweep_topology_counts) {
    std::vector<SpineSample> samples;
    std::vector<Frame> frames;
    const std::size_t rings = 5;
    for (std::size_t i = 0; i < rings; ++i) {
        const double t = double(i) / double(rings - 1);
        samples.push_back({{0, t * 2.0, 0}, {0, 1, 0}, t * 2.0, t, 0.2});
        frames.push_back(Frame{{0, 1, 0}, {1, 0, 0}, {0, 0, 1}});
    }
    SweepOptions options;
    options.radial_segments = 8;
    auto mesh = sweep_branch(samples, frames, options);
    CHECK(mesh.ok());
    if (mesh.ok()) {
        // 5 rings * 9 verts + cap ring 9 + center = 55 vertices.
        CHECK_EQ(mesh.value().vertex_count(), std::size_t{55});
        // Sides: 4 * 8 * 2 = 64; cap fan: 8 → 72 triangles.
        CHECK_EQ(mesh.value().triangle_count(), std::size_t{72});
        CHECK(validate_mesh(mesh.value()).ok());
        // Deterministic topology hash: identical input → identical hash.
        auto mesh2 = sweep_branch(samples, frames, options);
        CHECK(mesh2.ok() && topology_hash(mesh.value()) == topology_hash(mesh2.value()));
        CHECK(geometry_hash(mesh.value()) == geometry_hash(mesh2.value()));
    }
}

CANOPY_TEST(sweep_point_tip) {
    std::vector<SpineSample> samples = {
        {{0, 0, 0}, {0, 1, 0}, 0.0, 0.0, 0.3},
        {{0, 1, 0}, {0, 1, 0}, 1.0, 0.5, 0.2},
        {{0, 2, 0}, {0, 1, 0}, 2.0, 1.0, 0.0},
    };
    std::vector<Frame> frames(3, Frame{{0, 1, 0}, {1, 0, 0}, {0, 0, 1}});
    SweepOptions options;
    options.radial_segments = 6;
    auto mesh = sweep_branch(samples, frames, options);
    CHECK(mesh.ok());
    if (mesh.ok()) {
        // 2 rings * 7 + tip point = 15 vertices; 1 * 6 * 2 + 6 = 18 triangles.
        CHECK_EQ(mesh.value().vertex_count(), std::size_t{15});
        CHECK_EQ(mesh.value().triangle_count(), std::size_t{18});
    }
}

CANOPY_TEST(lobed_rings_modulate_radius_with_stable_topology) {
    std::vector<SpineSample> samples;
    std::vector<Frame> frames;
    for (int i = 0; i < 4; ++i) {
        const double t = double(i) / 3.0;
        // Full lobe influence at the base, none at the top.
        samples.push_back({{0, t, 0}, {0, 1, 0}, t, t, 0.5, 1.0 - t});
        frames.push_back(Frame{{0, 1, 0}, {1, 0, 0}, {0, 0, 1}});
    }
    // 16 segments with 4 lobes puts ring samples exactly on lobe extrema.
    SweepOptions circular;
    circular.radial_segments = 16;
    SweepOptions lobed = circular;
    lobed.lobe_count = 4;
    lobed.lobe_amplitude = 0.2;

    auto plain = sweep_branch(samples, frames, circular);
    auto buttressed = sweep_branch(samples, frames, lobed);
    CHECK(plain.ok() && buttressed.ok());
    if (plain.ok() && buttressed.ok()) {
        // Same connectivity, different geometry.
        CHECK(topology_hash(plain.value()) == topology_hash(buttressed.value()));
        CHECK(geometry_hash(plain.value()) != geometry_hash(buttressed.value()));
        CHECK(validate_mesh(buttressed.value()).ok());
        // Base ring radius varies within [R(1-a), R(1+a)]; top ring stays
        // circular because lobe_scale is 0 there.
        double base_min = 1e9;
        double base_max = 0.0;
        const auto& mesh = buttressed.value();
        for (std::size_t k = 0; k <= 16; ++k) {
            const Vec3& p = mesh.positions[k];
            const double r = std::sqrt(p.x * p.x + p.z * p.z);
            base_min = std::min(base_min, r);
            base_max = std::max(base_max, r);
        }
        CHECK_NEAR(base_min, 0.5 * 0.8, 1e-6);
        CHECK_NEAR(base_max, 0.5 * 1.2, 1e-6);
    }
}

CANOPY_TEST(uv_v_offset_shifts_bark_uvs_only) {
    std::vector<SpineSample> samples = {
        {{0, 0, 0}, {0, 1, 0}, 0.0, 0.0, 0.2, 0.0},
        {{0, 1, 0}, {0, 1, 0}, 1.0, 1.0, 0.2, 0.0},
    };
    std::vector<Frame> frames(2, Frame{{0, 1, 0}, {1, 0, 0}, {0, 0, 1}});
    SweepOptions options;
    options.cap_tip = false;
    SweepOptions shifted = options;
    shifted.uv_v_offset = 0.37;
    auto plain = sweep_branch(samples, frames, options);
    auto moved = sweep_branch(samples, frames, shifted);
    CHECK(plain.ok() && moved.ok());
    if (plain.ok() && moved.ok()) {
        CHECK(topology_hash(plain.value()) == topology_hash(moved.value()));
        CHECK_NEAR(moved.value().uvs[0].y - plain.value().uvs[0].y, 0.37, 1e-9);
        // Positions unchanged.
        CHECK_NEAR(moved.value().positions[0].x, plain.value().positions[0].x, 1e-12);
    }
}

CANOPY_TEST(sweep_rejects_invalid_input) {
    std::vector<SpineSample> samples = {{{0, 0, 0}, {0, 1, 0}, 0.0, 0.0, 0.2}};
    std::vector<Frame> frames = {Frame{}};
    SweepOptions options;
    CHECK(!sweep_branch(samples, frames, options).ok()); // too few samples
    samples.push_back({{0, 1, 0}, {0, 1, 0}, 1.0, 1.0, 0.2});
    frames.push_back(Frame{});
    options.radial_segments = 2;
    CHECK(!sweep_branch(samples, frames, options).ok()); // too few segments
    options.radial_segments = 8;
    samples[0].radius = -1.0;
    CHECK(!sweep_branch(samples, frames, options).ok()); // negative radius
}

CANOPY_TEST_MAIN()
