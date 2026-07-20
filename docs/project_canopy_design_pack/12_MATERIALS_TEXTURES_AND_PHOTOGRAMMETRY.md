# Materials, textures, cutouts, and photogrammetry

## Material model

The internal material model is renderer-neutral and maps to target pipelines through profiles.

Core channels:

- Base color and opacity
- Normal
- Roughness
- Metallic
- Specular level and tint
- Ambient occlusion
- Height/displacement
- Subsurface amount and color
- Transmission/thickness approximation
- Emissive
- Detail normal and detail mask
- Vertex blend and material-layer masks

Flags and parameters:

- Alpha clip, alpha blend, opaque
- Two-sided rendering
- Two-sided normal policy
- Leaf/frond subsurface profile
- Wind shading flags
- UV set and transform
- Texture color spaces
- Mip and sampler policy
- Physical texel density

## Non-destructive image operations

Image assets reference original pixels plus an operation stack:

- Exposure, brightness, contrast, hue, saturation
- Levels and curves
- Channel extraction, inversion, remap, and packing
- Normal conversion and strength
- Height-to-normal
- Alpha threshold and dilation
- Tiling and offset
- Color-space conversion
- Resize and filtering

Operations are evaluated through a tiled image graph and cached by content hash. Export can bake them to chosen formats and bit depths.

## Color management

- Documents specify an OpenColorIO configuration URI and working space.
- Texture inputs declare source color space.
- Data maps bypass color transforms.
- Viewport display transform is independent from stored working values.
- Export presets select target color spaces and file encodings.
- Manifests record configuration and transform identifiers.

## Material sets

A material set groups related materials and provides:

- Weighted selection
- Season curves and optional set-level override
- Variant tags such as wet, dry, young, old, diseased, stylized
- Export profile overrides
- Library metadata

Selection occurs through deterministic weighted sampling. Material changes do not reshuffle geometry random streams.

## Cutout editor

The cutout editor creates leaf, flower, bark-detail, or cluster meshes over a texture.

Capabilities:

- Display one or all cutouts over the texture or atlas
- Add, move, delete, and snap points
- Automatic constrained triangulation
- Remove fully transparent triangles
- Simplify while respecting alpha boundary error
- Generate convex hull, grid, or alpha-contour starting shapes
- Set pivot, stem point, axes, bend lines, anchors, and LOD variants
- Paint or import height segmentation masks
- Show vertex/face count and estimated overdraw
- Validate winding, overlaps, and out-of-bounds UVs

## UV area editor

UV areas define regions of an atlas for procedural branch patches or other tiled geometry. Each region stores rectangle or polygon bounds, orientation, seam behavior, pivot, and physical scale.

## Texture atlas builder

Atlas packing requirements:

- Identical placement across all material channels
- Rotation policy selectable per item
- Configurable pixel gutters and alpha dilation
- Mip-safe padding
- Power-of-two and arbitrary dimensions
- Maximum size and multiple-page output
- Fixed placements for incremental builds
- Separate or packed material styles
- Deterministic output independent of thread order
- Utilization and overdraw reports

Packing uses a deterministic rectangle/polygon algorithm with stable item ordering by explicit priority, dimensions, and asset ID.

## Photogrammetry ingest

The import pipeline accepts high-poly meshes and texture sets through standard formats. It performs:

- Unit, axis, and scale normalization
- Mesh validation and repair report
- Material consolidation
- Texture association
- Optional decimation proxy
- Surface acceleration structure
- Landmark and feature annotation
- Anchor and pivot setup

Automated repair never alters the source asset. It creates a derived mesh and records operations.

## Texture creation from scans

Project Canopy does not need to reproduce a full photogrammetry reconstruction solver. It must support the downstream authoring workflow:

- Import reconstructed mesh, cameras, and source images when available
- Bake color, normal, height, AO, curvature, thickness, and masks to target UVs
- Clone/heal texture seams with non-destructive layers
- Match bark tile textures to scan appearance
- Transfer maps to optimized or procedural geometry

A plugin interface permits integration with external reconstruction tools.

## Procedural conversion

To convert scan geometry into procedural control:

1. Identify trunk/branch centerlines manually or automatically.
2. Fit spines and radius profiles.
3. Compute cross-sections and feature constraints.
4. Associate scan surface regions with procedural coordinates.
5. Generate procedural branch surfaces.
6. Transfer textures and selected high-value features.
7. Compare deviation and expose a heat map.

The scan remains available as reference or proxy.

## Feature vertices

Artists can mark scan landmarks that must survive remeshing: deep cracks, knots, scars, branch sockets, silhouette points, or texture seams. The remesher inserts constraints and gives them high LOD retention weight.

## Extending a scanned trunk

### Bake stitch

Generate connector geometry, bake it into the original trunk texture atlas, and update maps. Best for game assets where minimizing materials and texture pages matters.

### Texture blending

Keep scan and procedural textures separate and blend them in a transition region using vertex weights or a mask. Best for flexible authoring and high quality.

### Vertex blending

Use vertex colors or named blend channels to mix material layers at runtime or export. Best where the target renderer supports material blending.

All strategies must generate a seam-distance and normal-continuity report.

## Branch and twig mesh preparation

Preparation tools:

- Set pivot and primary axis
- Normalize scale
- Create 3D anchors for child growth
- Mark cut planes and attachment regions
- Author wind masks and bone hints
- Create LODs or cutout cards
- Export a corrected mesh plus a sidecar manifest

## Material export

Exporters map internal materials through target profiles. Unsupported features produce explicit warnings and optionally bake to textures. A `.canopymat.json` sidecar may accompany generic mesh formats so DCC add-ons can reconstruct materials consistently.

## Acceptance

- Cutout triangulation never includes fully transparent triangles above threshold
- Atlas placement is identical across channels and repeatable
- Color-space round trips pass reference patches
- Scan-to-procedural transfer meets configurable geometric and texture error limits
- All three stitch modes export correctly
- DCC import helpers reconstruct equivalent PBR materials from sidecars
