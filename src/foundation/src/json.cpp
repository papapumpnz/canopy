#include "canopy/foundation/json.hpp"

#include <charconv>
#include <cmath>
#include <cstdio>

namespace canopy::json {

const Value* Value::find(std::string_view key) const {
    if (!is_object()) {
        return nullptr;
    }
    const auto& object = as_object();
    const auto it = object.find(key);
    return it == object.end() ? nullptr : &it->second;
}

std::string format_int(std::int64_t value) {
    char buffer[24];
    const auto result = std::to_chars(buffer, buffer + sizeof buffer, value);
    return std::string(buffer, result.ptr);
}

std::string format_double(double value) {
    // ADR-0002: integral values within the exactly-representable range print
    // as integers; everything else uses shortest round-trip formatting.
    constexpr double kMaxExactInt = 9007199254740992.0; // 2^53
    if (std::isfinite(value) && value == std::floor(value) && std::fabs(value) <= kMaxExactInt) {
        return format_int(std::int64_t(value));
    }
    char buffer[32];
    const auto result = std::to_chars(buffer, buffer + sizeof buffer, value);
    return std::string(buffer, result.ptr);
}

namespace {

constexpr std::size_t kMaxDepth = 128;

class Parser {
public:
    Parser(std::string_view text, std::string source_name)
        : text_(text), source_(std::move(source_name)) {}

    Result<Value> run() {
        skip_whitespace();
        auto value = parse_value(0);
        if (!value.ok()) {
            return value;
        }
        skip_whitespace();
        if (position_ != text_.size()) {
            return fail("trailing characters after top-level value");
        }
        return value;
    }

private:
    Diagnostic fail(std::string message) const {
        return Diagnostic::error(ErrorCode::parse_error, std::move(message),
                                 SourceLocation{source_, line_, column_});
    }

    bool eof() const { return position_ >= text_.size(); }
    char peek() const { return text_[position_]; }

    char advance() {
        const char c = text_[position_++];
        if (c == '\n') {
            ++line_;
            column_ = 1;
        } else {
            ++column_;
        }
        return c;
    }

    void skip_whitespace() {
        while (!eof()) {
            const char c = peek();
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                advance();
            } else {
                break;
            }
        }
    }

    bool consume_literal(std::string_view literal) {
        if (text_.substr(position_, literal.size()) != literal) {
            return false;
        }
        for (std::size_t i = 0; i < literal.size(); ++i) {
            advance();
        }
        return true;
    }

    Result<Value> parse_value(std::size_t depth) {
        if (depth > kMaxDepth) {
            return fail("nesting exceeds maximum depth");
        }
        if (eof()) {
            return fail("unexpected end of input");
        }
        switch (peek()) {
        case '{':
            return parse_object(depth);
        case '[':
            return parse_array(depth);
        case '"': {
            auto text = parse_string();
            if (!text.ok()) {
                return text.take_error();
            }
            return Value(std::move(text).value());
        }
        case 't':
            if (consume_literal("true")) {
                return Value(true);
            }
            return fail("invalid literal");
        case 'f':
            if (consume_literal("false")) {
                return Value(false);
            }
            return fail("invalid literal");
        case 'n':
            if (consume_literal("null")) {
                return Value(nullptr);
            }
            return fail("invalid literal");
        default:
            return parse_number();
        }
    }

    Result<Value> parse_object(std::size_t depth) {
        advance(); // '{'
        Object object;
        skip_whitespace();
        if (!eof() && peek() == '}') {
            advance();
            return Value(std::move(object));
        }
        while (true) {
            skip_whitespace();
            if (eof() || peek() != '"') {
                return fail("expected object key string");
            }
            auto key = parse_string();
            if (!key.ok()) {
                return key.take_error();
            }
            skip_whitespace();
            if (eof() || peek() != ':') {
                return fail("expected ':' after object key");
            }
            advance();
            skip_whitespace();
            auto value = parse_value(depth + 1);
            if (!value.ok()) {
                return value;
            }
            if (!object.emplace(std::move(key).value(), std::move(value).value()).second) {
                return fail("duplicate object key");
            }
            skip_whitespace();
            if (eof()) {
                return fail("unterminated object");
            }
            const char c = advance();
            if (c == '}') {
                return Value(std::move(object));
            }
            if (c != ',') {
                return fail("expected ',' or '}' in object");
            }
        }
    }

