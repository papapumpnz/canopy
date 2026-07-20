#include "canopy/export/obj_export.hpp"

#include <fstream>
#include <sstream>

namespace canopy::exp {

namespace {

Diagnostic io_error(const std::filesystem::path& path, std::string message) {
    return Diagnostic::error(ErrorCode::io_error, std::move(message),
                             SourceLocation{path.string(), 0, 0});
}

Result<void> write_file(const std::filesystem::path& path, std::string_view bytes) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) {
        return io_error(path, "cannot open file for writing");
    }
    stream.write(bytes.data(), std::streamsize(bytes.size()));
    stream.flush();
    if (!stream) {
        return io_error(path, "write failure");
    }
    return Ok{};
}

void append_number(std::string& out, double value) {
    out += json::format_double(value);
}

} // namespace

Result<ExportPreset> ExportPreset::load(const std::filesystem::path& preset_path) {
    std::ifstream stream(preset_path, std::ios::binary);
    if (!stream) {
        return io_error(preset_path, "cannot open export preset");
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    auto parsed = json::parse(buffer.str(), preset_path.filename().string());
    if (!parsed.ok()) {
        return parsed.take_error();
    }
    ExportPreset preset;
    if (const auto* format = parsed.value().find("format"); format != nullptr) {
        if (!format->is_string()) {
            return Diagnostic::error(ErrorCode::schema_violation, "preset 'format' must be a string");
        }
        preset.format = format->as_string();
    }
    if (preset.format != "obj") {
        return Diagnostic::error(ErrorCode::schema_violation,
                                 "unsupported export format '" + preset.format +
                                     "' (bootstrap supports: obj)");
    }
    if (const auto* profile = parsed.value().find("profile"); profile != nullptr) {
        if (!profile->is_string()) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "preset 'profile' must be a string");
        }
        preset.profile = profile->as_string();
    }
    if (const auto* normals = parsed.value().find("write_normals"); normals != nullptr) {
        if (!normals->is_bool()) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "preset 'write_normals' must be a boolean");
        }
        preset.write_normals = normals->as_bool();
    }
    if (const auto* uvs = parsed.value().find("write_uvs"); uvs != nullptr) {
        if (!uvs->is_bool()) {
            return Diagnostic::error(ErrorCode::schema_violation,
                                     "preset 'write_uvs' must be a boolean");
        }
        preset.write_uvs = uvs->as_bool();
    }
    return preset;
}

Result<ExportManifest> write_obj(const doc::Document& document, const eval::EvaluatedModel& model,
                                 const ExportPreset& preset,
                                 const std::filesystem::path& out_base) {
    std::error_code fs_error;
    const auto parent_dir = out_base.parent_path();
    if (!parent_dir.empty()) {
        std::filesystem::create_directories(parent_dir, fs_error);
        if (fs_error) {
            return io_error(parent_dir, "cannot create output directory: " + fs_error.message());
        }
    }
    const std::filesystem::path obj_path = out_base.string() + ".obj";
    const std::filesystem::path mtl_path = out_base.string() + ".mtl";
    const std::filesystem::path manifest_path = out_base.string() + ".manifest.json";

    // Deterministic MTL: referenced materials in UUID order, plus a default.
    std::string mtl;
    mtl += "# Project Canopy diagnostic export\n";
    mtl += "newmtl canopy_default\nKd 0.6 0.5 0.4\n";
    for (const auto& material : document.materials) {
        mtl += "newmtl ";
        mtl += material.name.empty() ? material.id.str() : material.name;
        mtl += "\nKd 0.5 0.4 0.3\n";
    }

    std::string obj;
    obj += "# Project Canopy diagnostic export\n";
    obj += "# document ";
    obj += document.manifest.document_id.str();
    obj += "\nmtllib ";
    obj += mtl_path.filename().string();
    obj += "\n";

    std::size_t vertex_base = 1; // OBJ indices are 1-based
    for (const auto& node : model.nodes) {
        obj += "g sem_";
        obj += node.semantic_id.str();
        obj += "\nusemtl ";
        const auto* material = document.find_material(node.material_id);
        obj += material != nullptr
                   ? (material->name.empty() ? material->id.str() : material->name)
                   : std::string("canopy_default");
        obj += "\n";
        for (const auto& position : node.mesh.positions) {
            obj += "v ";
            append_number(obj, position.x);
            obj += ' ';
            append_number(obj, position.y);
            obj += ' ';
            append_number(obj, position.z);
            obj += '\n';
        }
        if (preset.write_uvs) {
            for (const auto& uv : node.mesh.uvs) {
                obj += "vt ";
                append_number(obj, uv.x);
                obj += ' ';
                append_number(obj, uv.y);
                obj += '\n';
            }
        }
        if (preset.write_normals) {
            for (const auto& normal : node.mesh.normals) {
                obj += "vn ";
                append_number(obj, normal.x);
                obj += ' ';
                append_number(obj, normal.y);
                obj += ' ';
                append_number(obj, normal.z);
                obj += '\n';
            }
        }
        for (std::size_t t = 0; t < node.mesh.indices.size(); t += 3) {
            obj += "f";
            for (std::size_t k = 0; k < 3; ++k) {
                const std::size_t index = vertex_base + node.mesh.indices[t + k];
                const std::string index_text = json::format_int(std::int64_t(index));
                obj += ' ';
                obj += index_text;
                if (preset.write_uvs || preset.write_normals) {
                    obj += '/';
                    if (preset.write_uvs) {
                        obj += index_text;
                    }
                    if (preset.write_normals) {
                        obj += '/';
                        obj += index_text;
                    }
                }
            }
            obj += '\n';
        }
        vertex_base += node.mesh.vertex_count();
    }

    if (auto r = write_file(obj_path, obj); !r.ok()) {
        return r.take_error();
    }
    if (auto r = write_file(mtl_path, mtl); !r.ok()) {
        return r.take_error();
    }

    ExportManifest manifest;
    manifest.obj_file = obj_path.filename().string();
    manifest.mtl_file = mtl_path.filename().string();
    manifest.obj_sha256 = sha256(obj);
    manifest.model_hash = model.model_hash();
    manifest.model_topology_hash = model.model_topology_hash();
    manifest.node_count = model.nodes.size();
    manifest.vertex_count = model.total_vertices();
    manifest.triangle_count = model.total_triangles();

    json::Object manifest_json;
    manifest_json.emplace("format", "canopy-export-manifest");
    manifest_json.emplace("schema_version", "1.0.0");
    manifest_json.emplace("document_id", document.manifest.document_id.str());
    manifest_json.emplace("export_format", preset.format);
    manifest_json.emplace("profile", preset.profile);
    manifest_json.emplace("obj_file", manifest.obj_file);
    manifest_json.emplace("mtl_file", manifest.mtl_file);
    manifest_json.emplace("obj_sha256", manifest.obj_sha256.hex());
    manifest_json.emplace("model_hash", SemanticId{manifest.model_hash}.str());
    manifest_json.emplace("model_topology_hash", SemanticId{manifest.model_topology_hash}.str());
    manifest_json.emplace("node_count", std::int64_t(manifest.node_count));
    manifest_json.emplace("vertex_count", std::int64_t(manifest.vertex_count));
    manifest_json.emplace("triangle_count", std::int64_t(manifest.triangle_count));
    auto manifest_bytes = json::write_canonical(manifest_json);
    if (!manifest_bytes.ok()) {
        return manifest_bytes.take_error();
    }
    if (auto r = write_file(manifest_path, manifest_bytes.value()); !r.ok()) {
        return r.take_error();
    }
    return manifest;
}

} // namespace canopy::exp
