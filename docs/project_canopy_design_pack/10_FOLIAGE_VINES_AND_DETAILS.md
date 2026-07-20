# Foliage, fronds, vines, and detail geometry

## Leaf assets

A leaf asset is a cutout or imported mesh plus attachment metadata:

- Source material and texture region
- Polygon vertices and triangles
- Pivot, stem point, and forward/up axes
- Optional bend line or multiple deformation anchors
- Normal policy and thickness
- LOD variants
- Wind masks and custom channels
- Physical size and texel density

## Leaf Mesh generator

The evaluator creates individual semantic nodes. Per-leaf properties include:

- Scale, aspect, and variance
- Roll, pitch, yaw, tumble, and surface alignment
- Fold and curl
- Gravity and season droop
- Wind group, ripple, twitch, and phase
- Growth timing and scale
- Material choice and season lifecycle
- Collision mode and shade-pruning weight
- LOD importance and silhouette weight

Node edits may alter any supported property or transform.

## Batched Leaf generator

Batched leaves use structure-of-arrays storage and produce one or more instance batches before final mesh expansion. They are not individually editable by default. Selection tools may convert chosen leaves into an override batch or a Leaf Mesh generator.

The batched path supports:

- Deterministic placement and material assignment
- GPU-friendly expansion or static export
- Leaf collision and shade pruning
- Seasonal visibility, size, droop, curl, fold, and drop
- Game wind semantics
- LOD clustering and survivor growth

Topology-changing growth is not required for the batched representation. The growth wizard offers deterministic conversion to individual leaves.

## Leaf collision

Collision runs on leaf bounding shapes before final mesh construction, then optionally verifies actual triangles.

Policies:

- Keep all
- Remove lower-priority leaf
- Rotate or slide within limits
- Scale down within limits
- Remove if intersecting branch
- Remove if intersecting selected geometry force

Priority can combine age, generator order, silhouette contribution, material, parent depth, and random tie-breaker. The tie-breaker is a named deterministic stream.

## Shade pruning

Approximate exposure is computed through hemisphere samples, voxel/SDF visibility, ray queries, or a GPU bake. The production reference path must be deterministic on CPU. Leaves or fronds below a threshold are removed, with optional inversion to remove exterior foliage.

Shade pruning records exposure as a semantic scalar so it can also drive material variation or export.

## Fronds

A frond consists of a spine and one or more ribbon sections. Supported section types:

- Flat strip
- V-shaped or folded strip
- Repeated blade mesh
- Multi-strip palm or fern leaflet layout
- Custom cross-section

Properties include width, thickness, segment density, twist, curl, fold, gravity, ripple, texture repetition, mesh-section selection, growth, seasons, wind, and LOD.

Frond manipulation can randomize or art-direct gravity, wind rest pose, and curling without changing placement.

## Cards and clusters

Cards may represent single leaves, flowers, needles, twig clusters, or distant canopy chunks. A cluster builder can render selected geometry to a texture set and replace it with camera-independent cards or an oriented cluster mesh.

Each cluster stores source semantic IDs for traceability and re-baking.

## Fins, decals, knots, shells, and mesh details

### Fins

Thin surfaces attached along a branch spine or surface path. They support width, height, twist, segmentation, growth, and LOD.

### Decals

Surface patches use projection, conforming, or floating-offset modes. They support opacity masks, normal blending, material layering, seasons, and LOD.

### Knots

Knots can deform parent rings, insert a mesh, create a cavity, or blend a material patch. LOD policies retain the silhouette or collapse to normal/height detail.

### Shells

Offset surfaces inherit parent topology or are remeshed. Breakup masks create peeling bark, snow, moss, or stylized secondary color. Prevent z-fighting with physically meaningful thickness and export offsets.

### Mesh details

Imported detail meshes may be placed, projected, wrapped, or aligned to parent normals. Wrapped details adjust normals to the target surface while retaining authored hard edges where requested.

## Projectors

A projector emits procedural detail onto geometry selected by masks and ray direction. Typical uses include moss, snow, twigs, flowers, debris, and surface coloration.

Projector inputs:

- Projection shape and transform
- Direction or nearest-surface mode
- Target generator filters
- Slope, height, curvature, normal, material, and mask conditions
- Density and exclusion radius
- Detail generator or material-layer output

Projection is a deterministic pass after target geometry is available.

## Vine representation

A vine is a centerline with radius, twist, contact, and attachment constraints. The authoring solver uses a deterministic position-based dynamics or constrained-rod formulation.

Constraints include:

- Segment length
- Bend stiffness
- Twist stiffness
- Gravity
- Endpoint pinning
- Guide attraction
- Surface attraction
- Surface non-penetration
- Friction and crawling direction
- Self-collision
- Wind rest-shape influence

## Vine modes

### Hanging

One or more pinned endpoints with gravity and optional slack. Useful for swoops and bridges.

### Crawling

The vine is attracted to and obstructed by a surface. Contact points advance along the intended growth direction with friction.

### Wrapping

The solver seeks stable surface contact around a trunk, pole, or branch. A twist bias controls handedness.

### Guide following

A guide curve contributes attraction while geometry constraints prevent penetration.

### Ground growth

The ground acts as a contact surface with optional slope preference and obstacle avoidance.

### Low-poly game vine

The solved centerline is converted to a ribbon, low-sided tube, or card chain with LOD and packed wind data.

## Vine solve determinism

- Fixed time step and iteration count
- Stable constraint ordering by semantic ID
- Quantized contact cache keys
- No adaptive early exit in production deterministic mode
- Explicit tolerance profile
- Solver version recorded in document and export manifest

## Ground and dropped leaves

Dropped leaves have a final transform generated by ballistic or direct-deposition rules. They can land on the ground plane or selected mesh forces. Once dropped, wind is disabled by default, though an optional debris-wind extension may animate them separately.

## Acceptance scenes

- Broadleaf canopy with individual and batched leaves
- Conifer fronds and needle cards
- Palm fronds with folds and wind
- Dense leaf-collision stress test
- Shade-pruned interior preserving silhouette
- Moss and snow projector tests
- Peeling bark shells and knot details
- Hanging, wrapping, crawling, guided, and ground vines
- Low-poly vine export with valid LOD and wind attributes
