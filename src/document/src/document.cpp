#include "canopy/document/document.hpp"

#include "canopy/document/registry.hpp"

#include <algorithm>
#include <array>
#include <map>
#include <set>
#include <utility>

namespace canopy::doc {

const GeneratorInstance* Document::find_generator(const Uuid& id) const {
    for (const auto& generator : generators) {
        if (generator.id == id) {
            return &generator;
        }
    }
    return nullptr;
}

const Material* Document::find_material(const Uuid& id) const {
    for (const auto& material : materials) {
        if (material.id == id) {
            return &material;
        }
    }
    return nullptr;
}

Result<std::vector<const GeneratorInstance*>> Document::topological_order() const {
    if (auto valid = validate(); !valid.ok()) {
        return valid.take_error();
    }
    // Children sorted by UUID under each parent: deterministic traversal
    // independent of container insertion order (B-012 acceptance).
    std::map<Uuid, std::vector<const GeneratorInstance*>> children;
    const GeneratorInstance* root = nullptr;
    for (const auto& generator : generators) {
        if (generator.parent.is_nil()) {
            root = &generator;
        } else {
            children[generator.parent].push_back(&generator);
        }
    }
    for (auto& [parent, list] : children) {
        std::sort(list.begin(), list.end(),
                  [](const GeneratorInstance* a, const GeneratorInstance* b) {
                      return a->id < b->id;
                  });
    }
    std::vector<const GeneratorInstance*> order;
    order.reserve(generators.size());
    std::vector<const GeneratorInstance*> stack{root};
    while (!stack.empty()) {
        const GeneratorInstance* current = stack.back();
        stack.pop_back();
        order.push_back(current);
        const auto it = children.find(current->id);
        if (it != children.end()) {
            // Reverse push keeps UUID-sorted order on pop.
            for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
                stack.push_back(*rit);
            }
        }
    }
    return order;
}

Result<void> Document::validate() const {
    Diagnostic problems = Diagnostic::error(ErrorCode::schema_violation, "document validation failed");

    if (manifest.document_id.is_nil()) {
        problems.with_note(Diagnostic::error(ErrorCode::schema_violation,
                                             "manifest.document_id must not be nil"));
    }
    if (manifest.units != "meter") {
        problems.with_note(Diagnostic::error(
            ErrorCode::schema_violation, "unsupported units '" + manifest.units + "' (expected 'meter')"));
    }
    if (manifest.up_axis != "Y") {
        problems.with_note(Diagnostic::error(ErrorCode::schema_violation,
                                             "unsupported up_axis '" + manifest.up_axis + "'"));
    }
    if (manifest.handedness != "right") {
        problems.with_note(Diagnostic::error(ErrorCode::schema_violation,
                                             "unsupported handedness '" + manifest.handedness + "'"));
    }

    std::set<Uuid> ids;
    std::size_t root_count = 0;
    for (const auto& generator : generators) {
        const std::string label = generator.type + " '" + generator.name + "' (" +
                                  generator.id.str() + ")";
        if (generator.id.is_nil()) {
            problems.with_note(
                Diagnostic::error(ErrorCode::schema_violation, "generator with nil id: " + label));
            continue;
        }
        if (!ids.insert(generator.id).second) {
            problems.with_note(
                Diagnostic::error(ErrorCode::schema_violation, "duplicate generator id: " + label));
        }
        const auto* descriptor = find_generator_type(generator.type);
        if (descriptor == nullptr) {
            problems.with_note(Diagnostic::error(ErrorCode::graph_type_mismatch,
                                                 "unknown generator type: " + label));
            continue;
        }
        if (descriptor->is_root) {
            ++root_count;
            if (!generator.parent.is_nil()) {
                problems.with_note(Diagnostic::error(ErrorCode::graph_cardinality,
                                                     "root generator must not have a parent: " + label));
            }
        } else if (generator.parent.is_nil()) {
            problems.with_note(Diagnostic::error(ErrorCode::graph_cardinality,
                                                 "non-root generator requires a parent: " + label));
        }
    }
    if (root_count != 1) {
        problems.with_note(Diagnostic::error(
            ErrorCode::graph_cardinality,
            "document requires exactly one root generator, found " + std::to_string(root_count)));
    }

    // Parent references, type compatibility, and cycle detection.
    for (const auto& generator : generators) {
        if (generator.parent.is_nil()) {
            continue;
        }
        const std::string label = generator.type + " (" + generator.id.str() + ")";
        const auto* parent = find_generator(generator.parent);
        if (parent == nullptr) {
            problems.with_note(Diagnostic::error(ErrorCode::not_found,
                                                 "unknown parent " + generator.parent.str() +
                                                     " referenced by " + label));
            continue;
        }
        const auto* descriptor = find_generator_type(generator.type);
        if (descriptor != nullptr &&
            std::find(descriptor->allowed_parents.begin(), descriptor->allowed_parents.end(),
                      parent->type) == descriptor->allowed_parents.end()) {
            problems.with_note(Diagnostic::error(ErrorCode::graph_type_mismatch,
                                                 "generator " + label + " cannot attach to parent type '" +
                                                     parent->type + "'"));
        }
        // Walk toward the root; a repeat visit of this generator is a cycle.
        std::set<Uuid> seen{generator.id};
        const GeneratorInstance* cursor = parent;
        while (cursor != nullptr) {
            if (!seen.insert(cursor->id).second) {
                problems.with_note(Diagnostic::error(ErrorCode::graph_cycle,
                                                     "cycle through generator " + label));
                break;
            }
            cursor = cursor->parent.is_nil() ? nullptr : find_generator(cursor->parent);
        }
    }

    if (!problems.notes.empty()) {
        return problems;
    }
    return Ok{};
}