    Result<Value> parse_array(std::size_t depth) {
        advance(); // '['
        Array array;
        skip_whitespace();
        if (!eof() && peek() == ']') {
            advance();
            return Value(std::move(array));
        }
        while (true) {
            skip_whitespace();
            auto value = parse_value(depth + 1);
            if (!value.ok()) {
                return value;
            }
            array.push_back(std::move(value).value());
            skip_whitespace();
            if (eof()) {
                return fail("unterminated array");
            }
            const char c = advance();
            if (c == ']') {
                return Value(std::move(array));
            }
            if (c != ',') {
                return fail("expected ',' or ']' in array");
            }
        }
    }

    Result<std::string> parse_string() {
        advance(); // '"'
        std::string out;
        while (true) {
            if (eof()) {
                return fail("unterminated string");
            }
            const char c = advance();
            if (c == '"') {
                return out;
            }
            if (static_cast<unsigned char>(c) < 0x20) {
                return fail("raw control character in string");
            }
            if (c != '\\') {
                out.push_back(c);
                continue;
            }
            if (eof()) {
                return fail("unterminated escape sequence");
            }
            const char escape = advance();
            switch (escape) {
            case '"': out.push_back('"'); break;
            case '\\': out.push_back('\\'); break;
            case '/': out.push_back('/'); break;
            case 'b': out.push_back('\b'); break;
            case 'f': out.push_back('\f'); break;
            case 'n': out.push_back('\n'); break;
            case 'r': out.push_back('\r'); break;
            case 't': out.push_back('\t'); break;
            case 'u': {
                auto first = parse_hex4();
                if (!first.ok()) {
                    return first.take_error();
                }
                std::uint32_t code_point = first.value();
                if (code_point >= 0xd800 && code_point <= 0xdbff) {
                    // High surrogate: require a following \uXXXX low surrogate.
                    if (position_ + 1 >= text_.size() || text_[position_] != '\\' ||
                        text_[position_ + 1] != 'u') {
                        return fail("unpaired high surrogate");
                    }
                    advance();
                    advance();
                    auto second = parse_hex4();
                    if (!second.ok()) {
                        return second.take_error();
                    }
                    const std::uint32_t low = second.value();
                    if (low < 0xdc00 || low > 0xdfff) {
                        return fail("invalid low surrogate");
                    }
                    code_point = 0x10000 + ((code_point - 0xd800) << 10) + (low - 0xdc00);
                } else if (code_point >= 0xdc00 && code_point <= 0xdfff) {
                    return fail("unpaired low surrogate");
                }
                append_utf8(out, code_point);
                break;
            }
            default:
                return fail("invalid escape character");
            }
        }
    }

