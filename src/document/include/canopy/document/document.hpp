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

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace canopy::doc {

inline constexpr std::string_view kFormatName = "canopy-authoring";
// Written by this build. Readers accept any 1.x (ADR-0004: minor versions are
// additive-with-defaults; unknown fields are ignored). 1.2 adds material
// season_color (ADR-0006).
inline constexpr std::string_view kSchemaVersion = "1.2.0";
inline constexpr int kSupportedSchemaMajor = 1;

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

// Leaf cutout outline (ADR-0004): a simple polygon in normalized leaf space —
// y runs 0 (stem) to 1 (tip), x −0.5..0.5 across the blade.
struct MaterialCutout {
    std::vector<std::array<double, 2>> vertices;
    std::array<double, 2> stem{0.0, 0.0};
};

struct Material {
    Uuid id;
    std::string name;
    std::array<double, 4> base_color{0.5, 0.5, 0.5, 1.0}; // linear RGBA
    bool two_sided = false;
    std::optional<MaterialCutout> cutout;
    // Seasonal target color (ADR-0006); exporters blend base_color toward it
    // as the season transition advances.
    std::optional<std::array<double, 4>> season_color;
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
