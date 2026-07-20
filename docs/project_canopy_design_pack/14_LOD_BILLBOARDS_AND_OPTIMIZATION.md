# LOD, billboards, collision, and optimization

## Continuous authoring LOD

The modeler exposes a normalized LOD value from highest detail to lowest geometric detail. Each geometry type has reduction curves and behavior rules. Continuous preview allows artists to tune transitions before exporters bake discrete levels.

## Branch reduction

Branch candidates receive an importance score from:

- Hierarchy level
- Screen-space contribution estimate
- Length and radius
- Silhouette exposure
- User retention weight
- Feature vertices and semantic tags
- Season and material importance
- Randomized tie-breaker stream

As LOD decreases:

1. Radial and longitudinal tessellation reduce.
2. Bark detail and shells simplify.
3. Low-importance branches shrink toward their spine.
4. Descendants react according to parent-removal policy.
5. Fully collapsed branches are removed.

The shrink interval prevents a visible pop. Export can encode a morph offset or use dither/crossfade.

## Leaf and frond reduction

Leaves/fronds are ranked by silhouette, area, depth, exposure, user weight, and cluster membership. Removed elements shrink. Survivors can grow within configured limits to preserve canopy density and silhouette.

Material and wind group diversity should remain representative. Selection is deterministic.

## Detail reduction

Decals, knots, fins, shell fragments, mesh details, and projectors use independent curves. Some may bake into normal, height, color, or AO textures at lower LODs.

## Discrete LOD baking

An export preset defines:

- Number of LODs
- Target triangle, screen-error, or quality values
- Transition mode: hard, dither, crossfade, morph
- Per-geometry reduction curves
- Material batching policy
- Whether a final billboard LOD is included

Each LOD is evaluated from the same semantic model, not simplified cumulatively from the previous LOD. This avoids compounded error and supports stable remapping.

## Mesh simplification

Imported and frozen meshes use attribute-aware simplification with constraints for:

- Boundaries and seams
- Material borders
- UV and normal error
- Skinning weights
- Wind anchors
- Feature vertices
- Silhouette weight

The simplifier emits an error metric and remap table.

## Billboard impostors

### Capture configuration

- Azimuth view count
- Optional elevated and top views
- Orthographic or perspective capture
- Atlas size and padding
- Background alpha handling
- Wind pose: rest, representative, or multiple phase frames
- Season state
- Material passes

### Output passes

- Base color and opacity
- Normal
- Roughness/metallic or target-packed channels
- AO
- Subsurface amount
- Depth or parallax data
- Optional shadow or thickness data

### Runtime geometry

The runtime may use:

- Crossed cards
- View-selected single quad
- Octahedral or hemi-octahedral impostor mesh
- Engine-specific billboard renderer

View selection, camera facing, lighting normals, and shadow facing are target-adapter responsibilities.

## Billboard transition

Transition from final geometry LOD to billboard uses crossfade or dither. Bounding dimensions and pivot must match. Optional depth impostors reduce parallax error.

## Shade pruning

Shade pruning is both a realism and optimization process. It calculates exposure and removes interior leaves/fronds according to thresholds. Inverse mode removes exterior elements. The process is included in model and export hashes.

## Collision objects

Generated collision types:

- Capsule chains along trunk and branches
- Spheres
- Boxes
- Convex hulls
- Simplified triangle meshes
- User-authored proxies

Each object has a semantic source, layer, material, and LOD/runtime activation policy. Collision is not inferred from render meshes at runtime unless explicitly requested.

## Ambient occlusion

Per-vertex AO may be computed for the authoring viewport and export. Texture AO can be baked separately. AO settings include ray count, distance, bias, leaf transparency approximation, and deterministic sampling seed.

## Game batching

The game export optimizer:

- Groups geometry by target material and shader feature set
- Packs textures into atlases
- Merges compatible draw ranges
- Supports a one-draw-call profile where target constraints permit
- Preserves geometry-type and wind semantics in vertex data
- Emits meshlets/clusters when selected
- Reports draw calls, triangles, vertices, texture memory, overdraw, and billboard cost

## Overdraw analysis

The viewport provides:

- Alpha-tested overdraw heat map
- Leaf-area versus cutout-area ratio
- Atlas occupancy
- Depth complexity estimate
- Shade-pruning impact
- Billboard coverage and padding diagnostics

## Performance budgets

Presets may specify hard budgets. Validation fails or warns when exceeded:

- Triangle and vertex counts per LOD
- Draw calls
- Material count
- Texture dimensions and memory
- Bone count
- Vertex attribute count/stride
- Billboard atlas size
- Collision primitive count

## Acceptance

- Continuous LOD has no unbounded geometry pops in the reference camera sweep
- Discrete LODs meet target budgets and preserve bounds/pivot
- Removed branches shrink before disappearance
- Survivor leaves compensate canopy silhouette within configured limits
- Billboard PBR passes align pixel-for-pixel
- Billboard shadow orientation is correct in engine reference scenes
- Collision outputs match semantic source and remain stable across material edits
- One-draw-call export passes target importer and shader tests
