#include "canopy/document/project_io.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <system_error>

namespace canopy::doc {

namespace {

constexpr std::string_view kRecoveryMarker = "RECOVERY";

Diagnostic io_error(const std::filesystem::path& path, std::string message) {
    return Diagnostic::error(ErrorCode::io_error, std::move(message),
                             SourceLocation{path.string(), 0, 0});
}

Result<std::string> read_file(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return io_error(path, "cannot open file for reading");
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    if (stream.bad()) {
        return io_error(path, "read failure");
    }
    return std::move(buffer).str();
}

Result<void> write_file_atomic(const std::filesystem::path& target, std::string_view bytes) {
    const std::filesystem::path temp = target.string() + ".tmp";
    {
        std::ofstream stream(temp, std::ios::binary | std::ios::trunc);
        if (!stream) {
            return io_error(temp, "cannot open temporary file for writing");
        }
        stream.write(bytes.data(), std::streamsize(bytes.size()));
        stream.flush();
        if (!stream) {
            return io_error(temp, "write failure");
        }
    }
    std::error_code rename_error;
    std::filesystem::rename(temp, target, rename_error);
    if (rename_error) {
        std::error_code cleanup;
        std::filesystem::remove(temp, cleanup);
        return io_error(target, "atomic rename failed: " + rename_error.message());
    }
    return Ok{};
}

} // namespace

Result<ProjectFiles> render_project_files(const Document& document) {
    if (auto valid = document.validate(); !valid.ok()) {
        return valid.take_error();
    }
    ProjectFiles out;
    const std::array<std::pair<std::string, json::Value>, 4> parts = {{
        {"manifest.json", manifest_to_json(document.manifest)},
        {"graph.json", graph_to_json(document)},
        {"properties.json", properties_to_json(document)},
        {"materials.json", materials_to_json(document)},
    }};
    for (const auto& [name, value] : parts) {
        auto bytes = json::write_canonical(value);
        if (!bytes.ok()) {
            return bytes.take_error();
        }
        out.files.emplace(name, std::move(bytes).value());
    }
    return out;
}

ContentHash document_hash(const ProjectFiles& files) {
    // ADR-0002: per-file domain-separated hashes, then a hash over the sorted
    // (path, hash) list. std::map iteration is already path-sorted.
    Sha256 outer;
    outer.update(std::string_view("canopy-doc-v1"));
    outer.update("\0", 1);
    for (const auto& [path, bytes] : files.files) {
        Sha256 inner;
        inner.update(std::string_view("canopy-file-v1"));
        inner.update("\0", 1);
        inner.update(path);
        inner.update("\0", 1);
        inner.update(bytes);
        const ContentHash file_hash = inner.finish();
        outer.update(path);
        outer.update("\0", 1);
        outer.update(file_hash.bytes.data(), file_hash.bytes.size());
    }
    return outer.finish();
}

Result<void> save_project(const Document& document, const std::filesystem::path& directory) {
    auto rendered = render_project_files(document);
    if (!rendered.ok()) {
        return rendered.take_error();
    }

    std::error_code fs_error;
    std::filesystem::create_directories(directory, fs_error);
    if (fs_error) {
        return io_error(directory, "cannot create project directory: " + fs_error.message());
    }

    // Recovery marker protocol (B-014): the marker lists the files being
    // replaced. If a crash interrupts the renames, the marker survives and
    // load_project refuses the directory with an actionable diagnostic.
    const std::filesystem::path marker = directory / kRecoveryMarker;
    {
        std::string marker_text = "canopy save in progress\n";
        for (const auto& [name, bytes] : rendered.value().files) {
            marker_text += name;
            marker_text += '\n';
        }
        if (auto r = write_file_atomic(marker, marker_text); !r.ok()) {
            return r;
        }
    }
    for (const auto& [name, bytes] : rendered.value().files) {
        if (auto r = write_file_atomic(directory / name, bytes); !r.ok()) {
            return r;
        }
    }
    std::filesystem::remove(marker, fs_error);
    if (fs_error) {
        return io_error(marker, "cannot remove recovery marker: " + fs_error.message());
    }
    return Ok{};
}

Result<Document> load_project(const std::filesystem::path& directory) {
    std::error_code fs_error;
    if (!std::filesystem::is_directory(directory, fs_error)) {
        return Diagnostic::error(ErrorCode::not_found,
                                 "project directory not found: " + directory.string());
    }
    if (std::filesystem::exists(directory / kRecoveryMarker, fs_error)) {
        return Diagnostic::error(ErrorCode::corrupt_data,
                                 "project has an interrupted save (RECOVERY marker present); "
                                 "restore from source control or the previous file versions")
            .with_note(Diagnostic::warning(ErrorCode::io_error,
                                           (directory / kRecoveryMarker).string()));
    }

    auto parse_part = [&directory](std::string_view name) -> Result<json::Value> {
        const std::filesystem::path path = directory / name;
        auto bytes = read_file(path);
        if (!bytes.ok()) {
            return bytes.take_error();
        }
        return json::parse(bytes.value(), path.filename().string());
    };

    auto manifest = parse_part("manifest.json");
    if (!manifest.ok()) {
        return manifest.take_error();
    }
    auto graph = parse_part("graph.json");
    if (!graph.ok()) {
        return graph.take_error();
    }
    auto properties = parse_part("properties.json");
    if (!properties.ok()) {
        return properties.take_error();
    }
    auto materials = parse_part("materials.json");
    if (!materials.ok()) {
        return materials.take_error();
    }
    auto document = document_from_json(manifest.value(), graph.value(), properties.value(),
                                       materials.value());
    if (document.ok()) {
        document.value().project_root = directory.string();
    }
    return document;
}

} // namespace canopy::doc
