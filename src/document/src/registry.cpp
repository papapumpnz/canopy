#include "canopy/document/registry.hpp"

#include <array>

namespace canopy::doc {

namespace {

const std::array<GeneratorTypeDescriptor, 5>& registry() {
    static const std::array<GeneratorTypeDescriptor, 5> kTypes = {
        GeneratorTypeDescriptor{"canopy.tree", "Tree", true, {}},
        GeneratorTypeDescriptor{"canopy.branch", "Branch", false, {"canopy.tree", "canopy.branch"}},
        GeneratorTypeDescriptor{"canopy.batched_leaf", "Batched Leaf", false, {"canopy.branch"}},
        GeneratorTypeDescriptor{"canopy.leaf_mesh", "Leaf Mesh", false, {"canopy.branch"}},
        GeneratorTypeDescriptor{"canopy.frond", "Frond", false, {"canopy.branch"}},
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
