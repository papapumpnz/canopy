#include "canopy/evaluation/evaluate.hpp"
#include "canopy_test.hpp"

#include <set>

using namespace canopy;
using namespace canopy::doc;
using namespace canopy::eval;

namespace {

Uuid uuid(char digit) {
    std::string text = "00000000-0000-4000-8000-000000000000";
    text[35] = digit;
    return *Uuid::parse(text);
}

Document trunk_document() {
    Document document;
    document.manifest.document_id = uuid('a');
    document.manifest.name = "MinimalTrunk";
    document.manifest.seed = 182736451;

    GeneratorInstance tree;
    tree.id = uuid('1');
    tree.type = "canopy.tree";
    tree.name = "Tree";
    document.generators.push_back(tree);

    Material bark;
    bark.id = uuid('b');
    bark.name = "bark_default";
    document.materials.push_back(bark);

    GeneratorInstance trunk;
    trunk.id = uuid('2');
    trunk.type = "canopy.branch";
    trunk.name = "Trunk";
    trunk.parent = uuid('1');
    trunk.properties.emplace("generation.mode", json::Value("absolute"));
    trunk.properties.emplace("generation.count", json::Value(1));
    trunk.properties.emplace("spine.length.absolute", json::Value(4.0));
    trunk.properties.emplace("spine.radius.absolute", json::Value(0.25));
    trunk.properties.emplace("mesh.radial_segments", json::Value(8));
    trunk.properties.emplace("material.bark", json::Value(uuid('b').str()));
    {
        json::Object curve;
        curve.emplace("interpolation", "linear");
        json::Array keys;
        keys.push_back(json::Array{json::Value(0.0), json::Value(1.0)});
        keys.push_back(json::Array{json::Value(1.0), json::Value(0.15)});
        curve.emplace("keys", std::move(keys));
        trunk.properties.emplace("spine.radius.profile", json::Value(std::move(curve)));
    }
    document.generators.push_back(trunk);
    return document;
}

} // namespace

CANOPY_TEST(trunk_evaluates_with_stable_hashes) {
    const Document document = trunk_document();
    auto first = evaluate(document, EvaluationProfile::production());
    auto second = evaluate(document, EvaluationProfile::production());
    CHECK(first.ok() && second.ok());
    if (first.ok() && second.ok()) {
        CHECK_EQ(first.value().nodes.size(), std::size_t{1});
        CHECK(first.value().model_hash() == second.value().model_hash());
        CHECK(first.value().model_topology_hash() == second.value().model_topology_hash());
        CHECK(first.value().warnings.empty());
        const auto& node = first.value().nodes.front();
        CHECK(node.material_id == uuid('b'));
        CHECK_NEAR(node.length_m, 4.0, 1e-12);
        CHECK(node.mesh.triangle_count() > 0);
        CHECK(geo::validate_mesh(node.mesh).ok());
    }
}

CANOPY_TEST(profiles_change_density_not_identity) {
    const Document document = trunk_document();
    auto draft = evaluate(document, EvaluationProfile::draft());
    auto production = evaluate(document, EvaluationProfile::production());
    CHECK(draft.ok() && production.ok());
    if (draft.ok() && production.ok()) {
        // Same semantic identity (B-028 acceptance)...
        CHECK(draft.value().nodes.front().semantic_id ==
              production.value().nodes.front().semantic_id);
        // ...different density.
        CHECK(draft.value().total_triangles() < production.value().total_triangles());
    }
}

CANOPY_TEST(seed_changes_do_not_move_fixed_placements) {
    // A single absolute trunk uses no random draws: changing the seed changes
    // semantic IDs (they include the seed) but not the geometry shape.
    Document document = trunk_document();
    auto base = evaluate(document, EvaluationProfile::production());
    document.manifest.seed = 999;
    auto reseeded = evaluate(document, EvaluationProfile::production());
    CHECK(base.ok() && reseeded.ok());
    if (base.ok() && reseeded.ok()) {
        CHECK(base.value().model_topology_hash() != 0);
        CHECK_EQ(base.value().nodes.front().mesh.vertex_count(),
                 reseeded.value().nodes.front().mesh.vertex_count());
        CHECK(base.value().nodes.front().semantic_id !=
              reseeded.value().nodes.front().semantic_id);
    }
}

