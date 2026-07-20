#include "canopy/export/rt_compile.hpp"
#include "canopy/runtime/forest.hpp"
#include "canopy/runtime/model.hpp"
#include "canopy_test.hpp"

#include <filesystem>
#include <fstream>

using namespace canopy;
using namespace canopy::doc;
using namespace canopy::eval;

namespace {

Uuid uuid(char digit) {
    std::string text = "00000000-0000-4000-8000-000000000000";
    text[35] = digit;
    return *Uuid::parse(text);
}

Document sample_document() {
    Document document;
    document.manifest.document_id = uuid('a');
    document.manifest.name = "RtSample";
    document.manifest.seed = 11;
    GeneratorInstance tree;
    tree.id = uuid('1');
    tree.type = "canopy.tree";
    tree.name = "Tree";
    document.generators.push_back(tree);
    Material bark;
    bark.id = uuid('b');
    bark.name = "bark";
    document.materials.push_back(bark);
    Material leaf;
    leaf.id = uuid('c');
    leaf.name = "leaf";
    leaf.two_sided = true;
    document.materials.push_back(leaf);
    GeneratorInstance trunk;
    trunk.id = uuid('2');
    trunk.type = "canopy.branch";
    trunk.name = "Trunk";
    trunk.parent = uuid('1');
    trunk.properties.emplace("spine.length.absolute", json::Value(3.0));
    trunk.properties.emplace("spine.radius.absolute", json::Value(0.2));
    trunk.properties.emplace("mesh.radial_segments", json::Value(12));
    trunk.properties.emplace("material.bark", json::Value(uuid('b').str()));
    document.generators.push_back(trunk);
    GeneratorInstance leaves;
    leaves.id = uuid('3');
    leaves.type = "canopy.batched_leaf";
    leaves.name = "Leaves";
    leaves.parent = uuid('2');
    leaves.properties.emplace("material.leaf", json::Value(uuid('c').str()));
    document.generators.push_back(leaves);
    return document;
}

std::filesystem::path compile_sample(const char* name) {
    const auto root = std::filesystem::temp_directory_path() / "canopy-test-runtime";
    std::filesystem::create_directories(root);
    const Document document = sample_document();
    std::vector<EvaluatedModel> lods;
    for (const auto& profile : {EvaluationProfile::production(), EvaluationProfile::preview(),
                                EvaluationProfile::draft()}) {
        auto model = evaluate(document, profile);
        CHECK(model.ok());
        lods.push_back(std::move(model).value());
    }
    auto manifest = exp::write_canopyrt(document, lods, root / name);
    CHECK(manifest.ok());
    return (root / name).string() + ".canopyrt";
}

std::string read_all(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), {});
}

} // namespace

CANOPY_TEST(canopyrt_roundtrip_preserves_counts_and_bounds) {
    const auto path = compile_sample("roundtrip");
    auto loaded = rt::load_model(path);
    CHECK(loaded.ok());
    if (!loaded.ok()) {
        return;
    }
    const rt::RtModel& model = loaded.value();
    CHECK_EQ(model.name, std::string("RtSample"));
    CHECK_EQ(model.lods.size(), std::size_t{3});
    CHECK_EQ(model.materials.size(), std::size_t{2}); // bark + leaf
    // LOD 0 (production) has strictly more triangles than LOD 2 (draft).
    CHECK(model.lods[0].triangle_count() > model.lods[2].triangle_count());
    // Bounds contain every LOD-0 vertex and have positive height.
    CHECK(model.bounds.maximum[1] > 2.0);
    for (std::size_t v = 0; v < model.lods[0].vertex_count(); ++v) {
        for (std::size_t c = 0; c < 3; ++c) {
            const double value = double(model.lods[0].vertices[v * 8 + c]);
            CHECK(value >= model.bounds.minimum[c] - 1e-3);
            CHECK(value <= model.bounds.maximum[c] + 1e-3);
        }
    }
    // Repeat compile is byte-identical.
    const auto again = compile_sample("roundtrip2");
    CHECK(read_all(path) == read_all(again));
}

