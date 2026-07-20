// Directory project I/O for `.canopyproj` (backlog B-014) and the document
// content hash (ADR-0002).
//
// Save protocol (crash safety): every file is written to `<name>.tmp` and
// atomically renamed over the target. A `RECOVERY` marker listing the files
// being replaced is created before the first rename and removed after the
// last; loaders refuse to open a project with a marker present and report
// which files may be inconsistent.
#pragma once

#include "canopy/document/document.hpp"
#include "canopy/foundation/hash.hpp"

#include <filesystem>
#include <map>
#include <string>

namespace canopy::doc {

struct ProjectFiles {
    // Project-relative canonical path → canonical bytes.
    std::map<std::string, std::string> files;
};

// Renders all project files as canonical bytes (deterministic: byte-identical
// for equal documents regardless of platform, workers, or working directory).
Result<ProjectFiles> render_project_files(const Document& document);

// Document content hash over the canonical files (ADR-0002).
ContentHash document_hash(const ProjectFiles& files);

Result<void> save_project(const Document& document, const std::filesystem::path& directory);
Result<Document> load_project(const std::filesystem::path& directory);

} // namespace canopy::doc