CANOPY_TEST(interval_children_are_deterministic) {
    Document document = trunk_document();
    GeneratorInstance branches;
    branches.id = uuid('3');
    branches.type = "canopy.branch";
    branches.name = "Boughs";
    branches.parent = uuid('2');
    branches.properties.emplace("generation.mode", json::Value("interval"));
    branches.properties.emplace("generation.spacing.relative", json::Value(0.1));
    branches.properties.emplace("generation.first", json::Value(0.3));
    branches.properties.emplace("generation.last", json::Value(0.9));
    branches.properties.emplace("spine.length.absolute", json::Value(1.5));
    branches.properties.emplace("spine.radius.absolute", json::Value(0.06));
    document.generators.push_back(branches);

    auto first = evaluate(document, EvaluationProfile::preview());
    auto second = evaluate(document, EvaluationProfile::preview());
    CHECK(first.ok() && second.ok());
    if (first.ok() && second.ok()) {
        // Trunk + 7 interval placements (0.3 → 0.9 step 0.1).
        CHECK_EQ(first.value().nodes.size(), std::size_t{8});
        CHECK(first.value().model_hash() == second.value().model_hash());
        // Random azimuths must differ between sibling branches (named streams
        // keyed by semantic ID), which shows as differing geometry hashes.
        CHECK(geo::geometry_hash(first.value().nodes[1].mesh) !=
              geo::geometry_hash(first.value().nodes[2].mesh));
    }
}

CANOPY_TEST(unrelated_edit_preserves_sibling_random_decisions) {
    // B-018 acceptance shape: adding a second child generator must not change
    // the first generator's random decisions (streams key on generator +
    // semantic ID, not on evaluation order).
    Document document = trunk_document();
    GeneratorInstance branches;
    branches.id = uuid('3');
    branches.type = "canopy.branch";
    branches.name = "Boughs";
    branches.parent = uuid('2');
    branches.properties.emplace("generation.mode", json::Value("interval"));
    branches.properties.emplace("generation.spacing.relative", json::Value(0.2));
    branches.properties.emplace("spine.length.absolute", json::Value(1.5));
    branches.properties.emplace("spine.radius.absolute", json::Value(0.06));
    document.generators.push_back(branches);

    auto before = evaluate(document, EvaluationProfile::preview());

    GeneratorInstance late;
    late.id = uuid('9');
    late.type = "canopy.branch";
    late.name = "Late";
    late.parent = uuid('2');
    late.properties.emplace("generation.mode", json::Value("absolute"));
    late.properties.emplace("generation.count", json::Value(2));
    late.properties.emplace("spine.length.absolute", json::Value(0.8));
    late.properties.emplace("spine.radius.absolute", json::Value(0.03));
    document.generators.push_back(late);

    auto after = evaluate(document, EvaluationProfile::preview());
    CHECK(before.ok() && after.ok());
    if (before.ok() && after.ok()) {
        // Every node of the first evaluation appears unchanged in the second.
        for (const auto& node : before.value().nodes) {
            bool found = false;
            for (const auto& candidate : after.value().nodes) {
                if (candidate.semantic_id == node.semantic_id) {
                    CHECK(geo::geometry_hash(candidate.mesh) == geo::geometry_hash(node.mesh));
                    found = true;
                    break;
                }
            }
            CHECK(found);
        }
        CHECK_EQ(after.value().nodes.size(), before.value().nodes.size() + 2);
    }
}