// --- serialization ---------------------------------------------------------

json::Value manifest_to_json(const Manifest& manifest) {
    json::Object object;
    object.emplace("format", std::string(kFormatName));
    object.emplace("schema_version", manifest.schema_version);
    object.emplace("document_id", manifest.document_id.str());
    object.emplace("name", manifest.name);
    object.emplace("units", manifest.units);
    object.emplace("up_axis", manifest.up_axis);
    object.emplace("handedness", manifest.handedness);
    object.emplace("seed", std::int64_t(manifest.seed));
    object.emplace("engine_algorithm_set", manifest.engine_algorithm_set);
    json::Object files;
    files.emplace("graph", "graph.json");
    files.emplace("properties", "properties.json");
    files.emplace("materials", "materials.json");
    object.emplace("files", std::move(files));
    return object;
}

namespace {

// UUID-sorted copies give canonical serialized order (ADR-0002: arrays keep
// semantic order; the semantic order of generator/material lists is by id).
std::vector<const GeneratorInstance*> sorted_generators(const Document& document) {
    std::vector<const GeneratorInstance*> out;
    out.reserve(document.generators.size());
    for (const auto& generator : document.generators) {
        out.push_back(&generator);
    }
    std::sort(out.begin(), out.end(), [](const GeneratorInstance* a, const GeneratorInstance* b) {
        return a->id < b->id;
    });
    return out;
}

} // namespace

json::Value graph_to_json(const Document& document) {
    json::Array nodes;
    for (const auto* generator : sorted_generators(document)) {
        json::Object node;
        node.emplace("id", generator->id.str());
        node.emplace("type", generator->type);
        node.emplace("name", generator->name);
        node.emplace("enabled", generator->enabled);
        node.emplace("parent", generator->parent.is_nil() ? json::Value(nullptr)
                                                          : json::Value(generator->parent.str()));
        nodes.push_back(std::move(node));
    }
    json::Object root;
    root.emplace("nodes", std::move(nodes));
    return root;
}

json::Value properties_to_json(const Document& document) {
    json::Object by_generator;
    for (const auto* generator : sorted_generators(document)) {
        if (generator->properties.empty()) {
            continue;
        }
        json::Object values;
        for (const auto& [key, value] : generator->properties) {
            values.emplace(key, value);
        }
        by_generator.emplace(generator->id.str(), std::move(values));
    }
    json::Object root;
    root.emplace("generators", std::move(by_generator));
    return root;
}

json::Value materials_to_json(const Document& document) {
    std::vector<const Material*> sorted;
    sorted.reserve(document.materials.size());
    for (const auto& material : document.materials) {
        sorted.push_back(&material);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const Material* a, const Material* b) { return a->id < b->id; });
    json::Array materials;
    for (const auto* material : sorted) {
        json::Object object;
        object.emplace("id", material->id.str());
        object.emplace("name", material->name);
        materials.push_back(std::move(object));
    }
    json::Object root;
    root.emplace("materials", std::move(materials));
    return root;
}

namespace {

Diagnostic schema_error(std::string message) {
    return Diagnostic::error(ErrorCode::schema_violation, std::move(message));
}

Result<std::string> require_string(const json::Value& object, std::string_view key,
                                   std::string_view context) {
    const auto* value = object.find(key);
    if (value == nullptr || !value->is_string()) {
        return schema_error(std::string(context) + ": missing or non-string '" + std::string(key) +
                            "'");
    }
    return value->as_string();
}

Result<Uuid> require_uuid(const json::Value& object, std::string_view key,
                          std::string_view context) {
    auto text = require_string(object, key, context);
    if (!text.ok()) {
        return text.take_error();
    }
    auto parsed = Uuid::parse(text.value());
    if (!parsed) {
        return schema_error(std::string(context) + ": malformed UUID in '" + std::string(key) +
                            "': " + text.value());
    }
    return *parsed;
}

} // namespace

