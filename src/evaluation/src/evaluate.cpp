#include "canopy/evaluation/evaluate.hpp"

#include "canopy/foundation/curve.hpp"
#include "canopy/foundation/random.hpp"
#include "canopy/geometry/frames.hpp"
#include "canopy/geometry/spline.hpp"

#include <algorithm>
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
};

struct Placement {
    std::uint64_t ordinal = 0;
    double parent_t = 0.0; // normalized position along the parent spine
};

// Candidate placements for one generator under one parent node (deterministic;
// 07_EVALUATION_ENGINE.md interval/absolute subset).
Result<std::vector<Placement>> make_placements(const doc::GeneratorInstance& generator,
                                               bool parent_is_root) {
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
            placements.push_back(Placement{i, t});
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
            placements.push_back(Placement{ordinal++, std::min(t, 1.0)});
            if (placements.size() > 4096) {
                return Diagnostic::error(ErrorCode::schema_violation,
                                         "interval mode produced more than 4096 placements");
            }
        }
    } else {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "unsupported generation.mode '" + mode +
                                     "' (bootstrap supports: absolute, interval)");
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

Result<BranchBuildResult> build_branch(const doc::Document& document,
                                       const doc::GeneratorInstance& generator,
                                       const EvaluatedNode& parent, bool parent_is_root,
                                       const Placement& placement,
                                       const EvaluationProfile& profile,
                                       std::vector<Diagnostic>& warnings) {
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
        double azimuth_deg = get_number(generator, "generation.azimuth.degrees", -1.0);
        if (azimuth_deg < 0.0) {
            RandomStream stream(derive_stream_key(document.manifest.seed, generator.id,
                                                  semantic_id, "generation.azimuth.degrees",
                                                  "azimuth"));
            azimuth_deg = stream.uniform(0.0, 360.0);
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
    }
    auto spline = geo::Spline::create(std::move(controls));
    if (!spline.ok()) {
        return spline.take_error();
    }

    // Sample count from the profile (B-028: density changes with profile,
    // identity does not).
    const auto sample_count = std::clamp(
        std::uint32_t(std::lround(length * profile.length_samples_per_meter)),
        profile.min_length_samples, profile.max_length_samples);

    std::vector<geo::SpineSample> samples(sample_count);
    std::vector<Vec3> positions(sample_count);
    std::vector<Vec3> tangents(sample_count);
    const double total_length = spline.value().total_length();
    for (std::uint32_t i = 0; i < sample_count; ++i) {
        const double t = double(i) / double(sample_count - 1);
        positions[i] = spline.value().position_at(t);
        tangents[i] = spline.value().tangent_at(t);
        const double radius_scale = std::max(radius_profile.value().evaluate(t), 0.0);
        samples[i] = geo::SpineSample{positions[i], tangents[i], t * total_length, t,
                                      base_radius * radius_scale};
    }
    // Interior samples must keep positive radius; only the tip may reach 0.
    for (std::uint32_t i = 0; i + 1 < sample_count; ++i) {
        if (samples[i].radius <= 0.0) {
            samples[i].radius = 1e-4;
            warnings.push_back(Diagnostic::warning(
                ErrorCode::geometry_invalid,
                "radius profile reached zero before the tip on node " + semantic_id.str() +
                    "; clamped to 0.1 mm"));
        }
    }

    const Vec3 initial_normal =
        parent_is_root ? geo::stable_normal_for_tangent(tangents.front()) : parent_frame.normal;
    auto frames = geo::parallel_transport(positions, tangents, initial_normal);
    if (!frames.ok()) {
        return frames.take_error();
    }

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

    auto mesh = geo::sweep_branch(samples, frames.value(), sweep_options);
    if (!mesh.ok()) {
        return mesh.take_error();
    }

    // Material resolution: content warning, not failure (07: failure
    // containment; missing material is a diagnostic).
    Uuid material_id;
    const std::string material_text = get_string(generator, "material.bark", "");
    if (!material_text.empty()) {
        const auto parsed = Uuid::parse(material_text);
        if (!parsed || document.find_material(*parsed) == nullptr) {
            warnings.push_back(Diagnostic::warning(ErrorCode::not_found,
                                                   "material.bark '" + material_text +
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
                                geo::Frame{direction, initial_normal, cross(direction, initial_normal)}};
    result.geometry = BranchNodeGeometry{semantic_id,     parent.semantic_id, generator.id,
                                         material_id,     length,            base_radius,
                                         std::move(mesh).value()};
    return result;
}

} // namespace

Result<EvaluatedModel> evaluate(const doc::Document& document, const EvaluationProfile& profile) {
    auto order = document.topological_order();
    if (!order.ok()) {
        return order.take_error();
    }

    EvaluatedModel model;

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
        auto placements = make_placements(*generator, parent_is_root);
        if (!placements.ok()) {
            Diagnostic error = Diagnostic::error(ErrorCode::evaluation_failure,
                                                 "generator '" + generator->name + "' (" +
                                                     generator->id.str() + ") failed");
            error.with_note(placements.take_error());
            return error;
        }
        std::vector<EvaluatedNode> produced;
        for (const EvaluatedNode& parent : parent_nodes->second) {
            for (const Placement& placement : placements.value()) {
                auto built = build_branch(document, *generator, parent, parent_is_root,
                                          placement, profile, model.warnings);
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
    return model;
}

} // namespace canopy::eval
