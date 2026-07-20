// Minimal vector math for the bootstrap slice. Units are meters unless stated;
// coordinate convention: right-handed, +Y up (document manifest declares both).
#pragma once

#include <cmath>

namespace canopy {

struct Vec2 {
    double x = 0.0;
    double y = 0.0;
};

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(double s) const { return {x / s, y / s, z / s}; }
    Vec3& operator+=(const Vec3& o) {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
};

inline double dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

inline double length(const Vec3& v) { return std::sqrt(dot(v, v)); }

// Returns fallback when v is too short to normalize reliably.
inline Vec3 normalize_or(const Vec3& v, const Vec3& fallback, double epsilon = 1e-12) {
    const double len = length(v);
    if (len <= epsilon) {
        return fallback;
    }
    return v / len;
}

inline bool finite(const Vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

} // namespace canopy
