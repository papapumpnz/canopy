#include "canopy/evaluation/evaluate.hpp"
#include "canopy_test.hpp"

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
