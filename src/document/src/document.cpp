#include "canopy/document/document.hpp"

#include "canopy/document/registry.hpp"

#include <algorithm>
#include <array>
#include <charconv>
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
        json::Array color;
        for (const double channel : material->base_color) {
            color.push_back(channel);
        }
        object.emplace("base_color", std::move(color));
        object.emplace("two_sided", material->two_sided);
        if (material->season_color.has_value()) {
            json::Array season;
            for (const double channel : *material->season_color) {
                season.push_back(channel);
            }
            object.emplace("season_color", std::move(season));
        }
        if (material->textures.has_value()) {
            json::Object textures;
            if (!material->textures->base_color.empty()) {
                textures.emplace("base_color", material->textures->base_color);
            }
            if (!material->textures->normal.empty()) {
                textures.emplace("normal", material->textures->normal);
            }
            object.emplace("textures", std::move(textures));
        }
        if (material->card_region.has_value()) {
            json::Array region;
            for (const double edge : *material->card_region) {
                region.push_back(edge);
            }
            object.emplace("card_region", std::move(region));
        }
        if (material->cutout.has_value()) {
            json::Object cutout;
            json::Array stem;
            stem.push_back(material->cutout->stem[0]);
            stem.push_back(material->cutout->stem[1]);
            cutout.emplace("stem", std::move(stem));
            json::Array vertices;
            for (const auto& vertex : material->cutout->vertices) {
                json::Array pair;
                pair.push_back(vertex[0]);
                pair.push_back(vertex[1]);
                vertices.push_back(std::move(pair));
            }
            cutout.emplace("vertices", std::move(vertices));
            object.emplace("cutout", std::move(cutout));
        }
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
    // ADR-0004: accept any 1.x — minor versions are additive-with-defaults
    // and unknown fields are ignored. Reject other majors.
    {
        const std::string& text = schema_version.value();
        int major = 0;
        const auto result = std::from_chars(text.data(), text.data() + text.size(), major);
        if (result.ec != std::errc() || result.ptr == text.data() || *result.ptr != '.' ||
            major != kSupportedSchemaMajor) {
            return Diagnostic::error(ErrorCode::unsupported_version,
                                     "manifest: unsupported schema_version '" + text +
                                         "' (reader supports " +
                                         std::to_string(kSupportedSchemaMajor) + ".x)");
        }
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
            if (const auto* color = entry.find("base_color"); color != nullptr) {
                if (!color->is_array() || color->as_array().size() != 4) {
                    return schema_error(context + ": 'base_color' must be [r, g, b, a]");
                }
                for (std::size_t c = 0; c < 4; ++c) {
                    const auto& channel = color->as_array()[c];
                    if (!channel.is_number()) {
                        return schema_error(context + ": non-numeric base_color channel");
                    }
                    material.base_color[c] = channel.as_number();
                }
            }
            if (const auto* season = entry.find("season_color"); season != nullptr) {
                if (!season->is_array() || season->as_array().size() != 4) {
                    return schema_error(context + ": 'season_color' must be [r, g, b, a]");
                }
                std::array<double, 4> season_color{};
                for (std::size_t c = 0; c < 4; ++c) {
                    const auto& channel = season->as_array()[c];
                    if (!channel.is_number()) {
                        return schema_error(context + ": non-numeric season_color channel");
                    }
                    season_color[c] = channel.as_number();
                }
                material.season_color = season_color;
            }
            if (const auto* two_sided = entry.find("two_sided"); two_sided != nullptr) {
                if (!two_sided->is_bool()) {
                    return schema_error(context + ": 'two_sided' must be a boolean");
                }
                material.two_sided = two_sided->as_bool();
            }
            if (const auto* textures = entry.find("textures"); textures != nullptr) {
                if (!textures->is_object()) {
                    return schema_error(context + ": 'textures' must be an object");
                }
                MaterialTextures parsed_textures;
                if (const auto* base = textures->find("base_color");
                    base != nullptr && base->is_string()) {
                    parsed_textures.base_color = base->as_string();
                }
                if (const auto* normal = textures->find("normal");
                    normal != nullptr && normal->is_string()) {
                    parsed_textures.normal = normal->as_string();
                }
                // Reject path traversal outright: URIs are project-relative.
                for (const std::string* uri :
                     {&parsed_textures.base_color, &parsed_textures.normal}) {
                    if (uri->find("..") != std::string::npos ||
                        (!uri->empty() && uri->front() == '/')) {
                        return schema_error(context + ": texture URIs must be project-relative");
                    }
                }
                material.textures = std::move(parsed_textures);
            }
            if (const auto* region = entry.find("card_region"); region != nullptr) {
                if (!region->is_array() || region->as_array().size() != 4) {
                    return schema_error(context + ": 'card_region' must be [u0, v0, u1, v1]");
                }
                std::array<double, 4> card_region{};
                for (std::size_t c = 0; c < 4; ++c) {
                    if (!region->as_array()[c].is_number()) {
                        return schema_error(context + ": non-numeric card_region edge");
                    }
                    card_region[c] = region->as_array()[c].as_number();
                }
                if (!(card_region[2] > card_region[0]) || !(card_region[3] > card_region[1])) {
                    return schema_error(context + ": card_region must have positive extent");
                }
                material.card_region = card_region;
            }
            if (const auto* cutout = entry.find("cutout"); cutout != nullptr) {
                if (!cutout->is_object()) {
                    return schema_error(context + ": 'cutout' must be an object");
                }
                MaterialCutout parsed_cutout;
                if (const auto* stem = cutout->find("stem"); stem != nullptr) {
                    if (!stem->is_array() || stem->as_array().size() != 2 ||
                        !stem->as_array()[0].is_number() || !stem->as_array()[1].is_number()) {
                        return schema_error(context + ": cutout 'stem' must be [x, y]");
                    }
                    parsed_cutout.stem = {stem->as_array()[0].as_number(),
                                          stem->as_array()[1].as_number()};
                }
                const auto* vertices = cutout->find("vertices");
                if (vertices == nullptr || !vertices->is_array() ||
                    vertices->as_array().size() < 3) {
                    return schema_error(context + ": cutout requires >= 3 'vertices'");
                }
                for (const auto& vertex : vertices->as_array()) {
                    if (!vertex.is_array() || vertex.as_array().size() != 2 ||
                        !vertex.as_array()[0].is_number() || !vertex.as_array()[1].is_number()) {
                        return schema_error(context + ": cutout vertices must be [x, y] pairs");
                    }
                    parsed_cutout.vertices.push_back(
                        {vertex.as_array()[0].as_number(), vertex.as_array()[1].as_number()});
                }
                material.cutout = std::move(parsed_cutout);
            }
            document.materials.push_back(std::move(material));
        }
    }

    if (auto valid = document.validate(); !valid.ok()) {
        return valid.take_error();
    }
    return document;
}

} // namespace canopy::doc
