#include "canopy/evaluation/evaluate.hpp"

#include "canopy/foundation/curve.hpp"
#include "canopy/foundation/random.hpp"
#include "canopy/geometry/frames.hpp"
#include "canopy/geometry/polygon.hpp"
#include "canopy/geometry/spline.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <numbers>

namespace canopy::eval {

EvaluationProfile EvaluationProfile::draft() {
    return {"draft", 4.0, 4, 24, 6};
}
EvaluationProfile EvaluationProfile::preview() {
    return {"preview", 8.0, 6, 48, 12};
}
EvaluationProfile EvaluationProfile::production() {
    return {"production", 12.0, 8, 96, 32};
}

Result<EvaluationProfile> EvaluationProfile::by_name(std::string_view name) {
    if (name == "draft") {
        return draft();
    }
    if (name == "preview") {
        return preview();
    }
    if (name == "production") {
        return production();
    }
    return Diagnostic::error(ErrorCode::invalid_argument,
                             "unknown evaluation profile '" + std::string(name) +
                                 "' (known: draft, preview, production)");
}

std::size_t EvaluatedModel::total_vertices() const {
    std::size_t total = 0;
    for (const auto& node : nodes) {
        total += node.mesh.vertex_count();
    }
    return total;
}

std::size_t EvaluatedModel::total_triangles() const {
    std::size_t total = 0;
    for (const auto& node : nodes) {
        total += node.mesh.triangle_count();
    }
    return total;
}

namespace {

void hash_u64(Sha256& hasher, std::uint64_t value) {
    std::uint8_t bytes[8];
    for (int i = 0; i < 8; ++i) {
        bytes[i] = std::uint8_t((value >> (8 * i)) & 0xffu);
    }
    hasher.update(bytes, sizeof bytes);
}

std::uint64_t combine_node_hashes(const std::vector<BranchNodeGeometry>& nodes,
                                  std::string_view domain, bool topology_only) {
    Sha256 hasher;
    hasher.update(domain);
    hash_u64(hasher, nodes.size());
    for (const auto& node : nodes) {
        hash_u64(hasher, node.semantic_id.value);
        hash_u64(hasher, topology_only ? geo::topology_hash(node.mesh)
                                       : geo::geometry_hash(node.mesh));
    }
    return hasher.finish().low64();
}

} // namespace

std::uint64_t EvaluatedModel::model_hash() const {
    return combine_node_hashes(nodes, "canopy-model-v1", false);
}

std::uint64_t EvaluatedModel::model_topology_hash() const {
    return combine_node_hashes(nodes, "canopy-model-topology-v1", true);
}

namespace {

inline constexpr std::uint32_t kSemanticAlgorithmVersion = 1;

// Semantic ID derivation per 06_DATA_MODEL_AND_FILE_FORMATS.md.
SemanticId derive_semantic_id(std::uint64_t document_seed, const Uuid& generator_id,
                              SemanticId parent_semantic_id, std::string_view generation_mode,
                              std::uint64_t stable_ordinal_key) {
    Sha256 hasher;
    hasher.update(std::string_view("canopy-sem-v1"));
    hash_u64(hasher, document_seed);
    hasher.update(generator_id.bytes.data(), generator_id.bytes.size());
    hash_u64(hasher, parent_semantic_id.value);
    hash_u64(hasher, generation_mode.size());
    hasher.update(generation_mode);
    hash_u64(hasher, stable_ordinal_key);
    hash_u64(hasher, kSemanticAlgorithmVersion);
    return SemanticId{hasher.finish().low64()};
}

// --- typed property access -------------------------------------------------

double get_number(const doc::GeneratorInstance& generator, std::string_view key,
                  double fallback) {
    const auto it = generator.properties.find(key);
    if (it == generator.properties.end() || !it->second.is_number()) {
        return fallback;
    }
    return it->second.as_number();
}

bool has_property(const doc::GeneratorInstance& generator, std::string_view key) {
    return generator.properties.find(key) != generator.properties.end();
}

bool get_bool(const doc::GeneratorInstance& generator, std::string_view key, bool fallback) {
    const auto it = generator.properties.find(key);
    if (it == generator.properties.end() || !it->second.is_bool()) {
        return fallback;
    }
    return it->second.as_bool();
}

std::string get_string(const doc::GeneratorInstance& generator, std::string_view key,
                       std::string fallback) {
    const auto it = generator.properties.find(key);
    if (it == generator.properties.end() || !it->second.is_string()) {
        return fallback;
    }
    return it->second.as_string();
}

// Curve properties serialize as {"interpolation": "...", "keys": [[x, y], ...]}.
Result<Curve> get_curve(const doc::GeneratorInstance& generator, std::string_view key,
                        const Curve& fallback) {
    const auto it = generator.properties.find(key);
    if (it == generator.properties.end()) {
        return fallback;
    }
    const json::Value& value = it->second;
    if (!value.is_object()) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "property '" + std::string(key) + "' must be a curve object");
    }
    CurveInterpolation interpolation = CurveInterpolation::linear;
    if (const auto* mode = value.find("interpolation"); mode != nullptr && mode->is_string()) {
        const std::string& name = mode->as_string();
        if (name == "linear") {
            interpolation = CurveInterpolation::linear;
        } else if (name == "constant") {
            interpolation = CurveInterpolation::constant;
        } else if (name == "smoothstep") {
            interpolation = CurveInterpolation::smoothstep;
        } else if (name == "monotone_cubic") {
            interpolation = CurveInterpolation::monotone_cubic;
        } else {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "property '" + std::string(key) +
                                         "': unknown curve interpolation '" + name + "'");
        }
    }
    const auto* keys = value.find("keys");
    if (keys == nullptr || !keys->is_array() || keys->as_array().empty()) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "property '" + std::string(key) + "': curve requires keys");
    }
    std::vector<CurveKey> curve_keys;
    for (const auto& entry : keys->as_array()) {
        if (!entry.is_array() || entry.as_array().size() != 2 ||
            !entry.as_array()[0].is_number() || !entry.as_array()[1].is_number()) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "property '" + std::string(key) +
                                         "': curve keys must be [x, y] number pairs");
        }
        curve_keys.push_back(CurveKey{entry.as_array()[0].as_number(),
                                      entry.as_array()[1].as_number(), 0.0, 0.0, {}});
    }
    auto curve = Curve::create(std::move(curve_keys), interpolation, CurveBoundary::clamp);
    if (!curve.ok()) {
        Diagnostic error = Diagnostic::error(ErrorCode::schema_violation,
                                             "property '" + std::string(key) + "': invalid curve");
        error.with_note(curve.take_error());
        return error;
    }
    return std::move(curve).value();
}

// --- evaluation ------------------------------------------------------------

// A generated node instance available as a parent for descendants.
struct EvaluatedNode {
    SemanticId semantic_id;
    geo::Spline spline;           // world-space spine
    std::vector<double> radii_t;  // unused for root; radius lookup via curve
    double length_m = 0.0;
    double base_radius_m = 0.0;
    Curve radius_profile = Curve::constant_value(0.0);
    geo::Frame base_frame;
    std::uint32_t depth = 0; // generator-graph distance from the root
};

