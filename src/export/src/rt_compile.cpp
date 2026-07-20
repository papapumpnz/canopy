#include "canopy/export/rt_compile.hpp"

#include "canopy/export/mesh_merge.hpp"
#include "canopy/runtime/format.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <fstream>
#include <map>

namespace canopy::exp {

namespace {

Diagnostic io_error(const std::filesystem::path& path, std::string message) {
    return Diagnostic::error(ErrorCode::io_error, std::move(message),
                             SourceLocation{path.string(), 0, 0});
}

void put_u32(std::string& out, std::uint32_t value) {
    out.append(reinterpret_cast<const char*>(&value), 4);
}

void put_u64(std::string& out, std::uint64_t value) {
    out.append(reinterpret_cast<const char*>(&value), 8);
}

struct PendingSection {
    rt::SectionType type{};
    std::uint32_t subtype = 0;
    std::string payload;
};

} // namespace

Result<RtCompileManifest> write_canopyrt(const doc::Document& document,
                                         const std::vector<eval::EvaluatedModel>& lods,
                                         const std::filesystem::path& out_base) {
    static_assert(std::numeric_limits<float>::is_iec559, "IEEE-754 float required");
    const std::uint32_t probe = 1;
    if (*reinterpret_cast<const std::uint8_t*>(&probe) != 1) {
        return Diagnostic::error(ErrorCode::internal_error,
                                 "canopyrt writer requires a little-endian host");
    }
    if (lods.empty()) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "canopyrt requires at least one LOD model");
    }

    // Merge every LOD; build a global material table keyed by material UUID
    // so primitives across LODs share indices (season/LOD may drop whole
    // material groups — absent groups simply have no primitives).
    std::vector<std::vector<MergedPrimitive>> merged_lods;
    std::map<Uuid, std::uint32_t> material_slots;
    std::vector<const MergedPrimitive*> material_sources;
    for (const auto& model : lods) {
        merged_lods.push_back(merge_by_material(document, model));
        if (merged_lods.back().empty()) {
            return Diagnostic::error(ErrorCode::invalid_argument,
                                     "cannot compile an empty LOD model");
        }
    }
    for (const auto& primitives : merged_lods) {
        for (const auto& primitive : primitives) {
            if (material_slots.try_emplace(primitive.material_id,
                                           std::uint32_t(material_sources.size()))
                    .second) {
                material_sources.push_back(&primitive);
            }
        }
    }

    RtCompileManifest manifest;
    manifest.lod_count = lods.size();

    // Bounds from LOD 0.
    std::array<double, 3> minimum{1e30, 1e30, 1e30};
    std::array<double, 3> maximum{-1e30, -1e30, -1e30};
    for (const auto& primitive : merged_lods.front()) {
        for (std::size_t c = 0; c < 3; ++c) {
            minimum[c] = std::min(minimum[c], primitive.min_position[c]);
            maximum[c] = std::max(maximum[c], primitive.max_position[c]);
        }
    }

    std::vector<PendingSection> sections;
    {
        json::Object metadata;
        metadata.emplace("format", "canopy-runtime");
        metadata.emplace("version", std::int64_t(rt::kVersion));
        metadata.emplace("name", document.manifest.name);
        metadata.emplace("document_id", document.manifest.document_id.str());
        metadata.emplace("source_model_hash", SemanticId{lods.front().model_hash()}.str());
        json::Object bounds;
        json::Array bounds_min;
        json::Array bounds_max;
        for (std::size_t c = 0; c < 3; ++c) {
            bounds_min.push_back(minimum[c]);
            bounds_max.push_back(maximum[c]);
        }
        bounds.emplace("min", std::move(bounds_min));
        bounds.emplace("max", std::move(bounds_max));
        metadata.emplace("bounds", std::move(bounds));
        metadata.emplace("lod_count", std::int64_t(lods.size()));
        auto text = json::write_canonical(metadata);
        if (!text.ok()) {
            return text.take_error();
        }
        sections.push_back({rt::SectionType::metadata, 0, std::move(text).value()});
    }
    {
        json::Array materials;
        for (const MergedPrimitive* source : material_sources) {
            json::Object material;
            material.emplace("name", source->material_name);
            json::Array color;
            for (const double channel : source->color) {
                color.push_back(channel);
            }
            material.emplace("color", std::move(color));
            material.emplace("two_sided", source->double_sided);
            materials.push_back(std::move(material));
        }
        auto text = json::write_canonical(json::Value(std::move(materials)));
        if (!text.ok()) {
            return text.take_error();
        }
        sections.push_back({rt::SectionType::materials, 0, std::move(text).value()});
    }
    {
        json::Array lod_array;
        for (std::size_t lod_index = 0; lod_index < merged_lods.size(); ++lod_index) {
            json::Object descriptor;
            descriptor.emplace("lod", std::int64_t(lod_index));
            json::Array primitive_array;
            std::uint32_t index_cursor = 0;
            std::size_t lod_triangles = 0;
            for (const auto& primitive : merged_lods[lod_index]) {
                json::Object entry;
                entry.emplace("material",
                              std::int64_t(material_slots.at(primitive.material_id)));
                entry.emplace("index_offset", std::int64_t(index_cursor));
                entry.emplace("index_count", std::int64_t(primitive.indices.size()));
                primitive_array.push_back(std::move(entry));
                index_cursor += std::uint32_t(primitive.indices.size());
                lod_triangles += primitive.indices.size() / 3;
            }
            manifest.triangles_per_lod.push_back(lod_triangles);
            descriptor.emplace("primitives", std::move(primitive_array));
            lod_array.push_back(std::move(descriptor));
        }
        auto text = json::write_canonical(json::Value(std::move(lod_array)));
        if (!text.ok()) {
            return text.take_error();
        }
        sections.push_back({rt::SectionType::lods, 0, std::move(text).value()});
    }
    for (std::size_t lod_index = 0; lod_index < merged_lods.size(); ++lod_index) {
        // Interleave (pos3, normal3, uv2) per vertex; concatenate primitives
        // with rebased indices in material-table order.
        std::string vertex_bytes;
        std::string index_bytes;
        std::uint32_t vertex_cursor = 0;
        for (const auto& primitive : merged_lods[lod_index]) {
            const std::size_t count = primitive.vertex_count();
            for (std::size_t v = 0; v < count; ++v) {
                const float interleaved[8] = {
                    primitive.positions[v * 3],     primitive.positions[v * 3 + 1],
                    primitive.positions[v * 3 + 2], primitive.normals[v * 3],
                    primitive.normals[v * 3 + 1],   primitive.normals[v * 3 + 2],
                    primitive.uvs[v * 2],           primitive.uvs[v * 2 + 1]};
                vertex_bytes.append(reinterpret_cast<const char*>(interleaved),
                                    sizeof interleaved);
            }
            for (const std::uint32_t index : primitive.indices) {
                put_u32(index_bytes, vertex_cursor + index);
            }
            vertex_cursor += std::uint32_t(count);
        }
        sections.push_back(
            {rt::SectionType::vertices, std::uint32_t(lod_index), std::move(vertex_bytes)});
        sections.push_back(
            {rt::SectionType::indices, std::uint32_t(lod_index), std::move(index_bytes)});
    }

    // Assemble: header, table, 8-byte-aligned payloads.
    std::string file;
    file.append(rt::kMagic, sizeof rt::kMagic);
    put_u32(file, rt::kVersion);
    put_u32(file, rt::kEndianProbe);
    put_u64(file, 0); // feature flags
    put_u32(file, std::uint32_t(sections.size()));
    put_u32(file, 0); // reserved

    std::size_t payload_offset =
        rt::kHeaderSize + sections.size() * rt::kTableEntrySize;
    std::vector<std::size_t> offsets;
    for (const PendingSection& section : sections) {
        payload_offset = (payload_offset + 7) & ~std::size_t(7);
        offsets.push_back(payload_offset);
        payload_offset += section.payload.size();
    }
    for (std::size_t i = 0; i < sections.size(); ++i) {
        put_u32(file, std::uint32_t(sections[i].type));
        put_u32(file, sections[i].subtype);
        put_u64(file, offsets[i]);
        put_u64(file, sections[i].payload.size());
        put_u64(file, sha256(sections[i].payload).low64());
    }
    for (std::size_t i = 0; i < sections.size(); ++i) {
        while (file.size() < offsets[i]) {
            file.push_back('\0');
        }
        file += sections[i].payload;
    }

    std::error_code fs_error;
    const auto parent_dir = out_base.parent_path();
    if (!parent_dir.empty()) {
        std::filesystem::create_directories(parent_dir, fs_error);
        if (fs_error) {
            return io_error(parent_dir, "cannot create output directory: " + fs_error.message());
        }
    }
    const std::filesystem::path rt_path = out_base.string() + ".canopyrt";
    {
        std::ofstream stream(rt_path, std::ios::binary | std::ios::trunc);
        if (!stream) {
            return io_error(rt_path, "cannot open file for writing");
        }
        stream.write(file.data(), std::streamsize(file.size()));
        stream.flush();
        if (!stream) {
            return io_error(rt_path, "write failure");
        }
    }
    manifest.rt_file = rt_path.filename().string();
    manifest.rt_sha256 = sha256(file);

    json::Object manifest_json;
    manifest_json.emplace("format", "canopy-export-manifest");
    manifest_json.emplace("schema_version", "1.0.0");
    manifest_json.emplace("document_id", document.manifest.document_id.str());
    manifest_json.emplace("export_format", "canopyrt");
    manifest_json.emplace("rt_file", manifest.rt_file);
    manifest_json.emplace("rt_sha256", manifest.rt_sha256.hex());
    manifest_json.emplace("lod_count", std::int64_t(manifest.lod_count));
    json::Array triangles;
    for (const std::size_t count : manifest.triangles_per_lod) {
        triangles.push_back(std::int64_t(count));
    }
    manifest_json.emplace("triangles_per_lod", std::move(triangles));
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
