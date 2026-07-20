#include "canopy/document/registry.hpp"

#include <array>

namespace canopy::doc {

namespace {

const std::array<GeneratorTypeDescriptor, 2>& registry() {
    static const std::array<GeneratorTypeDescriptor, 2> kTypes = {
        GeneratorTypeDescriptor{"canopy.tree", "Tree", true, {}},
        GeneratorTypeDescriptor{"canopy.branch", "Branch", false, {"canopy.tree", "canopy.branch"}},
    };
    return kTypes;
}

} // namespace

const GeneratorTypeDescriptor* find_generator_type(std::string_view type) {
    for (const auto& descriptor : registry()) {
        if (descriptor.type == type) {
            return &descriptor;
        }
    }
    return nullptr;
}

} // namespace canopy::doc