struct Placement {
    std::uint64_t ordinal = 0;
    double parent_t = 0.0; // normalized position along the parent spine
    // Mode-driven azimuth (phyllotaxy, bifurcation). Unset → property or
    // named-stream random azimuth in the orientation step.
    std::optional<double> azimuth_deg;
};

// Candidate placements for one generator under one parent node (deterministic;
// 07_EVALUATION_ENGINE.md: absolute, interval, phyllotaxy, proportional,
// bifurcation subset).
Result<std::vector<Placement>> make_placements(const doc::GeneratorInstance& generator,
                                               bool parent_is_root, double parent_length_m) {
    const std::string mode = get_string(generator, "generation.mode", "absolute");
    const double first = std::clamp(get_number(generator, "generation.first", 0.0), 0.0, 1.0);
    const double last = std::clamp(get_number(generator, "generation.last", 1.0), 0.0, 1.0);
    if (!(last >= first)) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "generation.last must be >= generation.first");
    }

    std::vector<Placement> placements;
    if (mode == "absolute") {
        const double count_value = get_number(generator, "generation.count", 1.0);
        if (!(count_value >= 0.0) || count_value > 4096.0) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "generation.count out of range [0, 4096]");
        }
        const auto count = std::uint64_t(count_value);
        for (std::uint64_t i = 0; i < count; ++i) {
            // On the root there is no spine; children attach at the origin.
            const double t =
                parent_is_root
                    ? 0.0
                    : (count == 1 ? first
                                  : first + (last - first) * double(i) / double(count - 1));
            placements.push_back(Placement{i, t, std::nullopt});
        }
    } else if (mode == "interval") {
        if (parent_is_root) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "interval mode requires a parent with a spine");
        }
        const double spacing = get_number(generator, "generation.spacing.relative", 0.0);
        if (!(spacing > 0.0) || spacing > 1.0) {
            return Diagnostic::error(
                ErrorCode::schema_violation,
                "interval mode requires generation.spacing.relative in (0, 1]");
        }
        // Stable ordinal = interval index from the parent base; unchanged when
        // `last` grows (B-025 groundwork).
        std::uint64_t ordinal = 0;
        for (double t = first; t <= last + 1e-12; t += spacing) {
            placements.push_back(Placement{ordinal++, std::min(t, 1.0), std::nullopt});
            if (placements.size() > 4096) {
                return Diagnostic::error(ErrorCode::schema_violation,
                                         "interval mode produced more than 4096 placements");
            }
        }
    } else if (mode == "phyllotaxy") {
        // 07_EVALUATION_ENGINE "Phyllotaxy": whorls placed at an internode
        // spacing; each whorl advances by the divergence angle (golden angle
        // default) plus twist; members share the whorl evenly.
        if (parent_is_root) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "phyllotaxy mode requires a parent with a spine");
        }
        const double internode = get_number(generator, "generation.internode.absolute", 0.15);
        if (!(internode > 0.0) || internode > 100.0) {
            return Diagnostic::error(
                ErrorCode::schema_violation,
                "phyllotaxy mode requires generation.internode.absolute in (0, 100] meters");
        }
        const double members_value = get_number(generator, "generation.members_per_whorl", 1.0);
        if (!(members_value >= 1.0) || members_value > 16.0) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "generation.members_per_whorl out of range [1, 16]");
        }
        const auto members = std::uint64_t(members_value);
        const double divergence = get_number(generator, "generation.divergence.degrees", 137.5);
        const double twist = get_number(generator, "generation.twist.degrees", 0.0);
        if (!(parent_length_m > 0.0)) {
            return placements; // degenerate parent: no whorls
        }
        const double step = internode / parent_length_m;
        std::uint64_t whorl = 0;
        for (double t = first; t <= last + 1e-12; t += step, ++whorl) {
            for (std::uint64_t member = 0; member < members; ++member) {
                const double azimuth = double(whorl) * (divergence + twist) +
                                       double(member) * 360.0 / double(members);
                placements.push_back(
                    Placement{whorl * members + member, std::min(t, 1.0), azimuth});
            }
            if (placements.size() > 4096) {
                return Diagnostic::error(ErrorCode::schema_violation,
                                         "phyllotaxy mode produced more than 4096 placements");
            }
        }
    } else if (mode == "proportional") {
        // 07: count follows parent length and density; ordinals stay stable
        // because they are evenly spaced indices, not spacing-derived.
        if (parent_is_root) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "proportional mode requires a parent with a spine");
        }
        const double density = get_number(generator, "generation.density.per_meter", 4.0);
        if (!(density > 0.0) || density > 1000.0) {
            return Diagnostic::error(
                ErrorCode::schema_violation,
                "proportional mode requires generation.density.per_meter in (0, 1000]");
        }
        const auto count = std::uint64_t(
            std::lround(std::max(0.0, parent_length_m * (last - first) * density)));
        if (count > 4096) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "proportional mode produced more than 4096 placements");
        }
        for (std::uint64_t i = 0; i < count; ++i) {
            const double t =
                count == 1 ? first
                           : first + (last - first) * double(i) / double(count - 1);
            placements.push_back(Placement{i, t, std::nullopt});
        }
    } else if (mode == "bifurcation") {
        // 07 "Bifurcation" subset: the spine splits at its tip into `count`
        // children spread evenly in azimuth; recursion comes from chaining
        // branch generators. Asymmetry/probability controls arrive later.
        if (parent_is_root) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "bifurcation mode requires a parent with a spine");
        }
        const double count_value = get_number(generator, "generation.count", 2.0);
        if (!(count_value >= 1.0) || count_value > 8.0) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "bifurcation mode requires generation.count in [1, 8]");
        }
        const auto count = std::uint64_t(count_value);
        const double phase = get_number(generator, "generation.phase.degrees", 0.0);
        for (std::uint64_t i = 0; i < count; ++i) {
            placements.push_back(
                Placement{i, 1.0, phase + double(i) * 360.0 / double(count)});
        }
    } else {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "unsupported generation.mode '" + mode +
                                     "' (supported: absolute, interval, phyllotaxy, "
                                     "proportional, bifurcation)");
    }
    return placements;
}

struct BranchBuildResult {
    EvaluatedNode node;
    BranchNodeGeometry geometry;
};

// Rodrigues rotation of v around unit axis by angle (radians).
Vec3 rotate_around(const Vec3& v, const Vec3& axis, double angle) {
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    return v * c + cross(axis, v) * s + axis * (dot(axis, v) * (1.0 - c));
}

