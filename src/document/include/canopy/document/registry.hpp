// Generator type registry, bootstrap subset (backlog B-012; contract shape
// from 07_EVALUATION_ENGINE.md "Generator contract"). The full descriptor
// (properties, stages, capabilities) grows in later epics; the document layer
// needs parent/child compatibility and cardinality for validation.
#pragma once

#include <optional>
#include <string_view>
#include <vector>

namespace canopy::doc {

struct GeneratorTypeDescriptor {
    std::string_view type;         // reverse-domain, e.g. "canopy.branch"
    std::string_view display_name;
    bool is_root = false;          // may (and must) appear exactly once, parentless
    // Allowed parent types; empty means "any non-root placement is invalid".
    std::vector<std::string_view> allowed_parents;
};

// Bootstrap registry: canopy.tree (root) and canopy.branch (child of tree or
// branch). Extended in later epics; unknown types are preserved in documents
// but fail validation until registered.
const GeneratorTypeDescriptor* find_generator_type(std::string_view type);

} // namespace canopy::doc