    Result<std::uint32_t> parse_hex4() {
        std::uint32_t value = 0;
        for (int i = 0; i < 4; ++i) {
            if (eof()) {
                return fail("unterminated \\u escape");
            }
            const char c = advance();
            value <<= 4;
            if (c >= '0' && c <= '9') {
                value |= std::uint32_t(c - '0');
            } else if (c >= 'a' && c <= 'f') {
                value |= std::uint32_t(c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                value |= std::uint32_t(c - 'A' + 10);
            } else {
                return fail("invalid hex digit in \\u escape");
            }
        }
        return value;
    }

    static void append_utf8(std::string& out, std::uint32_t code_point) {
        if (code_point < 0x80) {
            out.push_back(char(code_point));
        } else if (code_point < 0x800) {
            out.push_back(char(0xc0u | (code_point >> 6)));
            out.push_back(char(0x80u | (code_point & 0x3fu)));
        } else if (code_point < 0x10000) {
            out.push_back(char(0xe0u | (code_point >> 12)));
            out.push_back(char(0x80u | ((code_point >> 6) & 0x3fu)));
            out.push_back(char(0x80u | (code_point & 0x3fu)));
        } else {
            out.push_back(char(0xf0u | (code_point >> 18)));
            out.push_back(char(0x80u | ((code_point >> 12) & 0x3fu)));
            out.push_back(char(0x80u | ((code_point >> 6) & 0x3fu)));
            out.push_back(char(0x80u | (code_point & 0x3fu)));
        }
    }

    Result<Value> parse_number() {
        const std::size_t start = position_;
        if (!eof() && peek() == '-') {
            advance();
        }
        if (eof() || peek() < '0' || peek() > '9') {
            return fail("invalid number");
        }
        if (peek() == '0') {
            advance();
            if (!eof() && peek() >= '0' && peek() <= '9') {
                return fail("leading zero in number");
            }
        } else {
            while (!eof() && peek() >= '0' && peek() <= '9') {
                advance();
            }
        }
        bool is_integer = true;
        if (!eof() && peek() == '.') {
            is_integer = false;
            advance();
            if (eof() || peek() < '0' || peek() > '9') {
                return fail("digit required after decimal point");
            }
            while (!eof() && peek() >= '0' && peek() <= '9') {
                advance();
            }
        }
        if (!eof() && (peek() == 'e' || peek() == 'E')) {
            is_integer = false;
            advance();
            if (!eof() && (peek() == '+' || peek() == '-')) {
                advance();
            }
            if (eof() || peek() < '0' || peek() > '9') {
                return fail("digit required in exponent");
            }
            while (!eof() && peek() >= '0' && peek() <= '9') {
                advance();
            }
        }
        const std::string_view token = text_.substr(start, position_ - start);
        if (is_integer) {
            std::int64_t integer_value = 0;
            const auto result =
                std::from_chars(token.data(), token.data() + token.size(), integer_value);
            if (result.ec == std::errc() && result.ptr == token.data() + token.size()) {
                return Value(integer_value);
            }
            // Falls through to double for integers outside the int64 range.
        }
        double double_value = 0.0;
        const auto result =
            std::from_chars(token.data(), token.data() + token.size(), double_value);
        if (result.ec != std::errc() || result.ptr != token.data() + token.size()) {
            return fail("unparsable number");
        }
        if (!std::isfinite(double_value)) {
            return fail("number out of range");
        }
        return Value(double_value);
    }

    std::string_view text_;
    std::string source_;
    std::size_t position_ = 0;
    std::uint32_t line_ = 1;
    std::uint32_t column_ = 1;
};

void write_escaped_string(std::string& out, std::string_view text) {
    out.push_back('"');
    for (const char c : text) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\t': out += "\\t"; break;
        case '\r': out += "\\r"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char buffer[8];
                std::snprintf(buffer, sizeof buffer, "\\u%04x",
                              unsigned(static_cast<unsigned char>(c)));
                out += buffer;
            } else {
                out.push_back(c);
            }
        }
    }
    out.push_back('"');
}

Result<void> write_value(std::string& out, const Value& value, std::size_t indent) {
    const std::string pad(indent * 2, ' ');
    const std::string child_pad((indent + 1) * 2, ' ');
    if (value.is_null()) {
        out += "null";
    } else if (value.is_bool()) {
        out += value.as_bool() ? "true" : "false";
    } else if (value.is_int()) {
        out += format_int(value.as_int());
    } else if (value.is_double()) {
        const double d = value.as_number();
        if (!std::isfinite(d)) {
            return Diagnostic::error(ErrorCode::invalid_argument,
                                     "cannot serialize non-finite number");
        }
        out += format_double(d);
    } else if (value.is_string()) {
        write_escaped_string(out, value.as_string());
    } else if (value.is_array()) {
        const auto& array = value.as_array();
        if (array.empty()) {
            out += "[]";
            return Ok{};
        }
        out += "[\n";
        for (std::size_t i = 0; i < array.size(); ++i) {
            out += child_pad;
            if (auto r = write_value(out, array[i], indent + 1); !r.ok()) {
                return r;
            }
            out += (i + 1 < array.size()) ? ",\n" : "\n";
        }
        out += pad;
        out.push_back(']');
    } else {
        const auto& object = value.as_object();
        if (object.empty()) {
            out += "{}";
            return Ok{};
        }
        out += "{\n";
        std::size_t i = 0;
        for (const auto& [key, member] : object) {
            out += child_pad;
            write_escaped_string(out, key);
            out += ": ";
            if (auto r = write_value(out, member, indent + 1); !r.ok()) {
                return r;
            }
            out += (++i < object.size()) ? ",\n" : "\n";
        }
        out += pad;
        out.push_back('}');
    }
    return Ok{};
}

} // namespace

Result<Value> parse(std::string_view text, std::string source_name) {
    return Parser(text, std::move(source_name)).run();
}

Result<std::string> write_canonical(const Value& value) {
    std::string out;
    if (auto r = write_value(out, value, 0); !r.ok()) {
        return r.take_error();
    }
    out.push_back('\n');
    return out;
}

} // namespace canopy::json