double smoothstep01(double x) {
    x = std::clamp(x, 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

// Growth factor for a generator at `growth` lifecycle time (13 "Growth"):
// start defaults derive from generator depth, which realizes parent-relative
// timing — deeper structure emerges later, and children attach to the grown
// parent spine so attachments move with growth.
double growth_factor(const doc::GeneratorInstance& generator, std::uint32_t depth,
                     double growth) {
    const double default_start = std::min(0.75, 0.16 * double(depth > 0 ? depth - 1 : 0));
    const double start = std::clamp(get_number(generator, "growth.start", default_start), 0.0, 1.0);
    const double duration =
        std::clamp(get_number(generator, "growth.duration", 0.35), 0.01, 1.0);
    return smoothstep01((growth - start) / duration);
}

Result<BranchBuildResult> build_branch(const doc::Document& document,
                                       const doc::GeneratorInstance& generator,
                                       const EvaluatedNode& parent, bool parent_is_root,
                                       const Placement& placement,
                                       const EvaluationProfile& profile, std::uint32_t depth,
                                       double growth_scale,
                                       std::vector<Diagnostic>& warnings) {
    // Fronds (08_GENERATORS "Frond") share the branch spine walk — placement,
    // orientation, bend, wander, variance — and swap the swept tube for a
    // ribbon blade with a width profile, fold, twist, and serration.
    const bool is_frond = generator.type == "canopy.frond";
    double length = get_number(generator, "spine.length.absolute", 1.0);
    if (!(length > 0.0) || length > 1000.0) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "spine.length.absolute out of range (0, 1000] meters");
    }
    double base_radius = get_number(generator, "spine.radius.absolute", 0.05);
    if (!(base_radius > 0.0) || base_radius > 100.0) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "spine.radius.absolute out of range (0, 100] meters");
    }

    static const Curve kDefaultTaper = []() {
        auto curve = Curve::create({CurveKey{0.0, 1.0, 0.0, 0.0, {}},
                                    CurveKey{1.0, 0.0, 0.0, 0.0, {}}},
                                   CurveInterpolation::linear, CurveBoundary::clamp);
        return std::move(curve).value();
    }();
    auto radius_profile = get_curve(generator, "spine.radius.profile", kDefaultTaper);
    if (!radius_profile.ok()) {
        return radius_profile.take_error();
    }

    const std::string mode = get_string(generator, "generation.mode", "absolute");
    const SemanticId semantic_id = derive_semantic_id(
        document.manifest.seed, generator.id, parent.semantic_id, mode, placement.ordinal);

    // Positional scaling: child.length.profile is evaluated at the attachment
    // coordinate along the parent (e.g. shorter boughs near the crown).
    static const Curve kUnitCurve = Curve::constant_value(1.0);
    auto length_profile = get_curve(generator, "child.length.profile", kUnitCurve);
    if (!length_profile.ok()) {
        return length_profile.take_error();
    }
    if (!parent_is_root) {
        const double scale = length_profile.value().evaluate(placement.parent_t);
        if (!(scale > 0.0)) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "child.length.profile must stay positive");
        }
        length *= scale;
        // Radius follows length by default so children stay proportionate.
        base_radius *= scale;
    }

    // Deterministic per-node variance (ADR-0003 named streams; data model
    // "Variance"): a change to one property cannot reshuffle another stream.
    const double length_variance =
        std::clamp(get_number(generator, "spine.length.variance.relative", 0.0), 0.0, 0.9);
    if (length_variance > 0.0) {
        RandomStream stream(derive_stream_key(document.manifest.seed, generator.id, semantic_id,
                                              "spine.length.absolute", "variance"));
        length *= 1.0 + stream.uniform(-length_variance, length_variance);
    }
    const double radius_variance =
        std::clamp(get_number(generator, "spine.radius.variance.relative", 0.0), 0.0, 0.9);
    if (radius_variance > 0.0) {
        RandomStream stream(derive_stream_key(document.manifest.seed, generator.id, semantic_id,
                                              "spine.radius.absolute", "variance"));
        base_radius *= 1.0 + stream.uniform(-radius_variance, radius_variance);
    }

    // Growth development (13): length and radius follow the lifecycle factor.
    length *= growth_scale;
    base_radius *= std::max(growth_scale, 0.25); // radius develops faster than length
    length = std::max(length, 1e-3);

    // Attachment frame from the parent spine (08_GENERATORS.md "Attachment
    // frames"); the root supplies origin/+Y.
    Vec3 base_position{0.0, 0.0, 0.0};
    geo::Frame parent_frame;
    if (!parent_is_root) {
        base_position = parent.spline.position_at(placement.parent_t);
        const Vec3 tangent = parent.spline.tangent_at(placement.parent_t);
        parent_frame.tangent = tangent;
        parent_frame.normal = geo::stable_normal_for_tangent(tangent);
        parent_frame.binormal = cross(parent_frame.tangent, parent_frame.normal);
    }

    // Orientation: polar angle from the parent tangent plus an azimuth around
    // it. Azimuth defaults to a named random stream draw (ADR-0003) unless
    // fixed by property.
    Vec3 direction{0.0, 1.0, 0.0};
    if (!parent_is_root) {
        double angle_deg = get_number(generator, "generation.angle.degrees", 45.0);
        const double angle_variance =
            std::clamp(get_number(generator, "generation.angle.variance.degrees", 0.0), 0.0, 90.0);
        if (angle_variance > 0.0) {
            RandomStream stream(derive_stream_key(document.manifest.seed, generator.id,
                                                  semantic_id, "generation.angle.degrees",
                                                  "variance"));
            angle_deg += stream.uniform(-angle_variance, angle_variance);
        }
        // Azimuth priority: mode-driven placement (phyllotaxy/bifurcation) >
        // fixed property > named-stream random.
        double azimuth_deg;
        if (placement.azimuth_deg.has_value()) {
            azimuth_deg = *placement.azimuth_deg;
            const double azimuth_variance = std::clamp(
                get_number(generator, "generation.azimuth.variance.degrees", 0.0), 0.0, 180.0);
            if (azimuth_variance > 0.0) {
                RandomStream stream(derive_stream_key(document.manifest.seed, generator.id,
                                                      semantic_id,
                                                      "generation.azimuth.degrees", "variance"));
                azimuth_deg += stream.uniform(-azimuth_variance, azimuth_variance);
            }
        } else {
            azimuth_deg = get_number(generator, "generation.azimuth.degrees", -1.0);
            if (azimuth_deg < 0.0) {
                RandomStream stream(derive_stream_key(document.manifest.seed, generator.id,
                                                      semantic_id, "generation.azimuth.degrees",
                                                      "azimuth"));
                azimuth_deg = stream.uniform(0.0, 360.0);
            }
        }
        const double angle = angle_deg * std::numbers::pi / 180.0;
        const double azimuth = azimuth_deg * std::numbers::pi / 180.0;
        const Vec3 radial = parent_frame.normal * std::cos(azimuth) +
                            parent_frame.binormal * std::sin(azimuth);
        direction = normalize_or(parent_frame.tangent * std::cos(angle) +
                                     radial * std::sin(angle),
                                 Vec3{0.0, 1.0, 0.0});
    }

    // Control polyline with deterministic droop and wander. Bend rotates the
    // growth direction in its vertical plane per step (positive = droop);
    // wander adds small per-step jitter from a named stream (organic gnarl).
    const double bend_deg =
        std::clamp(get_number(generator, "spine.bend.degrees", 0.0), -170.0, 170.0);
    const double wander_deg =
        std::clamp(get_number(generator, "spine.wander.degrees", 0.0), 0.0, 45.0);
    constexpr std::size_t kControlPoints = 8;
    constexpr Vec3 kUp{0.0, 1.0, 0.0};
    const double step_length = length / double(kControlPoints - 1);
    const double step_bend = -bend_deg * std::numbers::pi / 180.0 / double(kControlPoints - 1);
    RandomStream wander_stream(derive_stream_key(document.manifest.seed, generator.id,
                                                 semantic_id, "spine.wander.degrees", "wander"));
    // Optional ground clamp (roots crawling on terrain; 09 acceptance "Root
    // flare intersecting ground"). Absolute world Y in meters.
    const bool ground_clamped = has_property(generator, "spine.ground.level");
    const double ground_level = get_number(generator, "spine.ground.level", 0.0);
    std::vector<Vec3> controls(kControlPoints);
    controls[0] = base_position;
    Vec3 growth = direction;
    for (std::size_t i = 1; i < kControlPoints; ++i) {
        if (step_bend != 0.0) {
            const Vec3 bend_axis =
                normalize_or(cross(growth, kUp), geo::stable_normal_for_tangent(growth));
            growth = normalize_or(rotate_around(growth, bend_axis, step_bend), growth);
        }
        if (wander_deg > 0.0) {
            const double wander = wander_deg * std::numbers::pi / 180.0;
            const Vec3 side = geo::stable_normal_for_tangent(growth);
            const Vec3 side2 = cross(growth, side);
            growth = normalize_or(
                rotate_around(rotate_around(growth, side, wander_stream.uniform(-wander, wander)),
                              side2, wander_stream.uniform(-wander, wander)),
                growth);
        }
        controls[i] = controls[i - 1] + growth * step_length;
        if (ground_clamped && controls[i].y < ground_level) {
            controls[i].y = ground_level;
            // Flatten the growth direction so the spine crawls rather than
            // repeatedly diving below the clamp plane.
            growth = normalize_or(Vec3{growth.x, 0.0, growth.z},
                                  geo::stable_normal_for_tangent(Vec3{0.0, 1.0, 0.0}));
        }
    }
    auto spline = geo::Spline::create(std::move(controls));
    if (!spline.ok()) {
        return spline.take_error();
    }

    // Frond blade controls.
    const double frond_width = get_number(generator, "frond.width.absolute", 0.3);
    const double serration_count =
        std::clamp(get_number(generator, "frond.serration.count", 0.0), 0.0, 64.0);
    const double serration_depth =
        std::clamp(get_number(generator, "frond.serration.depth", 0.6), 0.0, 1.0);
    if (is_frond && (!(frond_width > 0.0) || frond_width > 10.0)) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "frond.width.absolute out of range (0, 10] meters");
    }
    static const Curve kDefaultBlade = []() {
        auto curve = Curve::create({CurveKey{0.0, 0.2, 0.0, 0.0, {}},
                                    CurveKey{0.3, 1.0, 0.0, 0.0, {}},
                                    CurveKey{1.0, 0.05, 0.0, 0.0, {}}},
                                   CurveInterpolation::linear, CurveBoundary::clamp);
        return std::move(curve).value();
    }();
    auto width_profile = get_curve(generator, "frond.width.profile", kDefaultBlade);
    if (!width_profile.ok()) {
        return width_profile.take_error();
    }

    // Sample count from the profile (B-028: density changes with profile,
    // identity does not). Serrated fronds need enough samples to resolve the
    // leaflet pattern, capped to bound the cost.
    auto sample_count = std::clamp(
        std::uint32_t(std::lround(length * profile.length_samples_per_meter)),
        profile.min_length_samples, profile.max_length_samples);
    if (is_frond && serration_count > 0.0) {
        sample_count = std::max(sample_count,
                                std::min(std::uint32_t(serration_count) * 4u, 192u));
    }

    // Base flare (09 "Radius and flare"): a base-local radius contribution
    // that decays with arc length. Serves trunk/root flare and, on child
    // branches, the collar junction strategy's flared base.
    const double flare =
        std::clamp(get_number(generator, "spine.flare.relative", 0.0), 0.0, 3.0);
    const double flare_length =
        std::clamp(get_number(generator, "spine.flare.length.relative", 0.15), 0.01, 1.0);
    auto flare_decay = [flare_length](double t) {
        if (t >= flare_length) {
            return 0.0;
        }
        const double x = 1.0 - t / flare_length;
        return x * x;
    };

    std::vector<geo::SpineSample> samples(sample_count);
    std::vector<Vec3> positions;
    std::vector<Vec3> tangents;
    spline.value().sample_uniform(sample_count, positions, tangents);
    const double total_length = spline.value().total_length();
    for (std::uint32_t i = 0; i < sample_count; ++i) {
        const double t = double(i) / double(sample_count - 1);
        double half_extent;
        if (is_frond) {
            // Ribbon half-width: width profile with optional leaflet serration.
            double width_scale = std::max(width_profile.value().evaluate(t), 0.0);
            if (serration_count > 0.0) {
                const double wave =
                    0.5 + 0.5 * std::sin(2.0 * std::numbers::pi * serration_count * t);
                width_scale *= 1.0 - serration_depth * wave * wave;
            }
            half_extent = 0.5 * frond_width * width_scale;
        } else {
            const double radius_scale = std::max(radius_profile.value().evaluate(t), 0.0);
            const double flared = 1.0 + flare * flare_decay(t);
            half_extent = base_radius * radius_scale * flared;
        }
        samples[i] = geo::SpineSample{positions[i],  tangents[i],
                                      t * total_length, t,
                                      half_extent,     is_frond ? 0.0 : flare_decay(t)};
    }
    // Interior samples must keep a positive extent; only the tip may reach 0
    // (fronds tolerate zero-width waists at serration notches).
    for (std::uint32_t i = 0; i + 1 < sample_count; ++i) {
        if (samples[i].radius <= 0.0) {
            samples[i].radius = 1e-4;
            if (!is_frond) {
                warnings.push_back(Diagnostic::warning(
                    ErrorCode::geometry_invalid,
                    "radius profile reached zero before the tip on node " + semantic_id.str() +
                        "; clamped to 0.1 mm"));
            }
        }
    }

    const Vec3 initial_normal =
        parent_is_root ? geo::stable_normal_for_tangent(tangents.front()) : parent_frame.normal;
    auto frames = geo::parallel_transport(positions, tangents, initial_normal);
    if (!frames.ok()) {
        return frames.take_error();
    }

    Result<geo::TriangleMesh> mesh =
        Diagnostic::error(ErrorCode::internal_error, "mesh not built");
    if (is_frond) {
        geo::RibbonOptions ribbon;
        ribbon.fold_radians =
            std::clamp(get_number(generator, "frond.fold.degrees", 30.0), 0.0, 80.0) *
            std::numbers::pi / 180.0;
        ribbon.twist_radians =
            std::clamp(get_number(generator, "frond.twist.degrees", 0.0), -720.0, 720.0) *
            std::numbers::pi / 180.0;
        ribbon.uv_tile_length_m = get_number(generator, "mesh.uv_tile_length.absolute", 1.0);
        if (!(ribbon.uv_tile_length_m > 0.0)) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "mesh.uv_tile_length.absolute must be positive");
        }
        mesh = geo::sweep_ribbon(samples, frames.value(), ribbon);
    } else {
    geo::SweepOptions sweep_options;
    const double radial_segments = get_number(generator, "mesh.radial_segments", 8.0);
    if (!(radial_segments >= 3.0) || radial_segments > 256.0) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "mesh.radial_segments out of range [3, 256]");
    }
    sweep_options.radial_segments =
        std::min(std::uint32_t(radial_segments), profile.radial_segments_cap);
    sweep_options.uv_tile_length_m = get_number(generator, "mesh.uv_tile_length.absolute", 2.0);
    if (!(sweep_options.uv_tile_length_m > 0.0)) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "mesh.uv_tile_length.absolute must be positive");
    }
    // Buttress lobes on the flare region.
    const double lobe_count_value = get_number(generator, "spine.flare.lobes", 0.0);
    if (lobe_count_value < 0.0 || lobe_count_value > 16.0) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "spine.flare.lobes out of range [0, 16]");
    }
    sweep_options.lobe_count = std::uint32_t(lobe_count_value);
    sweep_options.lobe_amplitude =
        std::clamp(get_number(generator, "spine.flare.lobe.relative", 0.15), 0.0, 0.5);
    if (sweep_options.lobe_count > 0) {
        const double phase_deg = get_number(generator, "spine.flare.phase.degrees", -1.0);
        if (phase_deg >= 0.0) {
            sweep_options.lobe_phase = phase_deg * std::numbers::pi / 180.0;
        } else {
            RandomStream stream(derive_stream_key(document.manifest.seed, generator.id,
                                                  semantic_id, "spine.flare.phase.degrees",
                                                  "flare-phase"));
            sweep_options.lobe_phase = stream.uniform(0.0, 2.0 * std::numbers::pi);
        }
    }
    // Per-node bark UV phase (09 "Bark UVs": per-node random phase uses named
    // streams). Off by default so plain documents keep byte-stable output.
    if (get_bool(generator, "mesh.uv.random_phase", false)) {
        RandomStream stream(derive_stream_key(document.manifest.seed, generator.id, semantic_id,
                                              "mesh.uv.random_phase", "uv-phase"));
        sweep_options.uv_v_offset = stream.uniform(0.0, 1.0);
    }

    mesh = geo::sweep_branch(samples, frames.value(), sweep_options);
    }
    if (!mesh.ok()) {
        return mesh.take_error();
    }

    // Material resolution: content warning, not failure (07: failure
    // containment; missing material is a diagnostic).
    Uuid material_id;
    const std::string_view material_key = is_frond ? "material.frond" : "material.bark";
    const std::string material_text = get_string(generator, material_key, "");
    if (!material_text.empty()) {
        const auto parsed = Uuid::parse(material_text);
        if (!parsed || document.find_material(*parsed) == nullptr) {
            warnings.push_back(Diagnostic::warning(ErrorCode::not_found,
                                                   std::string(material_key) + " '" +
                                                       material_text +
                                                       "' not found for generator " +
                                                       generator.id.str()));
        } else {
            material_id = *parsed;
        }
    }

    BranchBuildResult result;
    result.node = EvaluatedNode{semantic_id,
                                std::move(spline).value(),
                                {},
                                length,
                                base_radius,
                                radius_profile.value(),
                                geo::Frame{direction, initial_normal, cross(direction, initial_normal)},
                                depth};
    result.geometry = BranchNodeGeometry{semantic_id,
                                         parent.semantic_id,
                                         generator.id,
                                         material_id,
                                         length,
                                         base_radius,
                                         std::move(mesh).value(),
                                         base_position,
                                         is_frond ? NodeKind::frond : NodeKind::branch,
                                         depth,
                                         ground_clamped};
    return result;
}

