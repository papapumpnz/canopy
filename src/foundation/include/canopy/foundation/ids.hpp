// Stable identifiers (backlog B-009, data model "Stable identity").
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace canopy {

// 128-bit object identity, textually an RFC 4122-style lowercase UUID.
struct Uuid {
    std::array<std::uint8_t, 16> bytes{};

    bool is_nil() const;
    std::string str() const; // "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
    static std::optional<Uuid> parse(std::string_view text);
    // Random (version 4) UUID from the OS entropy source. Only for authoring
    // actions that create new persistent objects — never inside evaluation.
    static Uuid generate();
    auto operator<=>(const Uuid&) const = default;
};

// Deterministic identity of a generated node: SHA-256-derived, rendered as
// 16 lowercase hex chars ("sem:xxxxxxxxxxxxxxxx" in diagnostics). Derivation
// inputs are defined in 06_DATA_MODEL_AND_FILE_FORMATS.md.
struct SemanticId {
    std::uint64_t value = 0;

    std::string str() const;
    auto operator<=>(const SemanticId&) const = default;
};

} // namespace canopy
