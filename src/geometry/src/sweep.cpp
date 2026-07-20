#include "canopy/geometry/sweep.hpp"

#include "canopy/foundation/hash.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace canopy::geo {

Result<TriangleMesh> sweep_branch(const std::vector<SpineSample>& samples,
                                  const std::vector<Frame>& frames, const SweepOptions& options) {
    if (samples.size() < 2) {
        return Diagnostic::error(ErrorCode::invalid_argument, "sweep requires at least 2 samples");
    }
    if (frames.size() != samples.size()) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "sweep requires one frame per sample");
    }
    if (options.radial_segments < 3) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "radial_segments must be at least 3");
    }
    if (!(options.uv_tile_length_m > 0.0)) {
        return Diagnostic::error(ErrorCode::invalid_argument, "uv_tile_length_m must be positive");
    }
    if (options.lobe_amplitude < 0.0 || options.lobe_amplitude > 0.5) {
        return Diagnostic::error(ErrorCode::invalid_argument,
                                 "lobe_amplitude out of range [0, 0.5]");
    }
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const bool is_tip = i + 1 == samples.size();
        const double radius = samples[i].radius;
        if (!std::isfinite(radius) || radius < 0.0 || (!is_tip && radius <= 0.0)) {
            return Diagnostic::error(ErrorCode::invalid_argument,
                                     "sample " + std::to_string(i) + " has invalid radius");
        }
    }

    const std::uint32_t segments = options.radial_segments;
    // Seam duplication: segments + 1 vertices per ring so U spans [0, 1].
    const std::uint32_t ring_stride = segments + 1;
    const bool point_tip = samples.back().radius <= 0.0;
    const std::size_t ring_count = point_tip ? samples.size() - 1 : samples.size();

    TriangleMesh mesh;
    mesh.positions.reserve(ring_count * ring_stride + (point_tip ? 1 : 0));

    for (std::size_t s = 0; s < ring_count; ++s) {
        const SpineSample& sample = samples[s];
        const Frame& frame = frames[s];
        const double v =
            sample.arc_length / options.uv_tile_length_m + options.uv_v_offset;
        // Effective lobe amplitude at this ring.
        const double amp = options.lobe_count > 0
                               ? options.lobe_amplitude * std::clamp(sample.lobe_scale, 0.0, 1.0)
                               : 0.0;
        for (std::uint32_t k = 0; k <= segments; ++k) {
            const double u = double(k) / double(segments);
            const double angle = 2.0 * std::numbers::pi * u;
            const Vec3 radial =
                frame.normal * std::cos(angle) + frame.binormal * std::sin(angle);
            // Ring tangential direction (dθ of the radial vector).
            const Vec3 tangential =
                frame.normal * -std::sin(angle) + frame.binormal * std::cos(angle);
            double radius = sample.radius;
            Vec3 normal = radial;
            if (amp > 0.0) {
                // r(θ) = R(1 + a·cos(nθ+φ)); in-plane curve normal is
                // r·u − r′·v in the (radial u, tangential v) basis.
                const double lobe_arg =
                    double(options.lobe_count) * angle + options.lobe_phase;
                radius = sample.radius * (1.0 + amp * std::cos(lobe_arg));
                const double radius_derivative = -sample.radius * amp *
                                                 double(options.lobe_count) *
                                                 std::sin(lobe_arg);
                normal = normalize_or(radial * radius - tangential * radius_derivative,
                                      radial);
            }
            mesh.positions.push_back(sample.position + radial * radius);
            mesh.normals.push_back(normal);
            mesh.uvs.push_back(Vec2{u, v});
        }
    }

    auto ring_vertex = [ring_stride](std::size_t ring, std::uint32_t k) {
        return std::uint32_t(ring * ring_stride + k);
    };

    // Ring stitching: two CCW triangles per quad, outward winding.
    for (std::size_t s = 0; s + 1 < ring_count; ++s) {
        for (std::uint32_t k = 0; k < segments; ++k) {
            const std::uint32_t a = ring_vertex(s, k);
            const std::uint32_t b = ring_vertex(s, k + 1);
            const std::uint32_t c = ring_vertex(s + 1, k);
            const std::uint32_t d = ring_vertex(s + 1, k + 1);
            mesh.indices.insert(mesh.indices.end(), {a, b, c});
            mesh.indices.insert(mesh.indices.end(), {b, d, c});
        }
    }

    if (point_tip) {
        // Tip collapses to a single point with the tangent as normal.
        const SpineSample& tip = samples.back();
        const std::uint32_t tip_index = std::uint32_t(mesh.positions.size());
        mesh.positions.push_back(tip.position);
        mesh.normals.push_back(frames.back().tangent);
        mesh.uvs.push_back(Vec2{0.5, tip.arc_length / options.uv_tile_length_m});
        const std::size_t last_ring = ring_count - 1;
        for (std::uint32_t k = 0; k < segments; ++k) {
            mesh.indices.insert(
                mesh.indices.end(),
                {ring_vertex(last_ring, k), ring_vertex(last_ring, k + 1), tip_index});
        }
    } else if (options.cap_tip) {
        // Flat n-gon cap: duplicated ring with the tangent normal, fanned
        // around a center vertex (cap mode "flat" from 09_BRANCH_MESHING.md).
        const SpineSample& tip = samples.back();
        const Frame& frame = frames.back();
        const std::uint32_t cap_start = std::uint32_t(mesh.positions.size());
        for (std::uint32_t k = 0; k <= segments; ++k) {
            const double u = double(k) / double(segments);
            const double angle = 2.0 * std::numbers::pi * u;
            const Vec3 radial =
                frame.normal * std::cos(angle) + frame.binormal * std::sin(angle);
            mesh.positions.push_back(tip.position + radial * tip.radius);
            mesh.normals.push_back(frame.tangent);
            // Cap UVs occupy a unit disc mapped to [0,1]^2.
            mesh.uvs.push_back(
                Vec2{0.5 + 0.5 * std::cos(angle), 0.5 + 0.5 * std::sin(angle)});
        }
        const std::uint32_t center = std::uint32_t(mesh.positions.size());
        mesh.positions.push_back(tip.position);
        mesh.normals.push_back(frame.tangent);
        mesh.uvs.push_back(Vec2{0.5, 0.5});
        for (std::uint32_t k = 0; k < segments; ++k) {
            mesh.indices.insert(mesh.indices.end(),
                                {cap_start + k, cap_start + k + 1, center});
        }
    }

    if (auto valid = validate_mesh(mesh); !valid.ok()) {
        return valid.take_error();
    }
    return mesh;
}