// Resolved leaf silhouette: a triangulated material cutout (ADR-0004) or the
// default folded card when the material has no outline.
struct LeafShape {
    bool is_cutout = false;
    // Textured card (ADR-0009): a flat quad whose UVs span the material's
    // atlas region; the silhouette comes from the texture's alpha mask.
    bool is_region_card = false;
    std::array<double, 4> region{0.0, 0.0, 1.0, 1.0};
    std::vector<Vec2> outline;               // normalized: x −0.5..0.5, y 0..1
    std::vector<std::uint32_t> triangles;
    std::array<double, 2> stem{0.0, 0.0};
};

void append_leaf(geo::TriangleMesh& mesh, const LeafShape& shape, const Vec3& stem,
                 const Vec3& direction, const Vec3& side, const Vec3& fallback_normal,
                 double length, double width, double fold) {
    const auto base = std::uint32_t(mesh.positions.size());
    if (shape.is_region_card) {
        // Flat textured quad; silhouette from the alpha mask, so 2 triangles
        // regardless of leaf complexity (the game-budget path).
        const double half_width = 0.5 * width;
        const Vec3 flat_normal = normalize_or(cross(side, direction), fallback_normal);
        const Vec3 bottom_left = stem - side * half_width;
        const Vec3 bottom_right = stem + side * half_width;
        const Vec3 top_left = bottom_left + direction * length;
        const Vec3 top_right = bottom_right + direction * length;
        mesh.positions.insert(mesh.positions.end(),
                              {bottom_left, bottom_right, top_right, top_left});
        mesh.normals.insert(mesh.normals.end(), 4, flat_normal);
        const auto& r = shape.region;
        mesh.uvs.insert(mesh.uvs.end(), {Vec2{r[0], r[1]}, Vec2{r[2], r[1]},
                                         Vec2{r[2], r[3]}, Vec2{r[0], r[3]}});
        mesh.indices.insert(mesh.indices.end(),
                            {base, base + 1, base + 2, base, base + 2, base + 3});
        return;
    }
    if (!shape.is_cutout) {
        // Default card: diamond with a midrib fold (unchanged from the
        // batched-leaf bootstrap so existing documents keep their output).
        const double half_width = 0.5 * width;
        const Vec3 tip = stem + direction * length;
        const Vec3 mid = stem + direction * (length * 0.45);
        const Vec3 left = mid + rotate_around(side, direction, fold) * half_width;
        const Vec3 right = mid - rotate_around(side, direction, -fold) * half_width;
        const Vec3 face_normal = normalize_or(cross(tip - stem, right - left), fallback_normal);
        mesh.positions.insert(mesh.positions.end(), {stem, left, tip, right});
        mesh.normals.insert(mesh.normals.end(), 4, face_normal);
        mesh.uvs.insert(mesh.uvs.end(),
                        {Vec2{0.5, 0.0}, Vec2{0.0, 0.5}, Vec2{0.5, 1.0}, Vec2{1.0, 0.5}});
        mesh.indices.insert(mesh.indices.end(),
                            {base, base + 1, base + 2, base, base + 2, base + 3});
        return;
    }
    const Vec3 flat_normal = normalize_or(cross(side, direction), fallback_normal);
    for (const Vec2& vertex : shape.outline) {
        const double x = vertex.x - shape.stem[0];
        const double y = vertex.y - shape.stem[1];
        // Fold rotates each half of the blade around the midrib.
        const double half_fold = x >= 0.0 ? fold : -fold;
        const Vec3 folded_side = rotate_around(side, direction, half_fold);
        mesh.positions.push_back(stem + direction * (y * length) + folded_side * (x * width));
        mesh.normals.push_back(
            normalize_or(rotate_around(flat_normal, direction, half_fold), flat_normal));
        mesh.uvs.push_back(Vec2{vertex.x + 0.5, vertex.y});
    }
    for (const std::uint32_t index : shape.triangles) {
        mesh.indices.push_back(base + index);
    }
}

