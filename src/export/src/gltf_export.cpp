// GLB writer per the public glTF 2.0 specification (Khronos, open standard).
#include "canopy/export/gltf_export.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <limits>
#include <map>

namespace canopy::exp {

namespace {

Diagnostic io_error(const std::filesystem::path& path, std::string message) {
    return Diagnostic::error(ErrorCode::io_error, std::move(message),
                             SourceLocation{path.string(), 0, 0});
}

void put_u32(std::string& out, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        out.push_back(char((value >> (8 * i)) & 0xffu));
    }
}

void put_f32(std::string& out, double value) {
    const float f = float(value);
    std::array<char, 4> bytes{};
    std::memcpy(bytes.data(), &f, 4);
    out.append(bytes.data(), 4);
    // Little-endian assumption is checked once in write_glb.
}

// One primitive per material: geometry merged from nodes in semantic order.
struct PrimitiveData {
    std::string material_name;
    std::array<double, 4> color{0.6, 0.5, 0.4, 1.0};
    bool double_sided = false;
    std::vector<float> positions; // xyz
    std::vector<float> normals;   // xyz
    std::vector<float> uvs;       // uv
    std::vector<std::uint32_t> indices;
    std::array<double, 3> min_position{1e30, 1e30, 1e30};
    std::array<double, 3> max_position{-1e30, -1e30, -1e30};
};

} // namespace

