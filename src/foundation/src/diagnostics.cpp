#include "canopy/foundation/diagnostics.hpp"

#include "canopy/foundation/json.hpp"

namespace canopy {

std::string_view error_code_id(ErrorCode code) {
    switch (code) {
    case ErrorCode::none: return "CANOPY-E0000";
    case ErrorCode::invalid_argument: return "CANOPY-E1001";
    case ErrorCode::out_of_range: return "CANOPY-E1002";
    case ErrorCode::not_found: return "CANOPY-E1003";
    case ErrorCode::io_error: return "CANOPY-E2001";
    case ErrorCode::parse_error: return "CANOPY-E2002";
    case ErrorCode::schema_violation: return "CANOPY-E2003";
    case ErrorCode::unsupported_version: return "CANOPY-E2004";
    case ErrorCode::corrupt_data: return "CANOPY-E2005";
    case ErrorCode::graph_cycle: return "CANOPY-E3001";
    case ErrorCode::graph_type_mismatch: return "CANOPY-E3002";
    case ErrorCode::graph_cardinality: return "CANOPY-E3003";
    case ErrorCode::evaluation_failure: return "CANOPY-E4001";
    case ErrorCode::geometry_invalid: return "CANOPY-E4002";
    case ErrorCode::cancelled: return "CANOPY-E5001";
    case ErrorCode::internal_error: return "CANOPY-E9001";
    }
    return "CANOPY-E9001";
}

std::string_view error_code_name(ErrorCode code) {
    switch (code) {
    case ErrorCode::none: return "none";
    case ErrorCode::invalid_argument: return "invalid_argument";
    case ErrorCode::out_of_range: return "out_of_range";
    case ErrorCode::not_found: return "not_found";
    case ErrorCode::io_error: return "io_error";
    case ErrorCode::parse_error: return "parse_error";
    case ErrorCode::schema_violation: return "schema_violation";
    case ErrorCode::unsupported_version: return "unsupported_version";
    case ErrorCode::corrupt_data: return "corrupt_data";
    case ErrorCode::graph_cycle: return "graph_cycle";
    case ErrorCode::graph_type_mismatch: return "graph_type_mismatch";
    case ErrorCode::graph_cardinality: return "graph_cardinality";
    case ErrorCode::evaluation_failure: return "evaluation_failure";
    case ErrorCode::geometry_invalid: return "geometry_invalid";
    case ErrorCode::cancelled: return "cancelled";
    case ErrorCode::internal_error: return "internal_error";
    }
    return "internal_error";
}

namespace {

std::string_view severity_name(Severity severity) {
    switch (severity) {
    case Severity::info: return "info";
    case Severity::warning: return "warning";
    case Severity::error: return "error";
    }
    return "error";
}

void format_human_recursive(const Diagnostic& diagnostic, std::string& out, int depth) {
    for (int i = 0; i < depth; ++i) {
        out += "  ";
    }
    out += severity_name(diagnostic.severity);
    out += " [";
    out += error_code_id(diagnostic.code);
    out += "] ";
    out += diagnostic.message;
    if (!diagnostic.location.file.empty()) {
        out += " (";
        out += diagnostic.location.file;
        if (diagnostic.location.line != 0) {
            out += ':';
            out += json::format_int(diagnostic.location.line);
            if (diagnostic.location.column != 0) {
                out += ':';
                out += json::format_int(diagnostic.location.column);
            }
        }
        out += ')';
    }
    out += '\n';
    for (const auto& note : diagnostic.notes) {
        format_human_recursive(note, out, depth + 1);
    }
}

json::Value to_json_value(const Diagnostic& diagnostic) {
    json::Object object;
    object.emplace("code", std::string(error_code_id(diagnostic.code)));
    object.emplace("name", std::string(error_code_name(diagnostic.code)));
    object.emplace("severity", std::string(severity_name(diagnostic.severity)));
    object.emplace("message", diagnostic.message);
    if (!diagnostic.location.file.empty()) {
        json::Object location;
        location.emplace("file", diagnostic.location.file);
        if (diagnostic.location.line != 0) {
            location.emplace("line", std::int64_t{diagnostic.location.line});
        }
        if (diagnostic.location.column != 0) {
            location.emplace("column", std::int64_t{diagnostic.location.column});
        }
        object.emplace("location", std::move(location));
    }
    if (!diagnostic.notes.empty()) {
        json::Array notes;
        for (const auto& note : diagnostic.notes) {
            notes.push_back(to_json_value(note));
        }
        object.emplace("notes", std::move(notes));
    }
    return object;
}

} // namespace

std::string format_human(const Diagnostic& diagnostic) {
    std::string out;
    format_human_recursive(diagnostic, out, 0);
    return out;
}

std::string format_json(const Diagnostic& diagnostic) {
    auto result = json::write_canonical(to_json_value(diagnostic));
    // Diagnostics never contain non-finite numbers; write cannot fail.
    return result.ok() ? std::move(result).value() : std::string("{}\n");
}

} // namespace canopy