// Batched Leaf and Leaf Mesh (10_FOLIAGE_VINES_AND_DETAILS.md), bootstrap
// subset: deterministic placement along the parent spine. Batched mode emits
// one structure-of-arrays batch mesh per parent node; Leaf Mesh emits one
// semantic node per leaf (individually addressable, editable in later
// epics). Per-leaf random decisions each use their own derived stream so leaf
// N is unaffected by changes to leaf M or by adding later leaves.
Result<void> build_leaves(const doc::Document& document,
                          const doc::GeneratorInstance& generator, const EvaluatedNode& parent,
                          bool batched, std::uint32_t depth, double growth_scale, double season,
                          std::vector<BranchNodeGeometry>& out_nodes,
                          std::vector<Diagnostic>& warnings) {
    const double spacing = get_number(generator, "generation.spacing.relative", 0.1);
    if (!(spacing > 0.0) || spacing > 1.0) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "batched leaf requires generation.spacing.relative in (0, 1]");
    }
    const double first = std::clamp(get_number(generator, "generation.first", 0.15), 0.0, 1.0);
    const double last = std::clamp(get_number(generator, "generation.last", 1.0), 0.0, 1.0);
    const double per_point_value = get_number(generator, "generation.leaves_per_point", 2.0);
    if (!(per_point_value >= 1.0) || per_point_value > 8.0) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "generation.leaves_per_point out of range [1, 8]");
    }
    const auto per_point = std::uint64_t(per_point_value);
    const double leaf_length = get_number(generator, "leaf.length.absolute", 0.08);
    if (!(leaf_length > 0.0) || leaf_length > 5.0) {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "leaf.length.absolute out of range (0, 5] meters");
    }
    const double width_ratio =
        std::clamp(get_number(generator, "leaf.width.relative", 0.6), 0.05, 2.0);
    const double size_variance =
        std::clamp(get_number(generator, "leaf.size.variance.relative", 0.25), 0.0, 0.9);
    const double pitch_deg = get_number(generator, "leaf.pitch.degrees", 40.0);
    const double pitch_variance =
        std::clamp(get_number(generator, "leaf.pitch.variance.degrees", 15.0), 0.0, 90.0);
    const double droop =
        std::clamp(get_number(generator, "leaf.droop.relative", 0.2), 0.0, 1.0);
    const double fold_deg = std::clamp(get_number(generator, "leaf.fold.degrees", 18.0), 0.0, 80.0);

    // Material first: its cutout outline decides the leaf silhouette.
    Uuid material_id;
    const doc::Material* material = nullptr;
    const std::string material_text = get_string(generator, "material.leaf", "");
    if (!material_text.empty()) {
        const auto parsed = Uuid::parse(material_text);
        material = parsed ? document.find_material(*parsed) : nullptr;
        if (material == nullptr) {
            warnings.push_back(Diagnostic::warning(ErrorCode::not_found,
                                                   "material.leaf '" + material_text +
                                                       "' not found for generator " +
                                                       generator.id.str()));
        } else {
            material_id = *parsed;
        }
    }
    LeafShape shape;
    if (material != nullptr && material->card_region.has_value()) {
        shape.is_region_card = true;
        shape.region = *material->card_region;
    } else if (material != nullptr && material->cutout.has_value()) {
        shape.is_cutout = true;
        shape.stem = material->cutout->stem;
        for (const auto& vertex : material->cutout->vertices) {
            shape.outline.push_back(Vec2{vertex[0], vertex[1]});
        }
        auto triangles = geo::triangulate_polygon(shape.outline);
        if (!triangles.ok()) {
            Diagnostic error = Diagnostic::error(
                ErrorCode::schema_violation,
                "material '" + material->name + "': cutout outline cannot be triangulated");
            error.with_note(triangles.take_error());
            return error;
        }
        shape.triangles = std::move(triangles).value();
    }

    const std::string mode = batched ? "batched_leaf" : "leaf_mesh";
    // Batched: one node (ordinal 0) per parent node; Leaf Mesh: one node per
    // leaf, each with its own semantic ordinal.
    const SemanticId batch_id = derive_semantic_id(document.manifest.seed, generator.id,
                                                   parent.semantic_id, mode, 0);

    // Seasonal leaf drop (13 "Seasons"): each leaf owns a deterministic drop
    // value; as season passes the drop window the canopy thins mottled and
    // monotonically — the same leaf never re-attaches at a later season.
    const double drop_start =
        std::clamp(get_number(generator, "season.drop.start", 0.7), 0.0, 1.0);
    const double drop_fraction =
        season <= drop_start ? 0.0
                             : std::clamp((season - drop_start) / std::max(1.0 - drop_start, 1e-6),
                                          0.0, 1.0);

    geo::TriangleMesh batch_mesh;
    const double fold = fold_deg * std::numbers::pi / 180.0;
    std::uint64_t ordinal = 0;
    for (double t = first; t <= last + 1e-12; t += spacing) {
        const double tc = std::min(t, 1.0);
        const Vec3 point = parent.spline.position_at(tc);
        const Vec3 tangent = parent.spline.tangent_at(tc);
        const Vec3 normal = geo::stable_normal_for_tangent(tangent);
        const Vec3 binormal = cross(tangent, normal);
        const double parent_radius =
            parent.base_radius_m * std::max(parent.radius_profile.evaluate(tc), 0.0);
        for (std::uint64_t k = 0; k < per_point; ++k, ++ordinal) {
            const SemanticId leaf_id =
                batched ? batch_id
                        : derive_semantic_id(document.manifest.seed, generator.id,
                                             parent.semantic_id, mode, ordinal);
            RandomStream stream(derive_stream_key(document.manifest.seed, generator.id, leaf_id,
                                                  "leaf",
                                                  batched ? "leaf:" + std::to_string(ordinal)
                                                          : std::string("leaf")));
            const double azimuth = stream.uniform(0.0, 2.0 * std::numbers::pi);
            const double pitch =
                (pitch_deg + stream.uniform(-pitch_variance, pitch_variance)) *
                std::numbers::pi / 180.0;
            const double scale = 1.0 + stream.uniform(-size_variance, size_variance);
            const double roll = stream.uniform(0.0, 2.0 * std::numbers::pi);
            if (drop_fraction > 0.0) {
                RandomStream drop_stream(derive_stream_key(document.manifest.seed, generator.id,
                                                           leaf_id, "season.drop",
                                                           "drop:" + std::to_string(ordinal)));
                if (drop_stream.next_double() < drop_fraction) {
                    continue; // dropped (ground-debris batches are later work)
                }
            }

            const Vec3 radial = normal * std::cos(azimuth) + binormal * std::sin(azimuth);
            Vec3 direction = normalize_or(tangent * std::cos(pitch) + radial * std::sin(pitch),
                                          radial);
            // Gravity droop blends the leaf direction toward world down.
            direction = normalize_or(direction * (1.0 - droop) + Vec3{0.0, -1.0, 0.0} * droop,
                                     direction);

            const double len = leaf_length * scale * std::max(growth_scale, 0.05);
            const Vec3 stem = point + radial * parent_radius;
            Vec3 side = normalize_or(cross(direction, radial), binormal);
            // Roll spins the leaf blade around its midrib.
            side = normalize_or(rotate_around(side, direction, roll), side);

            if (batched) {
                append_leaf(batch_mesh, shape, stem, direction, side, radial, len,
                            width_ratio * len, fold);
            } else {
                geo::TriangleMesh leaf_mesh;
                append_leaf(leaf_mesh, shape, stem, direction, side, radial, len,
                            width_ratio * len, fold);
                if (auto valid = geo::validate_mesh(leaf_mesh); !valid.ok()) {
                    return valid.take_error();
                }
                out_nodes.push_back(BranchNodeGeometry{leaf_id, parent.semantic_id,
                                                       generator.id, material_id, len, 0.0,
                                                       std::move(leaf_mesh), stem,
                                                       NodeKind::foliage, depth, false});
            }
        }
        if (ordinal > 100000) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "leaf generator produced more than 100000 leaves per node");
        }
    }

    if (batched) {
        if (auto valid = geo::validate_mesh(batch_mesh); !valid.ok()) {
            return valid.take_error();
        }
        out_nodes.push_back(BranchNodeGeometry{batch_id, parent.semantic_id, generator.id,
                                               material_id, 0.0, 0.0, std::move(batch_mesh),
                                               parent.spline.position_at(0.0),
                                               NodeKind::foliage, depth, false});
    }
    return Ok{};
}

