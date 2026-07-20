#include "canopy/export/mesh_merge.hpp"

#include "canopy/foundation/random.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <numbers>

namespace canopy::exp {

namespace {

// Per-primitive tangent generation: accumulate UV-gradient tangents per
// triangle, then Gram-Schmidt against the normal; degenerate UVs fall back
// to any basis perpendicular to the normal. Deterministic.
void generate_tangents(MergedPrimitive& primitive) {
    const std::size_t vertex_count = primitive.vertex_count();
    std::vector<double> accumulated_t(vertex_count * 3, 0.0);
    std::vector<double> accumulated_b(vertex_count * 3, 0.0);
    for (std::size_t t = 0; t + 2 < primitive.indices.size(); t += 3) {
        const std::uint32_t ia = primitive.indices[t];
        const std::uint32_t ib = primitive.indices[t + 1];
        const std::uint32_t ic = primitive.indices[t + 2];
        auto position = [&](std::uint32_t v, int c) {
            return double(primitive.positions[v * 3 + std::size_t(c)]);
        };
        auto uv = [&](std::uint32_t v, int c) {
            return double(primitive.uvs[v * 2 + std::size_t(c)]);
        };
        const double e1[3] = {position(ib, 0) - position(ia, 0),
                              position(ib, 1) - position(ia, 1),
                              position(ib, 2) - position(ia, 2)};
        const double e2[3] = {position(ic, 0) - position(ia, 0),
                              position(ic, 1) - position(ia, 1),
                              position(ic, 2) - position(ia, 2)};
        const double du1 = uv(ib, 0) - uv(ia, 0);
        const double dv1 = uv(ib, 1) - uv(ia, 1);
        const double du2 = uv(ic, 0) - uv(ia, 0);
        const double dv2 = uv(ic, 1) - uv(ia, 1);
        const double denom = du1 * dv2 - du2 * dv1;
        if (std::fabs(denom) < 1e-12) {
            continue;
        }
        const double inverse = 1.0 / denom;
        for (int c = 0; c < 3; ++c) {
            const double tangent = (dv2 * e1[c] - dv1 * e2[c]) * inverse;
            const double bitangent = (du1 * e2[c] - du2 * e1[c]) * inverse;
            for (const std::uint32_t v : {ia, ib, ic}) {
                accumulated_t[v * 3 + std::size_t(c)] += tangent;
                accumulated_b[v * 3 + std::size_t(c)] += bitangent;
            }
        }
    }
    primitive.tangents.resize(vertex_count * 4);
    for (std::size_t v = 0; v < vertex_count; ++v) {
        const double n[3] = {double(primitive.normals[v * 3]),
                             double(primitive.normals[v * 3 + 1]),
                             double(primitive.normals[v * 3 + 2])};
        double t[3] = {accumulated_t[v * 3], accumulated_t[v * 3 + 1],
                       accumulated_t[v * 3 + 2]};
        const double n_dot_t = n[0] * t[0] + n[1] * t[1] + n[2] * t[2];
        for (int c = 0; c < 3; ++c) {
            t[std::size_t(c)] -= n[std::size_t(c)] * n_dot_t;
        }
        double length = std::sqrt(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]);
        if (length < 1e-9) {
            // Degenerate UVs: pick a stable perpendicular to the normal.
            t[0] = std::fabs(n[1]) < 0.9 ? -n[2] : 0.0;
            t[1] = std::fabs(n[1]) < 0.9 ? 0.0 : n[2];
            t[2] = std::fabs(n[1]) < 0.9 ? n[0] : -n[1];
            length = std::sqrt(t[0] * t[0] + t[1] * t[1] + t[2] * t[2]);
            if (length < 1e-9) {
                t[0] = 1.0;
                t[1] = 0.0;
                t[2] = 0.0;
                length = 1.0;
            }
        }
        const double cross_product[3] = {n[1] * t[2] - n[2] * t[1],
                                         n[2] * t[0] - n[0] * t[2],
                                         n[0] * t[1] - n[1] * t[0]};
        const double handedness = cross_product[0] * accumulated_b[v * 3] +
                                              cross_product[1] * accumulated_b[v * 3 + 1] +
                                              cross_product[2] * accumulated_b[v * 3 + 2] <
                                          0.0
                                      ? -1.0
                                      : 1.0;
        for (int c = 0; c < 3; ++c) {
            primitive.tangents[v * 4 + std::size_t(c)] = float(t[std::size_t(c)] / length);
        }
        primitive.tangents[v * 4 + 3] = float(handedness);
    }
}

} // namespace

std::vector<MergedPrimitive> merge_by_material(const doc::Document& document,
                                               const eval::EvaluatedModel& model) {
    // Season blend mirrors the MTL path (ADR-0006).
    const double season = model.sample.season;
    double blend = std::clamp((season - 0.5) / 0.35, 0.0, 1.0);
    blend = blend * blend * (3.0 - 2.0 * blend);

    // Foliage vertices carry their PARENT branch's wind parameters so shader
    // wind bends leaves with the branch they grow on (falloff amendment,
    // ADR-0009) — the same rule the authoring wind pass applies.
    std::map<SemanticId, const eval::BranchNodeGeometry*> nodes_by_id;
    for (const auto& node : model.nodes) {
        nodes_by_id.emplace(node.semantic_id, &node);
    }

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
                if (material->textures.has_value()) {
                    primitive.base_color_texture = material->textures->base_color;
                    primitive.normal_texture = material->textures->normal;
                }
                primitive.alpha_masked = material->card_region.has_value();
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
        // Wind channels (amplitude, phase, bend length, kind): mirror the
        // authoring falloff sway. Foliage points at its parent branch's
        // pivot/params; anchored (ground-clamped) nodes get zero amplitude.
        const eval::BranchNodeGeometry* wind_source = &node;
        if (node.kind == eval::NodeKind::foliage) {
            if (const auto parent_it = nodes_by_id.find(node.parent_semantic_id);
                parent_it != nodes_by_id.end()) {
                wind_source = parent_it->second;
            }
        }
        RandomStream phase_stream(derive_stream_key(document.manifest.seed,
                                                    wind_source->generator_id,
                                                    wind_source->semantic_id, "wind", "phase"));
        const double phase = phase_stream.uniform(0.0, 2.0 * std::numbers::pi);
        const double kind_gain = wind_source->kind == eval::NodeKind::frond ? 2.2 : 1.0;
        const double amplitude =
            (wind_source->anchored || wind_source->kind == eval::NodeKind::foliage)
                ? 0.0
                : kind_gain * (0.015 + 0.02 * double(wind_source->depth));
        const double bend_length = std::max(wind_source->length_m, 1e-3);
        for (std::size_t v = 0; v < node.mesh.positions.size(); ++v) {
            primitive.wind_anchor.insert(primitive.wind_anchor.end(),
                                         {float(wind_source->base_position.x),
                                          float(wind_source->base_position.y),
                                          float(wind_source->base_position.z)});
            primitive.wind_params.insert(primitive.wind_params.end(),
                                         {float(amplitude), float(phase),
                                          float(bend_length), float(int(node.kind))});
        }
    }

    std::vector<MergedPrimitive> out;
    out.reserve(primitives.size());
    for (auto& [id, primitive] : primitives) {
        generate_tangents(primitive);
        out.push_back(std::move(primitive));
    }
    return out;
}

} // namespace canopy::exp
