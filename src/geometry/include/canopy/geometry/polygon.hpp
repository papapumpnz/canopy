// Simple-polygon triangulation for leaf cutout outlines (ADR-0004,
// 10_FOLIAGE_VINES_AND_DETAILS.md "Leaf assets").
//
// Ear clipping over a simple (non-self-intersecting) polygon; winding may be
// CW or CCW. Deterministic: ears are clipped in fixed index order. Rejects
// degenerate input (< 3 vertices, non-finite, or no clippable ear — which is
// how self-intersecting outlines surface) with a diagnostic.
#pragma once

#include "canopy/foundation/diagnostics.hpp"
#include "canopy/foundation/vec.hpp"

#include <cstdint>
#include <vector>

namespace canopy::geo {

// Returns index triples into `outline` (CCW in the polygon's own plane);
// exactly (n - 2) triangles on success.
Result<std::vector<std::uint32_t>> triangulate_polygon(const std::vector<Vec2>& outline);

} // namespace canopy::geo