// --- wind ------------------------------------------------------------------

struct Mat3 {
    // Row-major.
    std::array<double, 9> m{1, 0, 0, 0, 1, 0, 0, 0, 1};

    static Mat3 rotation(const Vec3& axis, double angle) {
        const double c = std::cos(angle);
        const double s = std::sin(angle);
        const double t = 1.0 - c;
        const Vec3 a = axis;
        return Mat3{{c + a.x * a.x * t, a.x * a.y * t - a.z * s, a.x * a.z * t + a.y * s,
                     a.y * a.x * t + a.z * s, c + a.y * a.y * t, a.y * a.z * t - a.x * s,
                     a.z * a.x * t - a.y * s, a.z * a.y * t + a.x * s, c + a.z * a.z * t}};
    }
    Vec3 apply(const Vec3& v) const {
        return {m[0] * v.x + m[1] * v.y + m[2] * v.z, m[3] * v.x + m[4] * v.y + m[5] * v.z,
                m[6] * v.x + m[7] * v.y + m[8] * v.z};
    }
    Mat3 operator*(const Mat3& o) const {
        Mat3 r;
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                r.m[std::size_t(row * 3 + col)] =
                    m[std::size_t(row * 3)] * o.m[std::size_t(col)] +
                    m[std::size_t(row * 3 + 1)] * o.m[std::size_t(3 + col)] +
                    m[std::size_t(row * 3 + 2)] * o.m[std::size_t(6 + col)];
            }
        }
        return r;
    }
};

