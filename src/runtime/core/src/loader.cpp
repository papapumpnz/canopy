// `.canopyrt` loader (ADR-0008). Untrusted-input discipline per
// 06_DATA_MODEL_AND_FILE_FORMATS.md: validate all offsets, lengths and
// checksums before use; reject rather than repair.
#include "canopy/runtime/model.hpp"

#include "canopy/foundation/hash.hpp"
#include "canopy/foundation/json.hpp"
#include "canopy/runtime/format.hpp"

#include <cmath>
#include <cstring>
#include <fstream>
#include <map>

namespace canopy::rt {

double RtBounds::radius() const {
    double sum = 0.0;
    for (std::size_t c = 0; c < 3; ++c) {
        const double extent = maximum[c] - minimum[c];
        sum += extent * extent;
    }
    return 0.5 * std::sqrt(sum);
}

namespace {

Diagnostic corrupt(std::string_view source, std::string message) {
    return Diagnostic::error(ErrorCode::corrupt_data, std::move(message),
                             SourceLocation{std::string(source), 0, 0});
}

std::uint32_t read_u32(std::string_view bytes, std::size_t offset) {
    std::uint32_t value = 0;
    std::memcpy(&value, bytes.data() + offset, 4);
    return value; // little-endian hosts only (checked below)
}

std::uint64_t read_u64(std::string_view bytes, std::size_t offset) {
    std::uint64_t value = 0;
    std::memcpy(&value, bytes.data() + offset, 8);
    return value;
}

struct Section {
    SectionType type{};
    std::uint32_t subtype = 0;
    std::string_view payload;
};

Result<std::array<double, 3>> read_vec3(const json::Value& object, std::string_view key,
                                        std::string_view source) {
    const auto* value = object.find(key);
    if (value == nullptr || !value->is_array() || value->as_array().size() != 3) {
        return corrupt(source, "metadata: missing vec3 '" + std::string(key) + "'");
    }
    std::array<double, 3> out{};
    for (std::size_t c = 0; c < 3; ++c) {
        if (!value->as_array()[c].is_number()) {
            return corrupt(source, "metadata: non-numeric component in '" + std::string(key) + "'");
        }
        out[c] = value->as_array()[c].as_number();
    }
    return out;
}

} // namespace

Result<RtModel> load_model_bytes(std::string_view bytes, std::string source_name) {
    const std::uint32_t probe = 1;
    if (*reinterpret_cast<const std::uint8_t*>(&probe) != 1) {
        return Diagnostic::error(ErrorCode::internal_error,
                                 "canopyrt loader requires a little-endian host");
    }
    if (bytes.size() < kHeaderSize) {
        return corrupt(source_name, "file smaller than the header");
    }
    if (std::memcmp(bytes.data(), kMagic, sizeof kMagic) != 0) {
        return corrupt(source_name, "bad magic (not a .canopyrt file)");
    }
    const std::uint32_t version = read_u32(bytes, 8);
    if (version != kVersion) {
        return Diagnostic::error(ErrorCode::unsupported_version,
                                 "unsupported canopyrt version " + std::to_string(version));
    }
    if (read_u32(bytes, 12) != kEndianProbe) {
        return corrupt(source_name, "endianness probe mismatch");
    }
    const std::uint64_t feature_flags = read_u64(bytes, 16);
    if (feature_flags != 0) {
        return Diagnostic::error(ErrorCode::unsupported_version,
                                 "unknown required feature flags");
    }
    const std::uint32_t section_count = read_u32(bytes, 24);
    if (section_count == 0 || section_count > 1024) {
        return corrupt(source_name, "implausible section count");
    }
    const std::size_t table_end = kHeaderSize + std::size_t(section_count) * kTableEntrySize;
    if (table_end > bytes.size()) {
        return corrupt(source_name, "section table exceeds the file");
    }

    std::vector<Section> sections;
    sections.reserve(section_count);
    for (std::uint32_t i = 0; i < section_count; ++i) {
        const std::size_t entry = kHeaderSize + std::size_t(i) * kTableEntrySize;
        Section section;
        section.type = SectionType(read_u32(bytes, entry));
        section.subtype = read_u32(bytes, entry + 4);
        const std::uint64_t offset = read_u64(bytes, entry + 8);
        const std::uint64_t length = read_u64(bytes, entry + 16);
        const std::uint64_t checksum = read_u64(bytes, entry + 24);
        if (offset > bytes.size() || length > bytes.size() - offset) {
            return corrupt(source_name,
                           "section " + std::to_string(i) + " exceeds the file");
        }
        section.payload = bytes.substr(std::size_t(offset), std::size_t(length));
        if (sha256(section.payload).low64() != checksum) {
            return corrupt(source_name,
                           "section " + std::to_string(i) + " checksum mismatch");
        }
        sections.push_back(section);
    }

    auto find_section = [&sections](SectionType type,
                                    std::uint32_t subtype) -> const Section* {
        for (const Section& section : sections) {
            if (section.type == type && section.subtype == subtype) {
                return &section;
            }
        }
        return nullptr;
    };
    auto parse_json_section = [&](SectionType type,
                                  const char* label) -> Result<json::Value> {
        const Section* section = find_section(type, 0);
        if (section == nullptr) {
            return corrupt(source_name, std::string("missing ") + label + " section");
        }
        auto parsed = json::parse(section->payload, std::string(label));
        if (!parsed.ok()) {
            Diagnostic error = corrupt(source_name, std::string(label) + " section is invalid");
            error.with_note(parsed.take_error());
            return error;
        }
        return parsed;
    };

    RtModel model;
    auto metadata = parse_json_section(SectionType::metadata, "metadata");
    if (!metadata.ok()) {
        return metadata.take_error();
    }
    if (const auto* name = metadata.value().find("name"); name != nullptr && name->is_string()) {
        model.name = name->as_string();
    }
    if (const auto* id = metadata.value().find("document_id");
        id != nullptr && id->is_string()) {
        model.document_id = id->as_string();
    }
    if (const auto* hash = metadata.value().find("source_model_hash");
        hash != nullptr && hash->is_string()) {
        model.source_model_hash = hash->as_string();
    }
    const auto* bounds = metadata.value().find("bounds");
    if (bounds == nullptr) {
        return corrupt(source_name, "metadata: missing bounds");
    }
    auto minimum = read_vec3(*bounds, "min", source_name);
    if (!minimum.ok()) {
        return minimum.take_error();
    }
    auto maximum = read_vec3(*bounds, "max", source_name);
    if (!maximum.ok()) {
        return maximum.take_error();
    }
    model.bounds.minimum = minimum.value();
    model.bounds.maximum = maximum.value();

    auto materials = parse_json_section(SectionType::materials, "materials");
    if (!materials.ok()) {
        return materials.take_error();
    }
    if (!materials.value().is_array()) {
        return corrupt(source_name, "materials section must be an array");
    }
    for (const auto& entry : materials.value().as_array()) {
        RtMaterial material;
        if (const auto* name = entry.find("name"); name != nullptr && name->is_string()) {
            material.name = name->as_string();
        }
        if (const auto* color = entry.find("color");
            color != nullptr && color->is_array() && color->as_array().size() == 4) {
            for (std::size_t c = 0; c < 4; ++c) {
                if (color->as_array()[c].is_number()) {
                    material.color[c] = color->as_array()[c].as_number();
                }
            }
        }
        if (const auto* two_sided = entry.find("two_sided");
            two_sided != nullptr && two_sided->is_bool()) {
            material.two_sided = two_sided->as_bool();
        }
        model.materials.push_back(std::move(material));
    }

    auto lods = parse_json_section(SectionType::lods, "lods");
    if (!lods.ok()) {
        return lods.take_error();
    }
    if (!lods.value().is_array() || lods.value().as_array().empty()) {
        return corrupt(source_name, "lods section must be a non-empty array");
    }
    for (std::size_t lod_index = 0; lod_index < lods.value().as_array().size(); ++lod_index) {
        const auto& descriptor = lods.value().as_array()[lod_index];
        const Section* vertex_section =
            find_section(SectionType::vertices, std::uint32_t(lod_index));
        const Section* index_section =
            find_section(SectionType::indices, std::uint32_t(lod_index));
        if (vertex_section == nullptr || index_section == nullptr) {
            return corrupt(source_name,
                           "missing buffers for LOD " + std::to_string(lod_index));
        }
        if (vertex_section->payload.size() % kVertexStrideBytes != 0) {
            return corrupt(source_name, "vertex buffer size is not a stride multiple");
        }
        if (index_section->payload.size() % 4 != 0) {
            return corrupt(source_name, "index buffer size is not a multiple of 4");
        }
        RtLod lod;
        lod.vertices.resize(vertex_section->payload.size() / 4);
        std::memcpy(lod.vertices.data(), vertex_section->payload.data(),
                    vertex_section->payload.size());
        lod.indices.resize(index_section->payload.size() / 4);
        std::memcpy(lod.indices.data(), index_section->payload.data(),
                    index_section->payload.size());
        const std::size_t vertex_count = lod.vertex_count();
        for (const std::uint32_t index : lod.indices) {
            if (index >= vertex_count) {
                return corrupt(source_name,
                               "index out of range in LOD " + std::to_string(lod_index));
            }
        }
        for (const float value : lod.vertices) {
            if (!std::isfinite(value)) {
                return corrupt(source_name,
                               "non-finite vertex data in LOD " + std::to_string(lod_index));
            }
        }
        const auto* primitives = descriptor.find("primitives");
        if (primitives == nullptr || !primitives->is_array()) {
            return corrupt(source_name,
                           "LOD " + std::to_string(lod_index) + " has no primitives");
        }
        for (const auto& entry : primitives->as_array()) {
            RtPrimitive primitive;
            const auto* material = entry.find("material");
            const auto* offset = entry.find("index_offset");
            const auto* count = entry.find("index_count");
            if (material == nullptr || !material->is_int() || offset == nullptr ||
                !offset->is_int() || count == nullptr || !count->is_int()) {
                return corrupt(source_name, "malformed primitive descriptor");
            }
            primitive.material = std::uint32_t(material->as_int());
            primitive.index_offset = std::uint32_t(offset->as_int());
            primitive.index_count = std::uint32_t(count->as_int());
            if (primitive.material >= model.materials.size()) {
                return corrupt(source_name, "primitive references an unknown material");
            }
            if (std::size_t(primitive.index_offset) + primitive.index_count >
                    lod.indices.size() ||
                primitive.index_count % 3 != 0) {
                return corrupt(source_name, "primitive range exceeds the index buffer");
            }
            lod.primitives.push_back(primitive);
        }
        model.lods.push_back(std::move(lod));
    }
    return model;
}

Result<RtModel> load_model(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return Diagnostic::error(ErrorCode::not_found,
                                 "cannot open " + path.string());
    }
    std::string bytes((std::istreambuf_iterator<char>(stream)),
                      std::istreambuf_iterator<char>());
    return load_model_bytes(bytes, path.filename().string());
}

} // namespace canopy::rt
