// Diagnostics and error model (backlog B-006).
// Stable machine codes shared by C++, C-ABI and CLI surfaces. Thread safety:
// Diagnostic and Result are value types; safe to move across threads.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace canopy {

// Stable error categories. Values are part of the machine contract: never
// renumber or repurpose (playbook rule); add at the end only.
enum class ErrorCode : std::uint32_t {
    none = 0,
    invalid_argument = 1001,
    out_of_range = 1002,
    not_found = 1003,
    io_error = 2001,
    parse_error = 2002,
    schema_violation = 2003,
    unsupported_version = 2004,
    corrupt_data = 2005,
    graph_cycle = 3001,
    graph_type_mismatch = 3002,
    graph_cardinality = 3003,
    evaluation_failure = 4001,
    geometry_invalid = 4002,
    cancelled = 5001,
    internal_error = 9001,
};

// Stable machine string for an error code, e.g. "CANOPY-E2002".
std::string_view error_code_id(ErrorCode code);
std::string_view error_code_name(ErrorCode code);

enum class Severity : std::uint8_t { info, warning, error };

struct SourceLocation {
    std::string file;       // project-relative path or logical source name
    std::uint32_t line = 0; // 1-based; 0 when unknown
    std::uint32_t column = 0;
};

// One diagnostic with optional nested notes (e.g. schema error with the
// offending property path as a note).
struct Diagnostic {
    ErrorCode code = ErrorCode::none;
    Severity severity = Severity::error;
    std::string message;
    SourceLocation location;
    std::vector<Diagnostic> notes;

    static Diagnostic error(ErrorCode code, std::string message,
                            SourceLocation location = {}) {
        return Diagnostic{code, Severity::error, std::move(message), std::move(location), {}};
    }
    static Diagnostic warning(ErrorCode code, std::string message,
                              SourceLocation location = {}) {
        return Diagnostic{code, Severity::warning, std::move(message), std::move(location), {}};
    }

    Diagnostic& with_note(Diagnostic note) {
        notes.push_back(std::move(note));
        return *this;
    }
};

// Human-readable one-per-line rendering.
std::string format_human(const Diagnostic& diagnostic);
// Machine rendering as canonical JSON (same code surfaces everywhere: B-006).
std::string format_json(const Diagnostic& diagnostic);

// Minimal expected-style result. Ok state holds T; error state holds a
// Diagnostic. No exceptions cross public boundaries.
template <typename T>
class [[nodiscard]] Result {
public:
    Result(T value) : state_(std::in_place_index<0>, std::move(value)) {}
    Result(Diagnostic diagnostic) : state_(std::in_place_index<1>, std::move(diagnostic)) {}

    bool ok() const { return state_.index() == 0; }
    explicit operator bool() const { return ok(); }

    T& value() & { return std::get<0>(state_); }
    const T& value() const& { return std::get<0>(state_); }
    T&& value() && { return std::get<0>(std::move(state_)); }

    const Diagnostic& error() const { return std::get<1>(state_); }
    Diagnostic&& take_error() { return std::get<1>(std::move(state_)); }

private:
    std::variant<T, Diagnostic> state_;
};

struct Ok {};

template <>
class [[nodiscard]] Result<void> {
public:
    Result() = default;
    Result(Ok) {}
    Result(Diagnostic diagnostic) : error_(std::in_place, std::move(diagnostic)) {}

    bool ok() const { return !error_.has_value(); }
    explicit operator bool() const { return ok(); }
    const Diagnostic& error() const { return *error_; }
    Diagnostic&& take_error() { return std::move(*error_); }

private:
    std::optional<Diagnostic> error_{};
};

} // namespace canopy