Result<Document> document_from_json(const json::Value& manifest_json, const json::Value& graph_json,
                                    const json::Value& properties_json,
                                    const json::Value& materials_json) {
    Document document;

    if (!manifest_json.is_object()) {
        return schema_error("manifest: not a JSON object");
    }
    auto format = require_string(manifest_json, "format", "manifest");
    if (!format.ok()) {
        return format.take_error();
    }
    if (format.value() != kFormatName) {
        return schema_error("manifest: unknown format '" + format.value() + "'");
    }
    auto schema_version = require_string(manifest_json, "schema_version", "manifest");
    if (!schema_version.ok()) {
        return schema_version.take_error();
    }
    if (schema_version.value() != kSchemaVersion) {
        return Diagnostic::error(ErrorCode::unsupported_version,
                                 "manifest: unsupported schema_version '" + schema_version.value() +
                                     "' (reader supports " + std::string(kSchemaVersion) + ")");
    }
    document.manifest.schema_version = schema_version.value();
    auto document_id = require_uuid(manifest_json, "document_id", "manifest");
    if (!document_id.ok()) {
        return document_id.take_error();
    }
    document.manifest.document_id = document_id.value();
    if (const auto* name = manifest_json.find("name"); name != nullptr && name->is_string()) {
        document.manifest.name = name->as_string();
    }
    const std::array<std::pair<std::string_view, std::string*>, 4> string_fields = {{
        {"units", &document.manifest.units},
        {"up_axis", &document.manifest.up_axis},
        {"handedness", &document.manifest.handedness},
        {"engine_algorithm_set", &document.manifest.engine_algorithm_set},
    }};
    for (const auto& [key, member] : string_fields) {
        if (const auto* value = manifest_json.find(key); value != nullptr) {
            if (!value->is_string()) {
                return schema_error("manifest: non-string '" + std::string(key) + "'");
            }
            *member = value->as_string();
        }
    }
    if (const auto* seed = manifest_json.find("seed"); seed != nullptr) {
        if (!seed->is_int() || seed->as_int() < 0) {
            return schema_error("manifest: 'seed' must be a non-negative integer");
        }
        document.manifest.seed = std::uint64_t(seed->as_int());
    }

    const auto* nodes = graph_json.find("nodes");
    if (nodes == nullptr || !nodes->is_array()) {
        return schema_error("graph: missing 'nodes' array");
    }
    for (std::size_t i = 0; i < nodes->as_array().size(); ++i) {
        const auto& node = nodes->as_array()[i];
        const std::string context = "graph.nodes[" + std::to_string(i) + "]";
        if (!node.is_object()) {
            return schema_error(context + ": not an object");
        }
        GeneratorInstance generator;
        auto id = require_uuid(node, "id", context);
        if (!id.ok()) {
            return id.take_error();
        }
        generator.id = id.value();
        auto type = require_string(node, "type", context);
        if (!type.ok()) {
            return type.take_error();
        }
        generator.type = type.value();
        auto name = require_string(node, "name", context);
        if (!name.ok()) {
            return name.take_error();
        }
        generator.name = name.value();
        if (const auto* enabled = node.find("enabled"); enabled != nullptr) {
            if (!enabled->is_bool()) {
                return schema_error(context + ": non-boolean 'enabled'");
            }
            generator.enabled = enabled->as_bool();
        }
        const auto* parent = node.find("parent");
        if (parent == nullptr) {
            return schema_error(context + ": missing 'parent' (null for the root)");
        }
        if (!parent->is_null()) {
            if (!parent->is_string()) {
                return schema_error(context + ": 'parent' must be null or a UUID string");
            }
            auto parsed = Uuid::parse(parent->as_string());
            if (!parsed) {
                return schema_error(context + ": malformed parent UUID");
            }
            generator.parent = *parsed;
        }
        document.generators.push_back(std::move(generator));
    }

    if (const auto* by_generator = properties_json.find("generators"); by_generator != nullptr) {
        if (!by_generator->is_object()) {
            return schema_error("properties: 'generators' must be an object");
        }
        for (const auto& [id_text, values] : by_generator->as_object()) {
            auto parsed = Uuid::parse(id_text);
            if (!parsed) {
                return schema_error("properties: malformed generator UUID key '" + id_text + "'");
            }
            GeneratorInstance* target = nullptr;
            for (auto& generator : document.generators) {
                if (generator.id == *parsed) {
                    target = &generator;
                    break;
                }
            }
            if (target == nullptr) {
                return schema_error("properties: values for unknown generator " + id_text);
            }
            if (!values.is_object()) {
                return schema_error("properties: values for " + id_text + " must be an object");
            }
            for (const auto& [key, value] : values.as_object()) {
                target->properties.emplace(key, value);
            }
        }
    }

    if (const auto* materials = materials_json.find("materials"); materials != nullptr) {
        if (!materials->is_array()) {
            return schema_error("materials: 'materials' must be an array");
        }
        for (std::size_t i = 0; i < materials->as_array().size(); ++i) {
            const auto& entry = materials->as_array()[i];
            const std::string context = "materials[" + std::to_string(i) + "]";
            if (!entry.is_object()) {
                return schema_error(context + ": not an object");
            }
            Material material;
            auto id = require_uuid(entry, "id", context);
            if (!id.ok()) {
                return id.take_error();
            }
            material.id = id.value();
            auto name = require_string(entry, "name", context);
            if (!name.ok()) {
                return name.take_error();
            }
            material.name = name.value();
            document.materials.push_back(std::move(material));
        }
    }

    if (auto valid = document.validate(); !valid.ok()) {
        return valid.take_error();
    }
    return document;
}

} // namespace canopy::doc
