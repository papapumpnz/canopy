#include "canopy/export/gltf_export.hpp"
#include "canopy_test.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>

using namespace canopy;
using namespace canopy::doc;
using namespace canopy::eval;
using namespace canopy::exp;

namespace {

Uuid uuid(char digit) {
    std::string text = "00000000-0000-4000-8000-000000000000";
    text[35] = digit;
    return *Uuid::parse(text);
}

Document sample_document() {
    Document document;
    document.manifest.document_id = uuid('a');
    document.manifest.name = "GltfSample";
    document.manifest.seed = 7;
    GeneratorInstance tree;
    tree.id = uuid('1');
    tree.type = "canopy.tree";
    tree.name = "Tree";
    document.generators.push_back(tree);
    Material bark;
    bark.id = uuid('b');
    bark.name = "bark";
    bark.base_color = {0.4, 0.3, 0.2, 1.0};
    document.materials.push_back(bark);
    Material leaf;
    leaf.id = uuid('c');
    leaf.name = "leaf";
    leaf.base_color = {0.3, 0.5, 0.2, 1.0};
    leaf.two_sided = true;
    document.materials.push_back(leaf);
    GeneratorInstance trunk;
    trunk.id = uuid('2');
    trunk.type = "canopy.branch";
    trunk.name = "Trunk";
    trunk.parent = uuid('1');
    trunk.properties.emplace("spine.length.absolute", json::Value(3.0));
    trunk.properties.emplace("spine.radius.absolute", json::Value(0.2));
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

std::string read_all(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), {});
}

std::uint32_t read_u32(const std::string& bytes, std::size_t offset) {
    return std::uint32_t(std::uint8_t(bytes[offset])) |
           std::uint32_t(std::uint8_t(bytes[offset + 1])) << 8 |
           std::uint32_t(std::uint8_t(bytes[offset + 2])) << 16 |
           std::uint32_t(std::uint8_t(bytes[offset + 3])) << 24;
}

} // namespace

CANOPY_TEST(glb_container_is_wellformed_and_deterministic) {
    const auto root = std::filesystem::temp_directory_path() / "canopy-test-gltf";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);

    const Document document = sample_document();
    auto model = evaluate(document, EvaluationProfile::preview());
    CHECK(model.ok());
    if (!model.ok()) {
        return;
    }
    ExportPreset preset;
    preset.format = "gltf";
    auto first = write_glb(document, model.value(), preset, root / "tree");
    CHECK(first.ok());
    if (!first.ok()) {
        return;
    }
    const std::string bytes = read_all(root / "tree.glb");
    CHECK(bytes.size() > 44);
    // Header: magic, version 2, total length matches the file.
    CHECK_EQ(bytes.substr(0, 4), std::string("glTF"));
    CHECK_EQ(read_u32(bytes, 4), 2u);
    CHECK_EQ(read_u32(bytes, 8), std::uint32_t(bytes.size()));
    // JSON chunk: 4-byte aligned, declared type, parses with our own parser.
    const std::uint32_t json_length = read_u32(bytes, 12);
    CHECK_EQ(bytes.substr(16, 4), std::string("JSON"));
    CHECK_EQ(json_length % 4, 0u);
    auto parsed = json::parse(bytes.substr(20, json_length), "glb-json");
    CHECK(parsed.ok());
    if (parsed.ok()) {
        const auto* asset = parsed.value().find("asset");
        CHECK(asset != nullptr && asset->find("version") != nullptr &&
              asset->find("version")->as_string() == "2.0");
        // Two materials → two primitives; accessor vertex counts match the
        // manifest totals.
        const auto* meshes = parsed.value().find("meshes");
        CHECK(meshes != nullptr && meshes->as_array().size() == 1);
        const auto& primitives =
            *meshes->as_array()[0].find("primitives");
        CHECK_EQ(primitives.as_array().size(), first.value().primitive_count);
        CHECK_EQ(first.value().primitive_count, std::size_t{2});
        std::size_t position_total = 0;
        const auto* accessors = parsed.value().find("accessors");
        CHECK(accessors != nullptr);
        for (const auto& primitive : primitives.as_array()) {
            const auto* attributes = primitive.find("attributes");
            CHECK(attributes != nullptr);
            const auto position_index =
                std::size_t(attributes->find("POSITION")->as_int());
            position_total +=
                std::size_t(accessors->as_array()[position_index].find("count")->as_int());
        }
        CHECK_EQ(position_total, first.value().vertex_count);
        // BIN chunk sized as declared.
        const std::size_t bin_header = 20 + json_length;
        CHECK_EQ(bytes.substr(bin_header + 4, 3), std::string("BIN"));
        CHECK_EQ(read_u32(bytes, bin_header), std::uint32_t(bytes.size() - bin_header - 8));
        const auto* buffers = parsed.value().find("buffers");
        CHECK(buffers != nullptr &&
              std::uint32_t(buffers->as_array()[0].find("byteLength")->as_int()) ==
                  read_u32(bytes, bin_header));
    }
    // Repeated export is byte-identical.
    auto second = write_glb(document, model.value(), preset, root / "tree2");
    CHECK(second.ok());
    if (second.ok()) {
        CHECK_EQ(read_all(root / "tree2.glb").size(), bytes.size());
        CHECK(read_all(root / "tree2.glb") == bytes);
        CHECK(second.value().glb_sha256 == first.value().glb_sha256);
    }
}

CANOPY_TEST(glb_rejects_empty_models) {
    const Document document = sample_document();
    EvaluatedModel empty;
    ExportPreset preset;
    preset.format = "gltf";
    CHECK(!write_glb(document, empty, preset,
                     std::filesystem::temp_directory_path() / "canopy-test-gltf-empty")
               .ok());
}

CANOPY_TEST_MAIN()