CANOPY_TEST(canopyrt_rejects_corruption_and_truncation) {
    const auto path = compile_sample("corrupt");
    std::string bytes = read_all(path);
    CHECK(rt::load_model_bytes(bytes).ok());
    // Flip one payload byte: some section checksum must fail.
    std::string corrupted = bytes;
    corrupted[corrupted.size() / 2] = char(corrupted[corrupted.size() / 2] ^ 0x5a);
    auto flipped = rt::load_model_bytes(corrupted);
    CHECK(!flipped.ok());
    if (!flipped.ok()) {
        CHECK(flipped.error().code == ErrorCode::corrupt_data);
    }
    // Truncation at any of several points fails cleanly (no crashes/UB).
    for (const double fraction : {0.1, 0.5, 0.9}) {
        auto truncated =
            rt::load_model_bytes(std::string_view(bytes).substr(0, std::size_t(
                                     double(bytes.size()) * fraction)));
        CHECK(!truncated.ok());
    }
    // Wrong magic.
    std::string wrong = bytes;
    wrong[0] = 'X';
    CHECK(!rt::load_model_bytes(wrong).ok());
}

CANOPY_TEST(forest_scatter_is_deterministic_and_respects_spacing) {
    const auto path = compile_sample("forest");
    auto loaded = rt::load_model(path);
    CHECK(loaded.ok());
    if (!loaded.ok()) {
        return;
    }
    rt::Forest first;
    first.add_model(&loaded.value());
    CHECK(first.scatter(42, 30, 20.0, 2.5).ok());
    rt::Forest second;
    second.add_model(&loaded.value());
    CHECK(second.scatter(42, 30, 20.0, 2.5).ok());
    CHECK_EQ(first.instances().size(), second.instances().size());
    CHECK(first.instances().size() >= 20); // dense-but-satisfiable request
    for (std::size_t i = 0; i < first.instances().size(); ++i) {
        CHECK_NEAR(first.instances()[i].position.x, second.instances()[i].position.x, 0.0);
        CHECK_NEAR(first.instances()[i].yaw, second.instances()[i].yaw, 0.0);
        // Spacing holds.
        for (std::size_t j = 0; j < i; ++j) {
            const double dx = first.instances()[i].position.x - first.instances()[j].position.x;
            const double dz = first.instances()[i].position.z - first.instances()[j].position.z;
            CHECK(dx * dx + dz * dz >= 2.5 * 2.5 - 1e-9);
        }
    }
    // A different seed moves the forest.
    rt::Forest reseeded;
    reseeded.add_model(&loaded.value());
    CHECK(reseeded.scatter(43, 30, 20.0, 2.5).ok());
    CHECK(reseeded.instances()[0].position.x != first.instances()[0].position.x);
}

CANOPY_TEST(forest_lod_selection_follows_distance) {
    const auto path = compile_sample("lod");
    auto loaded = rt::load_model(path);
    CHECK(loaded.ok());
    if (!loaded.ok()) {
        return;
    }
    rt::Forest forest;
    forest.add_model(&loaded.value());
    CHECK(forest.scatter(7, 60, 80.0, 4.0).ok());
    // Camera on top of instance 0: guarantees a LOD-0 pick; the far side of
    // the 160 m scatter square guarantees deepest-LOD picks.
    const Vec3 camera = forest.instances().front().position + Vec3{0.0, 2.0, 0.0};
    const auto visible = forest.select(camera, 0.0);
    CHECK_EQ(visible.size(), forest.instances().size()); // no distance cull
    bool saw_lod0 = false;
    bool saw_deep = false;
    for (const auto& item : visible) {
        const double distance = length(item.instance->position - camera);
        if (item.lod == 0) {
            saw_lod0 = true;
        }
        if (item.lod == 2) {
            saw_deep = true;
        }
        // Monotonicity spot check: nothing near is at the deepest LOD.
        if (distance < 8.0 * loaded.value().bounds.radius()) {
            CHECK_EQ(item.lod, 0u);
        }
    }
    CHECK(saw_lod0);
    CHECK(saw_deep);
    // Distance culling drops far instances.
    const auto culled = forest.select(camera, 30.0);
    CHECK(culled.size() < visible.size());
}

CANOPY_TEST_MAIN()
