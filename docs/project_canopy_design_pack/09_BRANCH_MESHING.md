# Branch meshing and junctions

## Authoritative representation

A branch is authored as a spine plus profiles and semantic annotations. The surface mesh is derived.

```cpp
struct SpineSample {
    Vec3 position;
    Vec3 tangent;
    float arc_length;
    float normalized_length;
    float radius;
    float growth;
    Frame frame;
};

struct BranchSemanticData {
    ObjectId generator_id;
    SemanticId node_id;
    SemanticId parent_node_id;
    float parent_attachment;
    std::vector<SpineControlPoint> controls;
    Curve radius_profile;
    Curve radial_segments_profile;
    MaterialAssignment bark_material;
};
```

Spines may originate from procedural controls, hand drawing, mesh-helper extraction, imported skeletons, or vine solves. Curves use cubic Hermite or Bézier segments with arc-length parameterization.

## Spine construction

The branch spine is computed in this order:

1. Resolve intended length and initial local frame.
2. Create a low-resolution control polyline from growth rules.
3. Apply gravity, phototropism, targets, directional forces, noise, and parent influence.
4. Resolve splits and pruning.
5. Apply geometry-force attraction, avoidance, obstruction, crawling, stopping, or pruning.
6. Fit or update the smooth spline.
7. Apply manual spine edit layers.
8. Resample by curvature, radius change, force contacts, attachment sites, material seams, and export resolution.

Force integration may use fixed-step integration over normalized length. Step count is derived deterministically from length and profile, then clamped by the resolution profile.

## Frame transport

Use rotation-minimizing parallel transport frames to avoid unintended twist. The initial normal comes from the parent surface radial vector, a user orientation, or a stable world-axis fallback. User twist, gnarl, and local roll are applied after transport.

Degenerate tangent changes use a stable fallback axis selected from the least-aligned cardinal axis. Never derive the fallback from pointer or thread order.

## Ring generation

At each spine sample:

- Evaluate radius, flare, taper, seasonal scale, growth scale, and LOD scale.
- Choose radial segment count from screen/error profile or production resolution.
- Evaluate a radial profile: circular, elliptical, lobed, custom curve, or imported cross-section.
- Apply bark displacement and low-frequency gnarl in the local frame.
- Insert feature vertices at requested radial positions.
- Generate UV and semantic data.

Adaptive ring counts require deterministic stitching between rings. Use a generalized zipper triangulation that minimizes aspect ratio while preserving the material seam.

## Radius and flare

Radius can be specified as absolute value, parent-relative value, pipe-model contribution, or a blend. The default branch hierarchy uses a pipe-model-inspired conservation rule with user-controlled exponent and clamps. Root and trunk flare add a base-local contribution that decays with arc length.

The system must allow radius clamping to eliminate random outliers without changing the random stream.

## Junction strategies

### Overlap

Child geometry intersects the parent. Fastest and acceptable for distant or stylized assets. Export may retain separate draw surfaces.

### Collar

The child base forms a flared collar and is clipped slightly inside the parent. Normals are adjusted to minimize the seam. This is the default game strategy.

### Welded ring stitch

A hole is cut in the parent surface and the child base ring is stitched to its boundary. This requires local remeshing and UV seam handling. Use for hero game assets and VFX.

### Implicit remesh

Parent and child surfaces are converted to a local signed-distance representation, unioned, and remeshed with feature constraints. Reproject UV and semantic attributes. Use only for high-quality profiles because it is slower and can disturb bark UV continuity.

### Imported-mesh stitch

A dedicated stitch generator connects a procedural branch to a scanned or sculpted mesh using topology, texture, or vertex blending. See `12_MATERIALS_TEXTURES_AND_PHOTOGRAMMETRY.md`.

## UV generation

### Bark UVs

- `U` follows angle around the ring, with an explicit seam.
- `V` follows physical arc length divided by material tile length.
- Parent-child phase alignment is optional.
- UV patches may replace regions using atlas UV areas.
- Per-node random phase and scale use named streams.
- Texel-density diagnostics compare world area to texture area.

### Lightmap UVs

Generated only for selected export profiles. Charts respect branch boundaries and junction strategy. The unwrapping service must provide padding in final pixels, not only normalized UV units.

## Normals and tangents

- Analytical sweep normals are used before displacement.
- Normals are recomputed or adjusted after local remeshing and high-amplitude displacement.
- Tangents follow the selected tangent-space convention and are generated after final UVs.
- Two-sided leaf/frond normals are handled separately from branch normals.

## Feature vertices

Feature vertices force topology samples at artist-important locations such as bark cracks, cut edges, scan landmarks, attachment points, or shader data boundaries. They may carry:

- Position and normal constraints
- UV constraints
- Color/feature channel values
- Material seam markers
- LOD retention weight

## Caps and cuts

Branch cuts create cap requests. Cap modes:

- Flat n-gon triangulated around a center
- Concentric rings for growth rings
- Convex or concave profile
- Imported cap mesh
- Material-specific cut surface
- Broken or splintered procedural edge

Trim operations preserve enough spine and ring information to regenerate caps at all LODs.

## Bones and wind hierarchy

Each branch records one or more wind/bone anchors along its spine. Vertex data may include:

- Primary and secondary anchor positions
- Branch hierarchy depth
- Normalized position along branch
- Stiffness and flexibility
- Wind group and phase
- Leaf/frond attachment frame
- LOD morph offset

The authoring mesh stores semantic channels without committing to one engine’s packing. Export packing scripts map semantics to target vertex attributes.

## Mesh quality checks

- No NaN or infinite values
- No zero-area triangles above tolerated count
- No non-manifold edges unless explicitly allowed by overlap mode
- Consistent winding
- UV range and overlap diagnostics
- Tangent orthogonality checks
- Junction seam distance threshold
- Bounded aspect-ratio warnings
- Stable topology hash for identical inputs

## Reference pseudocode

```text
function mesh_branch(branch, profile):
    spine = build_spine(branch, profile)
    samples = adaptive_resample(spine, branch.features, profile)
    frames = parallel_transport(samples, branch.initial_frame)
    rings = []

    for sample, frame in zip(samples, frames):
        radius = evaluate_radius(branch, sample)
        count = radial_segment_count(branch, sample, profile)
        rings.append(make_ring(sample, frame, radius, count))

    mesh = stitch_rings(rings, branch.uv_policy)
    mesh = apply_junction(mesh, branch.parent, branch.junction_policy)
    mesh = apply_feature_constraints(mesh, branch.features)
    mesh = compute_normals_tangents(mesh)
    return validate(mesh)
```

## Acceptance scenes

- Straight cylindrical trunk with exact expected topology
- Highly curved branch proving stable frame transport
- Rapid radial-segment transitions
- Deep hierarchy with thousands of junctions
- Root flare intersecting ground
- Trimmed branch with caps across LODs
- Welded hero junction and implicit-remesh junction
- Bark UV continuity and atlas patches
- Cross-platform deterministic topology hash