CANOPY_TEST(flare_widens_base_and_decays) {
    Document document = trunk_document();
    auto plain = evaluate(document, EvaluationProfile::production());
    document.generators[1].properties.emplace("spine.flare.relative", json::Value(0.6));
    document.generators[1].properties.emplace("spine.flare.length.relative", json::Value(0.2));
    auto flared = evaluate(document, EvaluationProfile::production());
    CHECK(plain.ok() && flared.ok());
    if (plain.ok() && flared.ok()) {
        const auto& a = plain.value().nodes.front().mesh;
        const auto& b = flared.value().nodes.front().mesh;
        CHECK(geo::topology_hash(a) == geo::topology_hash(b)); // same connectivity
        auto ring_radius = [](const geo::TriangleMesh& mesh, std::size_t start) {
            const Vec3& p = mesh.positions[start];
            return std::sqrt(p.x * p.x + p.z * p.z);
        };
        // Base ring 60% wider; radius at the top unchanged.
        CHECK_NEAR(ring_radius(b, 0), ring_radius(a, 0) * 1.6, 1e-9);
        const std::size_t last_ring_start = a.vertex_count() - 1 - 9 - 9; // before cap+tip data
        CHECK_NEAR(ring_radius(b, last_ring_start), ring_radius(a, last_ring_start), 1e-6);
    }
}

CANOPY_TEST(ground_clamp_keeps_spine_above_level) {
    Document document = trunk_document();
    GeneratorInstance roots;
    roots.id = uuid('7');
    roots.type = "canopy.branch";
    roots.name = "Roots";
    roots.parent = uuid('2');
    roots.properties.emplace("generation.mode", json::Value("absolute"));
    roots.properties.emplace("generation.count", json::Value(4));
    roots.properties.emplace("generation.first", json::Value(0.01));
    roots.properties.emplace("generation.last", json::Value(0.03));
    roots.properties.emplace("generation.angle.degrees", json::Value(115.0)); // below horizontal
    roots.properties.emplace("spine.length.absolute", json::Value(2.0));
    roots.properties.emplace("spine.radius.absolute", json::Value(0.1));
    roots.properties.emplace("spine.ground.level", json::Value(0.0));
    document.generators.push_back(roots);
    auto model = evaluate(document, EvaluationProfile::preview());
    CHECK(model.ok());
    if (model.ok()) {
        for (const auto& node : model.value().nodes) {
            if (node.generator_id != uuid('7')) {
                continue;
            }
            // Spine (mesh center-line) never dips more than a radius below
            // the clamp plane; without the clamp a 115-degree root at 2 m
            // would reach y ≈ -1.8.
            for (const auto& p : node.mesh.positions) {
                CHECK(p.y > -0.25);
            }
        }
    }
}

CANOPY_TEST(uv_random_phase_differs_per_node_and_is_stable) {
    Document document = trunk_document();
    GeneratorInstance branches;
    branches.id = uuid('3');
    branches.type = "canopy.branch";
    branches.name = "Boughs";
    branches.parent = uuid('2');
    branches.properties.emplace("generation.mode", json::Value("interval"));
    branches.properties.emplace("generation.spacing.relative", json::Value(0.3));
    branches.properties.emplace("spine.length.absolute", json::Value(1.0));
    branches.properties.emplace("spine.radius.absolute", json::Value(0.04));
    branches.properties.emplace("mesh.uv.random_phase", json::Value(true));
    document.generators.push_back(branches);
    auto first = evaluate(document, EvaluationProfile::preview());
    auto second = evaluate(document, EvaluationProfile::preview());
    CHECK(first.ok() && second.ok());
    if (first.ok() && second.ok()) {
        CHECK(first.value().model_hash() == second.value().model_hash());
        // Sibling boughs share topology but differ in UV phase.
        const auto& nodes = first.value().nodes;
        CHECK(nodes.size() >= 3);
        if (nodes.size() >= 3) {
            const auto& m1 = nodes[1].mesh;
            const auto& m2 = nodes[2].mesh;
            CHECK(std::fabs(m1.uvs[0].y - m2.uvs[0].y) > 1e-6);
        }
    }
}