// Affine transform T(p) = R p + t.
struct WindTransform {
    Mat3 rotation;
    Vec3 translation{0.0, 0.0, 0.0};

    Vec3 apply_point(const Vec3& p) const { return rotation.apply(p) + translation; }
};

// Authoring wind preview (13 "VFX wind" / ADR-0006, falloff amendment):
// deterministic per-node oscillators applied as a bend about each node's
// base with a quadratic stiffness falloff — zero displacement at the base,
// full tip angle at the end. Ground-clamped nodes (roots) never sway, and
// their attachment near the parent base inherits ~zero motion through the
// same falloff. Foliage vertices ride their parent branch's bend directly
// (per vertex), which is also exactly what the exported wind vertex
// channels tell shaders to do.
struct WindSway {
    double main_angle = 0.0;    // tip angle about the bend axis
    double lateral_angle = 0.0; // tip angle about the wind direction
};

struct WindRecord {
    WindTransform inherited; // rigid transform of this node's base region
    Vec3 base;               // pre-wind base position
    double length = 1.0;
    WindSway sway;
};

double wind_falloff(double t) {
    t = std::clamp(t, 0.0, 1.0);
    return t * t;
}

void apply_wind(EvaluatedModel& model, const doc::Document& document,
                const TimelineSample& sample) {
    if (sample.wind_strength <= 0.0) {
        return;
    }
    const double strength = std::clamp(sample.wind_strength, 0.0, 1.0);
    const double direction = sample.wind_direction_deg * std::numbers::pi / 180.0;
    const Vec3 wind_dir{std::sin(direction), 0.0, std::cos(direction)};
    const Vec3 bend_axis = normalize_or(cross(Vec3{0.0, 1.0, 0.0}, wind_dir),
                                        Vec3{1.0, 0.0, 0.0});
    const double t = sample.time_s;
    auto sway_rotation = [&](const WindSway& sway, double falloff) {
        return Mat3::rotation(bend_axis, sway.main_angle * falloff) *
               Mat3::rotation(wind_dir, sway.lateral_angle * falloff);
    };

    // Parent-first order: depth ascending, semantic id as tiebreak (model
    // nodes are already semantic-sorted, so a stable sort keeps determinism).
    std::vector<std::size_t> order(model.nodes.size());
    for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = i;
    }
    std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return model.nodes[a].depth < model.nodes[b].depth;
    });

    std::map<SemanticId, WindRecord> records;
    for (const std::size_t index : order) {
        BranchNodeGeometry& node = model.nodes[index];
        const WindRecord* parent = nullptr;
        if (const auto it = records.find(node.parent_semantic_id); it != records.end()) {
            parent = &it->second;
        }

        // Rigid transform inherited by this node's base region: the parent's
        // inherited transform composed with the parent's bend evaluated at
        // this node's attachment point (falloff of the attachment distance).
        WindTransform inherited;
        if (parent != nullptr) {
            const double attachment =
                length(node.base_position - parent->base) / std::max(parent->length, 1e-6);
            const Mat3 parent_bend = sway_rotation(parent->sway, wind_falloff(attachment));
            inherited.rotation = parent->inherited.rotation * parent_bend;
            const Vec3 base_world = parent->inherited.apply_point(
                parent->base + parent_bend.apply(node.base_position - parent->base));
            inherited.translation = base_world - inherited.rotation.apply(node.base_position);
        }

        if (node.kind == NodeKind::foliage) {
            // Leaves ride the parent branch's bend per vertex, then ripple.
            const double ripple = 0.010 * strength;
            for (std::size_t v = 0; v < node.mesh.positions.size(); ++v) {
                Vec3& position = node.mesh.positions[v];
                Vec3 bent = position;
                if (parent != nullptr) {
                    const double along =
                        length(position - parent->base) / std::max(parent->length, 1e-6);
                    const Mat3 bend = sway_rotation(parent->sway, wind_falloff(along));
                    bent = parent->inherited.apply_point(
                        parent->base + bend.apply(position - parent->base));
                    node.mesh.normals[v] =
                        (parent->inherited.rotation * bend).apply(node.mesh.normals[v]);
                }
                const double vertex_phase = bent.x * 5.1 + bent.y * 3.7 + bent.z * 4.3;
                bent.y += ripple * std::sin(6.1 * t + vertex_phase);
                position = bent;
            }
            continue;
        }

        WindSway sway;
        if (!node.anchored) {
            RandomStream stream(derive_stream_key(document.manifest.seed, node.generator_id,
                                                  node.semantic_id, "wind", "phase"));
            const double phase = stream.uniform(0.0, 2.0 * std::numbers::pi);
            const double kind_gain = node.kind == NodeKind::frond ? 2.2 : 1.0;
            const double amplitude =
                strength * kind_gain * (0.015 + 0.02 * double(node.depth));
            const double oscillation =
                0.7 * std::sin(1.7 * t + phase) + 0.3 * std::sin(3.9 * t + 2.7 * phase);
            const double gust_wave = std::sin(0.53 * t + 0.5 * phase);
            const double gust_envelope =
                1.0 + sample.gust * std::max(0.0, gust_wave) * gust_wave * gust_wave;
            sway.main_angle = amplitude * gust_envelope * (0.9 + 0.55 * oscillation);
            sway.lateral_angle = amplitude * 0.4 * std::sin(1.3 * t + 1.9 * phase);
        }

        for (std::size_t v = 0; v < node.mesh.positions.size(); ++v) {
            Vec3& position = node.mesh.positions[v];
            const double along =
                length(position - node.base_position) / std::max(node.length_m, 1e-6);
            const Mat3 bend = sway_rotation(sway, wind_falloff(along));
            position = inherited.apply_point(node.base_position +
                                             bend.apply(position - node.base_position));
            node.mesh.normals[v] = (inherited.rotation * bend).apply(node.mesh.normals[v]);
        }

        records.emplace(node.semantic_id,
                        WindRecord{inherited, node.base_position, std::max(node.length_m, 1e-6),
                                   sway});
    }
}

} // namespace

