// Authoring document model, bootstrap subset (backlog B-010, B-012).
//
// A document is a manifest plus a typed generator DAG with per-generator
// property maps. Property values stay as canonical JSON values here; typed
// interpretation (units, curves) happens at evaluation using property
// descriptors. This keeps serialization loss-free for unknown extensions.
#pragma once

#include "canopy/foundation/diagnostics.hpp"
#include "canopy/foundation/ids.hpp"
#include "canopy/foundation/json.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace canopy::doc {

inline constexpr std::string_view kFormatName = "canopy-authoring";
inline constexpr std::string_view kSchemaVersion = "1.0.0";

struct Manifest {
    std::string schema_version{kSchemaVersion};
    Uuid document_id;
    std::string name;
    std::string units = "meter";
    std::string up_axis = "Y";
    std::string handedness = "right";
    std::uint64_t seed = 0;
    std::string engine_algorithm_set = "canopy-1";
};

// Generator type IDs are reverse-domain strings; the bootstrap registry knows
// "canopy.tree" and "canopy.branch".
struct GeneratorInstance {
    Uuid id;
    std::string type;
    std::string name;
    bool enabled = true;
    Uuid parent; // nil for the root generator
    // Property key → canonical JSON value, e.g. "spine.length.absolute": 4.0.
    std::map<std::string, json::Value, std::less<>> properties;
};

struct Material {
    Uuid id;
    std::string name;
};

struct Document {
    Manifest manifest;
    // Semantic order: parents precede children after validation; serialized
    // order is sorted by UUID for canonical output.
    std::vector<GeneratorInstance> generators;
    std::vector<Material> materials;

    const GeneratorInstance* find_generator(const Uuid& id) const;
    const Material* find_material(const Uuid& id) const;

    // Deterministic traversal order (backlog B-012): root first, children
    // sorted by UUID. Fails on cycles, unknown parents, unknown types,
    // cardinality violations, and type-compatibility violations.
    Result<std::vector<const GeneratorInstance*>> topological_order() const;

    // Full structural validation; collects all problems as notes on one
    // diagnostic rather than stopping at the first.
    Result<void> validate() const;
};

// Serialization to/from the canonical JSON documents that make up a project.
json::Value manifest_to_json(const Manifest& manifest);
json::Value graph_to_json(const Document& document);
json::Value properties_to_json(const Document& document);
json::Value materials_to_json(const Document& document);

Result<Document> document_from_json(const json::Value& manifest_json,
                                    const json::Value& graph_json,
                                    const json::Value& properties_json,
                                    const json::Value& materials_json);

} // namespace canopy::doc