CANOPY_TEST(batched_leaves_are_deterministic_and_countable) {
    Document document = trunk_document();
    Material leaf_material;
    leaf_material.id = uuid('c');
    leaf_material.name = "leaf_default";
    document.materials.push_back(leaf_material);

    GeneratorInstance leaves;
    leaves.id = uuid('5');
    leaves.type = "canopy.batched_leaf";
    leaves.name = "Leaves";
    leaves.parent = uuid('2');
    leaves.properties.emplace("generation.spacing.relative", json::Value(0.1));
    leaves.properties.emplace("generation.first", json::Value(0.5));
    leaves.properties.emplace("generation.last", json::Value(1.0));
    leaves.properties.emplace("generation.leaves_per_point", json::Value(3));
    leaves.properties.emplace("material.leaf", json::Value(uuid('c').str()));
    document.generators.push_back(leaves);

    auto first = evaluate(document, EvaluationProfile::preview());
    auto second = evaluate(document, EvaluationProfile::preview());
    CHECK(first.ok() && second.ok());
    if (first.ok() && second.ok()) {
        CHECK_EQ(first.value().nodes.size(), std::size_t{2}); // trunk + one batch
        CHECK(first.value().model_hash() == second.value().model_hash());
        const BranchNodeGeometry* batch = nullptr;
        for (const auto& node : first.value().nodes) {
            if (node.generator_id == uuid('5')) {
                batch = &node;
            }
        }
        CHECK(batch != nullptr);
        if (batch != nullptr) {
            // 6 placement points (0.5 → 1.0 step 0.1) × 3 leaves × 2 triangles.
            CHECK_EQ(batch->mesh.triangle_count(), std::size_t{36});
            CHECK(batch->material_id == uuid('c'));
            CHECK(geo::validate_mesh(batch->mesh).ok());
        }
    }
}

CANOPY_TEST(leaf_mesh_emits_individual_semantic_nodes) {
    Document document = trunk_document();
    GeneratorInstance leaves;
    leaves.id = uuid('5');
    leaves.type = "canopy.leaf_mesh";
    leaves.name = "HeroLeaves";
    leaves.parent = uuid('2');
    leaves.properties.emplace("generation.spacing.relative", json::Value(0.25));
    leaves.properties.emplace("generation.first", json::Value(0.5));
    leaves.properties.emplace("generation.last", json::Value(1.0));
    leaves.properties.emplace("generation.leaves_per_point", json::Value(2));
    document.generators.push_back(leaves);

    auto model = evaluate(document, EvaluationProfile::preview());
    CHECK(model.ok());
    if (model.ok()) {
        // Trunk + 3 points × 2 leaves as individual nodes.
        CHECK_EQ(model.value().nodes.size(), std::size_t{7});
        std::set<SemanticId> ids;
        for (const auto& node : model.value().nodes) {
            CHECK(ids.insert(node.semantic_id).second); // all distinct
            if (node.generator_id == uuid('5')) {
                CHECK_EQ(node.mesh.triangle_count(), std::size_t{2}); // default card
            }
        }
    }
}

CANOPY_TEST(cutout_material_shapes_leaves) {
    Document document = trunk_document();
    Material leaf_material;
    leaf_material.id = uuid('c');
    leaf_material.name = "leaf_cutout";
    MaterialCutout cutout;
    // Hexagonal blade: 6 vertices → 4 triangles per leaf.
    cutout.vertices = {{0.0, 0.0}, {0.3, 0.3}, {0.25, 0.7}, {0.0, 1.0}, {-0.25, 0.7}, {-0.3, 0.3}};
    leaf_material.cutout = cutout;
    document.materials.push_back(leaf_material);

    GeneratorInstance leaves;
    leaves.id = uuid('5');
    leaves.type = "canopy.batched_leaf";
    leaves.name = "Leaves";
    leaves.parent = uuid('2');
    leaves.properties.emplace("generation.spacing.relative", json::Value(0.25));
    leaves.properties.emplace("generation.first", json::Value(0.5));
    leaves.properties.emplace("generation.leaves_per_point", json::Value(1));
    leaves.properties.emplace("material.leaf", json::Value(uuid('c').str()));
    document.generators.push_back(leaves);

    auto model = evaluate(document, EvaluationProfile::preview());
    CHECK(model.ok());
    if (model.ok()) {
        const BranchNodeGeometry* batch = nullptr;
        for (const auto& node : model.value().nodes) {
            if (node.generator_id == uuid('5')) {
                batch = &node;
            }
        }
        CHECK(batch != nullptr);
        if (batch != nullptr) {
            // 3 placement points × 1 leaf × 4 triangles (hexagon), 6 verts each.
            CHECK_EQ(batch->mesh.triangle_count(), std::size_t{12});
            CHECK_EQ(batch->mesh.vertex_count(), std::size_t{18});
            CHECK(geo::validate_mesh(batch->mesh).ok());
        }
    }
}