Result<void> validate_mesh(const TriangleMesh& mesh) {
    if (mesh.normals.size() != mesh.positions.size() || mesh.uvs.size() != mesh.positions.size()) {
        return Diagnostic::error(ErrorCode::geometry_invalid,
                                 "mesh attribute streams have mismatched sizes");
    }
    if (mesh.indices.size() % 3 != 0) {
        return Diagnostic::error(ErrorCode::geometry_invalid,
                                 "index count is not a multiple of 3");
    }
    for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
        if (!finite(mesh.positions[i]) || !finite(mesh.normals[i]) ||
            !std::isfinite(mesh.uvs[i].x) || !std::isfinite(mesh.uvs[i].y)) {
            return Diagnostic::error(ErrorCode::geometry_invalid,
                                     "non-finite attribute at vertex " + std::to_string(i));
        }
    }
    for (std::size_t i = 0; i < mesh.indices.size(); ++i) {
        if (mesh.indices[i] >= mesh.positions.size()) {
            return Diagnostic::error(ErrorCode::geometry_invalid,
                                     "index out of range at position " + std::to_string(i));
        }
    }
    for (std::size_t t = 0; t < mesh.indices.size(); t += 3) {
        const std::uint32_t a = mesh.indices[t];
        const std::uint32_t b = mesh.indices[t + 1];
        const std::uint32_t c = mesh.indices[t + 2];
        if (a == b || b == c || a == c) {
            return Diagnostic::error(ErrorCode::geometry_invalid,
                                     "degenerate triangle (repeated vertex) at triangle " +
                                         std::to_string(t / 3));
        }
    }
    return Ok{};
}

namespace {

void hash_u64(Sha256& hasher, std::uint64_t value) {
    std::uint8_t bytes[8];
    for (int i = 0; i < 8; ++i) {
        bytes[i] = std::uint8_t((value >> (8 * i)) & 0xffu);
    }
    hasher.update(bytes, sizeof bytes);
}

// Quantized signed value for geometry hashing: round(value / 1e-6), which is
// stable across platforms for well-separated values.
void hash_quantized(Sha256& hasher, double value) {
    hash_u64(hasher, std::uint64_t(std::int64_t(std::llround(value * 1e6))));
}

} // namespace

std::uint64_t topology_hash(const TriangleMesh& mesh) {
    Sha256 hasher;
    hasher.update(std::string_view("canopy-topology-v1"));
    hash_u64(hasher, mesh.positions.size());
    hash_u64(hasher, mesh.indices.size());
    for (const std::uint32_t index : mesh.indices) {
        hash_u64(hasher, index);
    }
    return hasher.finish().low64();
}

std::uint64_t geometry_hash(const TriangleMesh& mesh) {
    Sha256 hasher;
    hasher.update(std::string_view("canopy-geometry-v1"));
    hash_u64(hasher, mesh.positions.size());
    hash_u64(hasher, mesh.indices.size());
    for (const std::uint32_t index : mesh.indices) {
        hash_u64(hasher, index);
    }
    for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
        hash_quantized(hasher, mesh.positions[i].x);
        hash_quantized(hasher, mesh.positions[i].y);
        hash_quantized(hasher, mesh.positions[i].z);
        hash_quantized(hasher, mesh.normals[i].x);
        hash_quantized(hasher, mesh.normals[i].y);
        hash_quantized(hasher, mesh.normals[i].z);
        hash_quantized(hasher, mesh.uvs[i].x);
        hash_quantized(hasher, mesh.uvs[i].y);
    }
    return hasher.finish().low64();
}

} // namespace canopy::geo
