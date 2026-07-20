// CPU forest slice (17_RUNTIME_SDK_AND_FOREST.md, ADR-0008): deterministic
// instance scatter, distance-band LOD selection, distance culling. Depends on
// runtime core + foundation only. Frustum culling, GPU expansion and
// streaming arrive in later epics.
#pragma once

#include "canopy/foundation/vec.hpp"
#include "canopy/runtime/model.hpp"

#include <cstdint>
#include <vector>

namespace canopy::rt {

struct ForestInstance {
    std::uint32_t id = 0;
    std::uint32_t model = 0; // index into the forest's model list
    Vec3 position;
    double yaw = 0.0;   // radians around +Y
    double scale = 1.0; // uniform
};

struct VisibleInstance {
    const ForestInstance* instance = nullptr;
    std::uint32_t lod = 0;
};

class Forest {
public:
    // Models must outlive the forest; they are immutable and shared.
    void add_model(const RtModel* model) { models_.push_back(model); }
    const std::vector<const RtModel*>& models() const { return models_; }
    const std::vector<ForestInstance>& instances() const { return instances_; }

    // Deterministic scatter: uniform positions in a square of ±extent meters
    // around the origin, minimum spacing by rejection (bounded attempts, so
    // dense requests degrade to fewer instances rather than hanging), yaw
    // 0..2π, scale 0.85..1.15, model chosen uniformly. Same seed → same
    // forest on every platform.
    Result<void> scatter(std::uint64_t seed, std::uint32_t count, double extent,
                         double min_spacing);

    // Distance-band LOD: bands scale with the model's bounding radius
    // (< 8r → LOD 0, < 20r → LOD 1, else deepest LOD), clamped to the
    // model's LOD count. Instances beyond max_distance are culled. Output
    // order is instance order (deterministic).
    std::vector<VisibleInstance> select(const Vec3& camera, double max_distance) const;

private:
    std::vector<const RtModel*> models_;
    std::vector<ForestInstance> instances_;
};

} // namespace canopy::rt