CANOPY_TEST(frond_generator_produces_ribbon) {
    Document document = trunk_document();
    GeneratorInstance fronds;
    fronds.id = uuid('5');
    fronds.type = "canopy.frond";
    fronds.name = "Fronds";
    fronds.parent = uuid('2');
    fronds.properties.emplace("generation.mode", json::Value("absolute"));
    fronds.properties.emplace("generation.count", json::Value(4));
    fronds.properties.emplace("generation.first", json::Value(0.95));
    fronds.properties.emplace("generation.last", json::Value(1.0));
    fronds.properties.emplace("spine.length.absolute", json::Value(1.8));
    fronds.properties.emplace("spine.bend.degrees", json::Value(50.0));
    fronds.properties.emplace("frond.width.absolute", json::Value(0.4));
    fronds.properties.emplace("frond.fold.degrees", json::Value(45.0));
    fronds.properties.emplace("frond.serration.count", json::Value(12));
    document.generators.push_back(fronds);

    auto first = evaluate(document, EvaluationProfile::production());
    auto second = evaluate(document, EvaluationProfile::production());
    CHECK(first.ok() && second.ok());
    if (first.ok() && second.ok()) {
        CHECK(first.value().model_hash() == second.value().model_hash());
        std::size_t frond_nodes = 0;
        for (const auto& node : first.value().nodes) {
            if (node.generator_id != uuid('5')) {
                continue;
            }
            ++frond_nodes;
            // Ribbon topology: vertices divisible by 3 (left/midrib/right rows).
            CHECK_EQ(node.mesh.vertex_count() % 3, std::size_t{0});
            CHECK(node.mesh.triangle_count() >= 4);
            CHECK(geo::validate_mesh(node.mesh).ok());
        }
        CHECK_EQ(frond_nodes, std::size_t{4});
    }
}

CANOPY_TEST(leaf_under_tree_root_fails_validation) {
    Document document = trunk_document();
    GeneratorInstance leaves;
    leaves.id = uuid('6');
    leaves.type = "canopy.batched_leaf";
    leaves.name = "Leaves";
    leaves.parent = uuid('1'); // tree root: not an allowed parent
    document.generators.push_back(leaves);
    CHECK(!document.validate().ok());
}

CANOPY_TEST(phyllotaxy_places_whorls_with_deterministic_azimuths) {
    Document document = trunk_document();
    GeneratorInstance whorls;
    whorls.id = uuid('3');
    whorls.type = "canopy.branch";
    whorls.name = "Whorls";
    whorls.parent = uuid('2');
    whorls.properties.emplace("generation.mode", json::Value("phyllotaxy"));
    whorls.properties.emplace("generation.internode.absolute", json::Value(1.0));
    whorls.properties.emplace("generation.members_per_whorl", json::Value(3));
    whorls.properties.emplace("generation.first", json::Value(0.25));
    whorls.properties.emplace("generation.last", json::Value(1.0));
    whorls.properties.emplace("spine.length.absolute", json::Value(1.0));
    whorls.properties.emplace("spine.radius.absolute", json::Value(0.03));
    document.generators.push_back(whorls);

    auto first = evaluate(document, EvaluationProfile::preview());
    auto second = evaluate(document, EvaluationProfile::preview());
    CHECK(first.ok() && second.ok());
    if (first.ok() && second.ok()) {
        // 4 m trunk, 1 m internode from t=0.25: whorls at 0.25, 0.5, 0.75,
        // 1.0 → 4 whorls × 3 members + trunk.
        CHECK_EQ(first.value().nodes.size(), std::size_t{13});
        // No random azimuth draws: results are bit-identical.
        CHECK(first.value().model_hash() == second.value().model_hash());
        // Members of one whorl differ in azimuth → different geometry.
        CHECK(geo::geometry_hash(first.value().nodes[1].mesh) !=
              geo::geometry_hash(first.value().nodes[2].mesh));
    }
}

