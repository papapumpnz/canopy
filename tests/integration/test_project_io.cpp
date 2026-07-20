#include "canopy/document/project_io.hpp"
#include "canopy_test.hpp"

#include <filesystem>
#include <fstream>

using namespace canopy;
using namespace canopy::doc;

namespace {

std::filesystem::path temp_root() {
    const auto root =
        std::filesystem::temp_directory_path() / "canopy-test-project-io";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    return root;
}

Uuid uuid(char digit) {
    std::string text = "00000000-0000-4000-8000-000000000000";
    text[35] = digit;
    return *Uuid::parse(text);
}

Document sample_document() {
    Document document;
    document.manifest.document_id = uuid('a');
    document.manifest.name = "IO";
    document.manifest.seed = 11;
    GeneratorInstance tree;
    tree.id = uuid('1');
    tree.type = "canopy.tree";
    tree.name = "Tree";
    document.generators.push_back(tree);
    GeneratorInstance trunk;
    trunk.id = uuid('2');
    trunk.type = "canopy.branch";
    trunk.name = "Trunk";
    trunk.parent = uuid('1');
    trunk.properties.emplace("spine.length.absolute", json::Value(2.5));
    document.generators.push_back(trunk);
    return document;
}

std::string read_all(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), {});
}

} // namespace

CANOPY_TEST(save_load_resave_is_byte_identical) {
    const auto root = temp_root();
    const auto first_dir = root / "First.canopyproj";
    const auto second_dir = root / "Second.canopyproj";

    const Document document = sample_document();
    CHECK(save_project(document, first_dir).ok());
    auto loaded = load_project(first_dir);
    CHECK(loaded.ok());
    if (!loaded.ok()) {
        return;
    }
    CHECK(save_project(loaded.value(), second_dir).ok());

    for (const char* name : {"manifest.json", "graph.json", "properties.json", "materials.json"}) {
        CHECK_EQ(read_all(first_dir / name), read_all(second_dir / name));
    }

    // Document hash matches between renders of the same content (ADR-0002).
    auto files_a = render_project_files(document);
    auto files_b = render_project_files(loaded.value());
    CHECK(files_a.ok() && files_b.ok());
    if (files_a.ok() && files_b.ok()) {
        CHECK_EQ(document_hash(files_a.value()).hex(), document_hash(files_b.value()).hex());
    }
}

CANOPY_TEST(recovery_marker_blocks_load) {
    const auto root = temp_root();
    const auto dir = root / "Marked.canopyproj";
    CHECK(save_project(sample_document(), dir).ok());
    std::ofstream(dir / "RECOVERY") << "canopy save in progress\n";
    auto loaded = load_project(dir);
    CHECK(!loaded.ok());
    if (!loaded.ok()) {
        CHECK(loaded.error().code == ErrorCode::corrupt_data);
    }
}

CANOPY_TEST(missing_directory_reports_not_found) {
    auto loaded = load_project(temp_root() / "DoesNotExist.canopyproj");
    CHECK(!loaded.ok());
    if (!loaded.ok()) {
        CHECK(loaded.error().code == ErrorCode::not_found);
    }
}

CANOPY_TEST(truncated_file_reports_parse_error) {
    const auto root = temp_root();
    const auto dir = root / "Broken.canopyproj";
    CHECK(save_project(sample_document(), dir).ok());
    std::ofstream(dir / "graph.json", std::ios::binary | std::ios::trunc) << "{\"nodes\": [";
    auto loaded = load_project(dir);
    CHECK(!loaded.ok());
    if (!loaded.ok()) {
        CHECK(loaded.error().code == ErrorCode::parse_error);
    }
}

CANOPY_TEST(interrupted_save_leaves_previous_files_valid) {
    // Fault-injection shape for B-014: a save that fails mid-way (simulated by
    // making the directory read-only is platform-dependent, so instead verify
    // the temp-file protocol: a stale .tmp never shadows the real file).
    const auto root = temp_root();
    const auto dir = root / "Stale.canopyproj";
    CHECK(save_project(sample_document(), dir).ok());
    std::ofstream(dir / "graph.json.tmp", std::ios::binary) << "garbage";
    auto loaded = load_project(dir);
    CHECK(loaded.ok());
}

CANOPY_TEST_MAIN()