Result<GltfManifest> write_glb(const doc::Document& document, const eval::EvaluatedModel& model,
                               const ExportPreset& preset,
                               const std::filesystem::path& out_base) {
    static_assert(std::numeric_limits<float>::is_iec559, "IEEE-754 float required");
    const std::uint32_t endian_probe = 1;
    if (*reinterpret_cast<const std::uint8_t*>(&endian_probe) != 1) {
        return Diagnostic::error(ErrorCode::internal_error,
                                 "GLB writer requires a little-endian host");
    }

    // Season blend mirrors the OBJ/MTL path (ADR-0006).
    const double season = model.sample.season;
    double blend = std::clamp((season - 0.5) / 0.35, 0.0, 1.0);
    blend = blend * blend * (3.0 - 2.0 * blend);

    // Group nodes by material UUID (nil → default), materials in UUID order.
    std::map<Uuid, PrimitiveData> primitives;
    for (const auto& node : model.nodes) {
        auto [it, inserted] = primitives.try_emplace(node.material_id);
        PrimitiveData& primitive = it->second;
        if (inserted) {
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
        const auto base = std::uint32_t(primitive.positions.size() / 3);
        for (std::size_t i = 0; i < node.mesh.positions.size(); ++i) {
            const Vec3& p = node.mesh.positions[i];
            const Vec3& n = node.mesh.normals[i];
            const Vec2& uv = node.mesh.uvs[i];
            primitive.positions.insert(primitive.positions.end(),
                                       {float(p.x), float(p.y), float(p.z)});
            const Vec3 unit = normalize_or(n, Vec3{0.0, 1.0, 0.0});
            primitive.normals.insert(primitive.normals.end(),
                                     {float(unit.x), float(unit.y), float(unit.z)});
            primitive.uvs.insert(primitive.uvs.end(), {float(uv.x), float(uv.y)});
            for (int c = 0; c < 3; ++c) {
                const double v = c == 0 ? p.x : (c == 1 ? p.y : p.z);
                primitive.min_position[std::size_t(c)] =
                    std::min(primitive.min_position[std::size_t(c)], v);
                primitive.max_position[std::size_t(c)] =
                    std::max(primitive.max_position[std::size_t(c)], v);
            }
        }
        for (const std::uint32_t index : node.mesh.indices) {
            primitive.indices.push_back(base + index);
        }
    }
    if (primitives.empty()) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "cannot export an empty model to glTF");
    }

    // --- binary buffer: per primitive, positions/normals/uvs/indices -------
    std::string bin;
    struct ViewRange {
        std::uint32_t offset = 0;
        std::uint32_t length = 0;
    };
    struct PrimitiveViews {
        ViewRange positions, normals, uvs, indices;
        std::uint32_t vertex_count = 0;
        std::uint32_t index_count = 0;
    };
    std::vector<PrimitiveViews> views;
    auto append_floats = [&bin](const std::vector<float>& values) {
        ViewRange range;
        range.offset = std::uint32_t(bin.size());
        for (const float value : values) {
            put_f32(bin, double(value));
        }
        range.length = std::uint32_t(bin.size()) - range.offset;
        return range;
    };
    for (const auto& [id, primitive] : primitives) {
        PrimitiveViews view;
        view.vertex_count = std::uint32_t(primitive.positions.size() / 3);
        view.index_count = std::uint32_t(primitive.indices.size());
        view.positions = append_floats(primitive.positions);
        view.normals = append_floats(primitive.normals);
        view.uvs = append_floats(primitive.uvs);
        view.indices.offset = std::uint32_t(bin.size());
        for (const std::uint32_t index : primitive.indices) {
            put_u32(bin, index);
        }
        view.indices.length = std::uint32_t(bin.size()) - view.indices.offset;
        views.push_back(view);
    }
    while (bin.size() % 4 != 0) {
        bin.push_back('\0');
    }

    // --- JSON chunk ---------------------------------------------------------
    json::Array buffer_views;
    json::Array accessors;
    json::Array gltf_materials;
    json::Array primitive_array;
    std::size_t view_index = 0;
    std::size_t accessor_index = 0;
    std::size_t material_index = 0;
    std::size_t total_vertices = 0;
    std::size_t total_indices = 0;
    auto add_view = [&buffer_views](const ViewRange& range) {
        json::Object view;
        view.emplace("buffer", 0);
        view.emplace("byteOffset", std::int64_t(range.offset));
        view.emplace("byteLength", std::int64_t(range.length));
        buffer_views.push_back(std::move(view));
    };
    auto primitive_it = primitives.begin();
    for (const PrimitiveViews& view : views) {
        const PrimitiveData& primitive = primitive_it->second;
        ++primitive_it;
        total_vertices += view.vertex_count;
        total_indices += view.index_count;

        add_view(view.positions);
        add_view(view.normals);
        add_view(view.uvs);
        add_view(view.indices);

        auto float_accessor = [&](std::size_t view_slot, std::uint32_t count,
                                  const char* type, bool with_bounds) {
            json::Object accessor;
            accessor.emplace("bufferView", std::int64_t(view_slot));
            accessor.emplace("componentType", 5126); // FLOAT
            accessor.emplace("count", std::int64_t(count));
            accessor.emplace("type", type);
            if (with_bounds) {
                json::Array minimum;
                json::Array maximum;
                for (std::size_t c = 0; c < 3; ++c) {
                    minimum.push_back(double(float(primitive.min_position[c])));
                    maximum.push_back(double(float(primitive.max_position[c])));
                }
                accessor.emplace("min", std::move(minimum));
                accessor.emplace("max", std::move(maximum));
            }
            accessors.push_back(std::move(accessor));
            return accessor_index++;
        };
        const std::size_t position_accessor =
            float_accessor(view_index + 0, view.vertex_count, "VEC3", true);
        const std::size_t normal_accessor =
            float_accessor(view_index + 1, view.vertex_count, "VEC3", false);
        const std::size_t uv_accessor =
            float_accessor(view_index + 2, view.vertex_count, "VEC2", false);
        json::Object index_accessor;
        index_accessor.emplace("bufferView", std::int64_t(view_index + 3));
        index_accessor.emplace("componentType", 5125); // UNSIGNED_INT
        index_accessor.emplace("count", std::int64_t(view.index_count));
        index_accessor.emplace("type", "SCALAR");
        accessors.push_back(std::move(index_accessor));
        const std::size_t index_accessor_slot = accessor_index++;
        view_index += 4;

        json::Object pbr;
        json::Array color_factor;
        for (const double channel : primitive.color) {
            color_factor.push_back(channel);
        }
        pbr.emplace("baseColorFactor", std::move(color_factor));
        pbr.emplace("metallicFactor", 0);
        pbr.emplace("roughnessFactor", 0.9);
        json::Object material;
        material.emplace("name", primitive.material_name);
        material.emplace("pbrMetallicRoughness", std::move(pbr));
        material.emplace("doubleSided", primitive.double_sided);
        if (primitive.color[3] < 1.0) {
            material.emplace("alphaMode", "BLEND");
        }
        gltf_materials.push_back(std::move(material));

        json::Object attributes;
        attributes.emplace("NORMAL", std::int64_t(normal_accessor));
        attributes.emplace("POSITION", std::int64_t(position_accessor));
        attributes.emplace("TEXCOORD_0", std::int64_t(uv_accessor));
        json::Object primitive_json;
        primitive_json.emplace("attributes", std::move(attributes));
        primitive_json.emplace("indices", std::int64_t(index_accessor_slot));
        primitive_json.emplace("material", std::int64_t(material_index++));
        primitive_array.push_back(std::move(primitive_json));
    }

    json::Object root;
    {
        json::Object asset;
        asset.emplace("generator", "canopy-cli");
        asset.emplace("version", "2.0");
        root.emplace("asset", std::move(asset));
    }
    {
        json::Array buffers;
        json::Object buffer;
        buffer.emplace("byteLength", std::int64_t(bin.size()));
        buffers.push_back(std::move(buffer));
        root.emplace("buffers", std::move(buffers));
    }
    root.emplace("bufferViews", std::move(buffer_views));
    root.emplace("accessors", std::move(accessors));
    root.emplace("materials", std::move(gltf_materials));
    {
        json::Object mesh;
        mesh.emplace("name", document.manifest.name);
        mesh.emplace("primitives", std::move(primitive_array));
        json::Array meshes;
        meshes.push_back(std::move(mesh));
        root.emplace("meshes", std::move(meshes));
    }
    {
        json::Object node;
        node.emplace("mesh", 0);
        node.emplace("name", document.manifest.name);
        json::Array nodes;
        nodes.push_back(std::move(node));
        root.emplace("nodes", std::move(nodes));
        json::Object scene;
        json::Array scene_nodes;
        scene_nodes.push_back(0);
        scene.emplace("nodes", std::move(scene_nodes));
        json::Array scenes;
        scenes.push_back(std::move(scene));
        root.emplace("scenes", std::move(scenes));
        root.emplace("scene", 0);
    }

    auto json_text = json::write_canonical(root);
    if (!json_text.ok()) {
        return json_text.take_error();
    }
    std::string json_chunk = std::move(json_text).value();
    // The canonical writer appends a newline; keep it, pad with spaces per spec.
    while (json_chunk.size() % 4 != 0) {
        json_chunk.push_back(' ');
    }

    // --- container ----------------------------------------------------------
    std::string glb;
    glb.reserve(12 + 8 + json_chunk.size() + 8 + bin.size());
    glb += "glTF";
    put_u32(glb, 2);
    put_u32(glb, std::uint32_t(12 + 8 + json_chunk.size() + 8 + bin.size()));
    put_u32(glb, std::uint32_t(json_chunk.size()));
    glb += "JSON";
    glb += json_chunk;
    put_u32(glb, std::uint32_t(bin.size()));
    glb.push_back('B');
    glb.push_back('I');
    glb.push_back('N');
    glb.push_back('\0');
    glb += bin;

    std::error_code fs_error;
    const auto parent_dir = out_base.parent_path();
    if (!parent_dir.empty()) {
        std::filesystem::create_directories(parent_dir, fs_error);
        if (fs_error) {
            return io_error(parent_dir, "cannot create output directory: " + fs_error.message());
        }
    }
    const std::filesystem::path glb_path = out_base.string() + ".glb";
    {
        std::ofstream stream(glb_path, std::ios::binary | std::ios::trunc);
        if (!stream) {
            return io_error(glb_path, "cannot open file for writing");
        }
        stream.write(glb.data(), std::streamsize(glb.size()));
        stream.flush();
        if (!stream) {
            return io_error(glb_path, "write failure");
        }
    }

    GltfManifest manifest;
    manifest.glb_file = glb_path.filename().string();
    manifest.glb_sha256 = sha256(glb);
    manifest.model_hash = model.model_hash();
    manifest.primitive_count = primitives.size();
    manifest.vertex_count = total_vertices;
    manifest.triangle_count = total_indices / 3;

    json::Object manifest_json;
    manifest_json.emplace("format", "canopy-export-manifest");
    manifest_json.emplace("schema_version", "1.0.0");
    manifest_json.emplace("document_id", document.manifest.document_id.str());
    manifest_json.emplace("export_format", "gltf");
    manifest_json.emplace("granularity", "material");
    manifest_json.emplace("precision", "float32");
    manifest_json.emplace("profile", preset.profile);
    manifest_json.emplace("glb_file", manifest.glb_file);
    manifest_json.emplace("glb_sha256", manifest.glb_sha256.hex());
    manifest_json.emplace("model_hash", SemanticId{manifest.model_hash}.str());
    manifest_json.emplace("primitive_count", std::int64_t(manifest.primitive_count));
    manifest_json.emplace("vertex_count", std::int64_t(manifest.vertex_count));
    manifest_json.emplace("triangle_count", std::int64_t(manifest.triangle_count));
    auto manifest_bytes = json::write_canonical(manifest_json);
    if (!manifest_bytes.ok()) {
        return manifest_bytes.take_error();
    }
    const std::filesystem::path manifest_path = out_base.string() + ".manifest.json";
    std::ofstream stream(manifest_path, std::ios::binary | std::ios::trunc);
    if (!stream) {
        return io_error(manifest_path, "cannot open file for writing");
    }
    stream.write(manifest_bytes.value().data(), std::streamsize(manifest_bytes.value().size()));
    if (!stream) {
        return io_error(manifest_path, "write failure");
    }
    return manifest;
}

} // namespace canopy::exp
