#include "canopy/runtime/forest.hpp"

#include "canopy/foundation/random.hpp"

#include <cmath>
#include <numbers>

namespace canopy::rt {

Result<void> Forest::scatter(std::uint64_t seed, std::uint32_t count, double extent,
                             double min_spacing) {
    if (models_.empty()) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "forest has no models to scatter");
    }
    if (!(extent > 0.0) || count > 100000) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "scatter requires positive extent and count <= 100000");
    }
    instances_.clear();
    RandomStream stream(StreamKey{seed});
    const double spacing_squared = min_spacing * min_spacing;
    std::uint32_t placed = 0;
    // Bounded attempts: dense requests degrade gracefully and deterministically.
    const std::uint64_t max_attempts = std::uint64_t(count) * 24 + 64;
    for (std::uint64_t attempt = 0; attempt < max_attempts && placed < count; ++attempt) {
        const double x = stream.uniform(-extent, extent);
        const double z = stream.uniform(-extent, extent);
        bool clear = true;
        if (min_spacing > 0.0) {
            for (const ForestInstance& existing : instances_) {
                const double dx = existing.position.x - x;
                const double dz = existing.position.z - z;
                if (dx * dx + dz * dz < spacing_squared) {
                    clear = false;
                    break;
                }
            }
        }
        // Draws below happen regardless of acceptance so the decision stream
        // for instance N does not depend on how many rejections preceded it.
        const double yaw = stream.uniform(0.0, 2.0 * std::numbers::pi);
        const double scale = stream.uniform(0.85, 1.15);
        const std::uint64_t model = stream.uniform_index(models_.size());
        if (!clear) {
            continue;
        }
        ForestInstance instance;
        instance.id = placed;
        instance.model = std::uint32_t(model);
        instance.position = Vec3{x, 0.0, z};
        instance.yaw = yaw;
        instance.scale = scale;
        instances_.push_back(instance);
        ++placed;
    }
    return Ok{};
}

std::vector<VisibleInstance> Forest::select(const Vec3& camera, double max_distance) const {
    std::vector<VisibleInstance> visible;
    visible.reserve(instances_.size());
    for (const ForestInstance& instance : instances_) {
        const Vec3 offset = instance.position - camera;
        const double distance = length(offset);
        if (max_distance > 0.0 && distance > max_distance) {
            continue;
        }
        const RtModel& model = *models_[instance.model];
        const double radius = std::max(model.bounds.radius() * instance.scale, 1e-3);
        std::uint32_t lod = 0;
        if (distance >= 8.0 * radius) {
            lod = 1;
        }
        if (distance >= 20.0 * radius) {
            lod = 2;
        }
        const auto deepest = std::uint32_t(model.lods.size() - 1);
        visible.push_back(VisibleInstance{&instance, lod > deepest ? deepest : lod});
    }
    return visible;
}

} // namespace canopy::rt
