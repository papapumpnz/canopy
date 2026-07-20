#include "canopy/document/document.hpp"
#include "canopy_test.hpp"

using namespace canopy;
using namespace canopy::doc;

namespace {

Uuid uuid(char digit) {
    std::string text = "00000000-0000-4000-8000-000000000000";
    text[35] = digit;
    return *Uuid::parse(text);
}

Document minimal_document() {
    Document document;
    document.manifest.document_id = uuid('a');
    document.manifest.name = "Test";
    document.manifest.seed = 7;

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
    trunk.properties.emplace("spine.length.absolute", json::Value(4.0));
    document.generators.push_back(trunk);
    return document;
}

bool has_code(const Diagnostic& diagnostic, ErrorCode code) {
    if (diagnostic.code == code) {
        return true;
    }
    for (const auto& note : diagnostic.notes) {
        if (has_code(note, code)) {
            return true;
        }
    }
    return false;
}

} // namespace

CANOPY_TEST(valid_document_passes) {
    CHECK(minimal_document().validate().ok());
}

CANOPY_TEST(missing_root_fails) {
    Document document = minimal_document();
    document.generators.erase(document.generators.begin());
    auto result = document.validate();
    CHECK(!result.ok());
    if (!result.ok()) {
        CHECK(has_code(result.error(), ErrorCode::graph_cardinality));
        // The unknown-parent reference is also reported.
        CHECK(has_code(result.error(), ErrorCode::not_found));
    }
}

CANOPY_TEST(two_roots_fail) {
    Document document = minimal_document();
    GeneratorInstance second_tree;
    second_tree.id = uuid('3');
    second_tree.type = "canopy.tree";
    second_tree.name = "Tree2";
    document.generators.push_back(second_tree);
    auto result = document.validate();
    CHECK(!result.ok() && has_code(result.error(), ErrorCode::graph_cardinality));
}

CANOPY_TEST(cycle_fails_with_stable_code) {
    Document document = minimal_document();
    GeneratorInstance a;
    a.id = uuid('3');
    a.type = "canopy.branch";
    a.name = "A";
    a.parent = uuid('4');
    GeneratorInstance b;
    b.id = uuid('4');
    b.type = "canopy.branch";
    b.name = "B";
    b.parent = uuid('3');
    document.generators.push_back(a);
    document.generators.push_back(b);
    auto result = document.validate();
    CHECK(!result.ok() && has_code(result.error(), ErrorCode::graph_cycle));
}

CANOPY_TEST(unknown_type_fails) {
    Document document = minimal_document();
    document.generators[1].type = "canopy.unknown";
    auto result = document.validate();
    CHECK(!result.ok() && has_code(result.error(), ErrorCode::graph_type_mismatch));
}

CANOPY_TEST(branch_under_nothing_fails) {
    Document document = minimal_document();
    document.generators[1].parent = Uuid{};
    auto result = document.validate();
    CHECK(!result.ok() && has_code(result.error(), ErrorCode::graph_cardinality));
}

CANOPY_TEST(duplicate_id_fails) {
    Document document = minimal_document();
    document.generators[1].id = uuid('1');
    auto result = document.validate();
    CHECK(!result.ok() && has_code(result.error(), ErrorCode::schema_violation));
}

CANOPY_TEST(topological_order_is_uuid_sorted) {
    Document document = minimal_document();
    // Insert children in reverse UUID order; traversal must sort by UUID.
    GeneratorInstance late;
    late.id = uuid('9');
    late.type = "canopy.branch";
    late.name = "Late";
    late.parent = uuid('1');
    GeneratorInstance early;
    early.id = uuid('3');
    early.type = "canopy.branch";
    early.name = "Early";
    early.parent = uuid('1');
    document.generators.push_back(late);
    document.generators.push_back(early);
    auto order = document.topological_order();
    CHECK(order.ok());
    if (order.ok()) {
        CHECK_EQ(order.value().size(), std::size_t{4});
        CHECK_EQ(order.value()[0]->name, std::string("Tree"));
        CHECK_EQ(order.value()[1]->name, std::string("Trunk"));
        CHECK_EQ(order.value()[2]->name, std::string("Early"));
        CHECK_EQ(order.value()[3]->name, std::string("Late"));
    }
}

CANOPY_TEST(json_roundtrip_preserves_document) {
    const Document document = minimal_document();
    auto restored = document_from_json(manifest_to_json(document.manifest),
                                       graph_to_json(document), properties_to_json(document),
                                       materials_to_json(document));
    CHECK(restored.ok());
    if (restored.ok()) {
        CHECK(restored.value().manifest.document_id == document.manifest.document_id);
        CHECK_EQ(restored.value().generators.size(), document.generators.size());
        const auto* trunk = restored.value().find_generator(uuid('2'));
        CHECK(trunk != nullptr);
        if (trunk != nullptr) {
            const auto it = trunk->properties.find("spine.length.absolute");
            CHECK(it != trunk->properties.end() && it->second.as_number() == 4.0);
        }
    }
}

CANOPY_TEST(unsupported_schema_version_rejected) {
    const Document document = minimal_document();
    auto manifest = manifest_to_json(document.manifest);
    manifest.as_object().insert_or_assign("schema_version", json::Value("9.0.0"));
    auto restored = document_from_json(manifest, graph_to_json(document),
                                       properties_to_json(document), materials_to_json(document));
    CHECK(!restored.ok());
    if (!restored.ok()) {
        CHECK(restored.error().code == ErrorCode::unsupported_version);
    }
}

CANOPY_TEST_MAIN()
