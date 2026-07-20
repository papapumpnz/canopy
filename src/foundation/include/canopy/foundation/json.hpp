// Canonical JSON value, parser and writer (ADR-0002, backlog B-013).
//
// Objects are key-sorted maps: canonical serialization sorts keys byte-wise,
// so an ordered map keeps parse → write round trips stable by construction.
// Arrays preserve caller order (semantic order is the caller's contract).
//
// Thread safety: Value is a value type; parser/writer are reentrant.
#pragma once

#include "canopy/foundation/diagnostics.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace canopy::json {

class Value;
using Array = std::vector<Value>;
using Object = std::map<std::string, Value, std::less<>>;

class Value {
public:
    Value() : storage_(nullptr) {}
    Value(std::nullptr_t) : storage_(nullptr) {}
    Value(bool b) : storage_(b) {}
    Value(std::int64_t i) : storage_(i) {}
    Value(int i) : storage_(std::int64_t{i}) {}
    Value(double d) : storage_(d) {}
    Value(const char* s) : storage_(std::string(s)) {}
    Value(std::string s) : storage_(std::move(s)) {}
    Value(std::string_view s) : storage_(std::string(s)) {}
    Value(Array a) : storage_(std::move(a)) {}
    Value(Object o) : storage_(std::move(o)) {}

    bool is_null() const { return std::holds_alternative<std::nullptr_t>(storage_); }
    bool is_bool() const { return std::holds_alternative<bool>(storage_); }
    bool is_int() const { return std::holds_alternative<std::int64_t>(storage_); }
    bool is_double() const { return std::holds_alternative<double>(storage_); }
    bool is_number() const { return is_int() || is_double(); }
    bool is_string() const { return std::holds_alternative<std::string>(storage_); }
    bool is_array() const { return std::holds_alternative<Array>(storage_); }
    bool is_object() const { return std::holds_alternative<Object>(storage_); }

    bool as_bool() const { return std::get<bool>(storage_); }
    std::int64_t as_int() const { return std::get<std::int64_t>(storage_); }
    // Numeric read: int or double, widened to double.
    double as_number() const {
        return is_int() ? double(std::get<std::int64_t>(storage_)) : std::get<double>(storage_);
    }
    const std::string& as_string() const { return std::get<std::string>(storage_); }
    const Array& as_array() const { return std::get<Array>(storage_); }
    Array& as_array() { return std::get<Array>(storage_); }
    const Object& as_object() const { return std::get<Object>(storage_); }
    Object& as_object() { return std::get<Object>(storage_); }

    // Object member lookup; nullptr when absent or not an object.
    const Value* find(std::string_view key) const;

    bool operator==(const Value&) const = default;

private:
    std::variant<std::nullptr_t, bool, std::int64_t, double, std::string, Array, Object> storage_;
};

// Parses strict JSON (UTF-8, no comments, no trailing commas). Rejects
// duplicate object keys and nesting deeper than 128. `source_name` labels
// diagnostics (file path or logical name).
Result<Value> parse(std::string_view text, std::string source_name = {});

// Canonical serialization per ADR-0002: sorted keys, 2-space indent, LF,
// trailing newline. Fails (never emits) on NaN/infinity.
Result<std::string> write_canonical(const Value& value);

// Canonical scalar formatting, shared with any code that prints numbers into
// observable output (B-009 "canonical numeric formatting").
std::string format_double(double value); // shortest round-trip; "0" style ints when integral
std::string format_int(std::int64_t value);

} // namespace canopy::json