CANOPY_TEST(proportional_count_follows_parent_length) {
    Document document = trunk_document();
    GeneratorInstance shoots;
    shoots.id = uuid('3');
    shoots.type = "canopy.branch";
    shoots.name = "Shoots";
    shoots.parent = uuid('2');
    shoots.properties.emplace("generation.mode", json::Value("proportional"));
    shoots.properties.emplace("generation.density.per_meter", json::Value(2.0));
    shoots.properties.emplace("spine.length.absolute", json::Value(0.8));
    shoots.properties.emplace("spine.radius.absolute", json::Value(0.03));
    document.generators.push_back(shoots);

    auto short_trunk = evaluate(document, EvaluationProfile::preview());
    document.generators[1].properties.insert_or_assign("spine.length.absolute",
                                                       json::Value(8.0));
    auto long_trunk = evaluate(document, EvaluationProfile::preview());
    CHECK(short_trunk.ok() && long_trunk.ok());
    if (short_trunk.ok() && long_trunk.ok()) {
        // 4 m × 2/m = 8 shoots; 8 m × 2/m = 16 shoots (+1 trunk node each).
        CHECK_EQ(short_trunk.value().nodes.size(), std::size_t{9});
        CHECK_EQ(long_trunk.value().nodes.size(), std::size_t{17});
    }
}

CANOPY_TEST(bifurcation_splits_at_parent_tip) {
    Document document = trunk_document();
    GeneratorInstance forks;
    forks.id = uuid('3');
    forks.type = "canopy.branch";
    forks.name = "Forks";
    forks.parent = uuid('2');
    forks.properties.emplace("generation.mode", json::Value("bifurcation"));
    forks.properties.emplace("generation.count", json::Value(3));
    forks.properties.emplace("generation.angle.degrees", json::Value(25.0));
    forks.properties.emplace("spine.length.absolute", json::Value(1.5));
    forks.properties.emplace("spine.radius.absolute", json::Value(0.08));
    document.generators.push_back(forks);

    auto model = evaluate(document, EvaluationProfile::preview());
    CHECK(model.ok());
    if (model.ok()) {
        CHECK_EQ(model.value().nodes.size(), std::size_t{4}); // trunk + 3 forks
        // The straight 4 m trunk tip is at y = 4: every fork base vertex must
        // start in that neighborhood (branches grow upward from the tip).
        for (const auto& node : model.value().nodes) {
            if (node.generator_id != uuid('3')) {
                continue;
            }
            double min_y = 1e9;
            for (const auto& p : node.mesh.positions) {
                min_y = std::min(min_y, p.y);
            }
            CHECK(min_y > 3.5);
        }
    }
}

CANOPY_TEST(evaluation_errors_are_contained) {
    Document document = trunk_document();
    document.generators[1].properties.insert_or_assign("spine.length.absolute",
                                                       json::Value(-3.0));
    auto result = evaluate(document, EvaluationProfile::draft());
    CHECK(!result.ok());
    if (!result.ok()) {
        CHECK(result.error().code == ErrorCode::evaluation_failure);
        CHECK(!result.error().notes.empty());
    }
}

CANOPY_TEST_MAIN()
