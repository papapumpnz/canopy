#include "canopy/export/mesh_merge.hpp"

#include <algorithm>
#include <map>

namespace canopy::exp {

std::vector<MergedPrimitive> merge_by_material(const doc::Document& document,
                                               const eval::EvaluatedModel& model) {
    // Season blend mirrors the MTL path (ADR-0006).
    const double season = model.sample.season;
    double blend = std::clamp((season - 0.5) / 0.35, 0.0, 1.0);
    blend = blend * blend * (3.0 - 2.0 * blend);

    std::map<Uuid, MergedPrimitive> primitives;
    for (const auto& node : model.nodes) {
        auto [it, inserted] = primitives.try_emplace(node.material_id);
        MergedPrimitive& primitive = it->second;
        if (inserted) {
            primitive.material_id = node.material_id;
            const auto* material = document.find_material(node.material_id);
            if (material != nullptr) {
                primitive.material_name =
                    material->name.empty() ? material->id.str() : material->name;
                primitive.color = material->base_color;
                if (material->season_color.has_value() && blend > 0.0) {
                    for (std::size_t c = 0; c < 4; ++c) {
                        primitive.color[c] +=
                            ((*material->season_color)[c] - primitive.color[c]) * blend;
                    }
                }
                primitive.double_sided = material->two_sided;
            } else {
                primitive.material_name = "canopy_default";
            }
        }
        const auto base = std::uint32_t(primitive.vertex_count());
        for (std::size_t i = 0; i < node.mesh.positions.size(); ++i) {
            const Vec3& p = node.mesh.positions[i];
            const Vec3 n = normalize_or(node.mesh.normals[i], Vec3{0.0, 1.0, 0.0});
            const Vec2& uv = node.mesh.uvs[i];
            primitive.positions.insert(primitive.positions.end(),
                                       {float(p.x), float(p.y), float(p.z)});
            primitive.normals.insert(primitive.normals.end(),
                                     {float(n.x), float(n.y), float(n.z)});
            primitive.uvs.insert(primitive.uvs.end(), {float(uv.x), float(uv.y)});
            const std::array<double, 3> components{p.x, p.y, p.z};
            for (std::size_t c = 0; c < 3; ++c) {
                primitive.min_position[c] = std::min(primitive.min_position[c], components[c]);
                primitive.max_position[c] = std::max(primitive.max_position[c], components[c]);
            }
        }
        for (const std::uint32_t index : node.mesh.indices) {
            primitive.indices.push_back(base + index);
        }
    }

    std::vector<MergedPrimitive> out;
    out.reserve(primitives.size());
    for (auto& [id, primitive] : primitives) {
        out.push_back(std::move(primitive));
    }
    return out;
}

} // namespace canopy::exp