Result<EvaluatedModel> evaluate(const doc::Document& document, const EvaluationProfile& profile,
                                const TimelineSample& sample) {
    auto order = document.topological_order();
    if (!order.ok()) {
        return order.take_error();
    }

    EvaluatedModel model;
    model.sample = sample;
    const double growth = std::clamp(sample.growth, 0.0, 1.0);
    const double season = std::clamp(sample.season, 0.0, 1.0);

    // Generator depth = graph distance from the root (parent-relative growth
    // timing and wind response both key on it).
    std::map<Uuid, std::uint32_t> generator_depth;

    // Root pseudo-node: the Tree generator creates no geometry (08_GENERATORS
    // "Tree") but anchors semantic identity and attachment for its children.
    const doc::GeneratorInstance* root = order.value().front();
    EvaluatedNode root_node;
    root_node.semantic_id =
        derive_semantic_id(document.manifest.seed, root->id, SemanticId{0}, "root", 0);
    {
        auto spline = geo::Spline::create({Vec3{0.0, 0.0, 0.0}});
        root_node.spline = std::move(spline).value();
    }

    // Evaluated parent nodes per generator, in deterministic creation order.
    std::map<Uuid, std::vector<EvaluatedNode>> nodes_by_generator;
    nodes_by_generator.emplace(root->id, std::vector<EvaluatedNode>{std::move(root_node)});

    for (const doc::GeneratorInstance* generator : order.value()) {
        if (generator == root || !generator->enabled) {
            continue;
        }
        const bool parent_is_root = generator->parent == root->id;
        const auto parent_nodes = nodes_by_generator.find(generator->parent);
        if (parent_nodes == nodes_by_generator.end()) {
            continue; // parent generator disabled or produced no nodes
        }
        const std::uint32_t depth =
            parent_is_root ? 1 : generator_depth[generator->parent] + 1;
        generator_depth[generator->id] = depth;
        const double node_growth = growth_factor(*generator, depth, growth);
        if (node_growth <= 0.0) {
            continue; // not yet emerged at this lifecycle time (13 "Growth")
        }
        if (generator->type == "canopy.batched_leaf" || generator->type == "canopy.leaf_mesh") {
            const bool batched = generator->type == "canopy.batched_leaf";
            for (const EvaluatedNode& parent : parent_nodes->second) {
                auto built =
                    build_leaves(document, *generator, parent, batched, depth, node_growth,
                                 season, model.nodes, model.warnings);
                if (!built.ok()) {
                    Diagnostic error =
                        Diagnostic::error(ErrorCode::evaluation_failure,
                                          "generator '" + generator->name + "' (" +
                                              generator->id.str() + ") failed");
                    error.with_note(built.take_error());
                    return error;
                }
            }
            continue; // leaf nodes never parent other generators
        }
        std::vector<EvaluatedNode> produced;
        for (const EvaluatedNode& parent : parent_nodes->second) {
            // Placements depend on the specific parent node (proportional and
            // phyllotaxy counts scale with its actual length).
            auto placements = make_placements(*generator, parent_is_root, parent.length_m);
            if (!placements.ok()) {
                Diagnostic error = Diagnostic::error(ErrorCode::evaluation_failure,
                                                     "generator '" + generator->name + "' (" +
                                                         generator->id.str() + ") failed");
                error.with_note(placements.take_error());
                return error;
            }
            for (const Placement& placement : placements.value()) {
                auto built = build_branch(document, *generator, parent, parent_is_root,
                                          placement, profile, depth, node_growth,
                                          model.warnings);
                if (!built.ok()) {
                    Diagnostic error =
                        Diagnostic::error(ErrorCode::evaluation_failure,
                                          "generator '" + generator->name + "' (" +
                                              generator->id.str() + ") failed");
                    error.with_note(built.take_error());
                    return error;
                }
                produced.push_back(std::move(built.value().node));
                model.nodes.push_back(std::move(built.value().geometry));
            }
        }
        nodes_by_generator.emplace(generator->id, std::move(produced));
    }

    std::sort(model.nodes.begin(), model.nodes.end(),
              [](const BranchNodeGeometry& a, const BranchNodeGeometry& b) {
                  return a.semantic_id < b.semantic_id;
              });
    apply_wind(model, document, sample);
    return model;
}

} // namespace canopy::eval
