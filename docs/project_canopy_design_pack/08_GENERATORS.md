# Generator specification

## Common generator behavior

Every generator has:

- Stable ID, name, enabled state, compute stage, and tags
- One anatomical parent except the root tree generator
- Zero or more children
- Generation properties controlling count, location, and orientation
- Type-specific geometry or helper properties
- Random stream controls
- Optional season, growth, wind, LOD, material, vertex-color, and force groups
- Per-node override support according to capability
- Diagnostics and statistics

## Generator catalog

### Tree

Root container and global settings. Owns units, extents, randomization policy, dynamic LOD curves, vertex-color enablement, global resolution, model metadata, collision generation, and export defaults. It creates no visible geometry by itself.

### Branch

Creates swept 3D geometry from a spine. Used for trunks, roots, boughs, branches, twigs, stems, and procedural extensions. Supports length/radius profiles, flare, noise, gravity, gnarl, pruning, splits, caps, UV tiling, bark detail, feature vertices, bones, wind, growth, seasons, and LOD.

### Leaf Mesh

Creates individually editable leaf mesh nodes from cutouts or imported meshes. Supports material choice, size, orientation, fold, curl, tumble, collision, shade pruning, wind, growth, seasons, and custom channels.

### Batched Leaf

Creates high-volume leaves in compact batches. It prioritizes compute and render speed over per-leaf editing and topology-changing growth. It still supports placement, material choice, collision, shade pruning, seasons, wind data, and vertex colors.

### Frond

Creates ribbons or repeated blade meshes along a spine. Used for palms, ferns, grasses, conifer sprays, and stylized foliage. Supports width profiles, segments, twisting, curling, folding, gravity, mesh sections, wind, growth, seasons, and LOD.

### Zone

Defines a volume or surface region used to populate grass, shrubs, debris, or other generators. Supports box, sphere, cylinder, spline volume, mesh volume, and mask-driven domains.

### Vine

Creates a constrained flexible spine that can hang, bridge, crawl, wrap, follow a guide, or grow along geometry. Uses deterministic gravity and collision solving. It may emit branch geometry, fronds, leaves, or low-poly ribbons.

### Card

Creates camera-independent or camera-facing rectangular geometry using a material or cutout. Used for foliage clusters, flowers, distant detail, and stylized elements.

### Base

Creates a base plate, mound, root flare helper, or grounding mesh. It can clip or blend nearby roots and supplies a ground contact region.

### Reference

Instantiates or references a reusable mini-hierarchy without duplicating its definition. References can expose selected parameters and can be converted to unique copies.

### Cap

Closes cut branch ends or creates cap details. Supports flat, convex, concave, material-switched, and growth-aware caps.

### Decal

Projects or places a small material patch on a parent surface. Used for moss, scars, lichen, snow, color variation, and bark detail. Supports seasons and LOD.

### Fin

Creates thin extruded or ribbon geometry aligned to a branch. Used for thorns, ridges, buttress details, or stylized effects. Supports growth and LOD.

### Knot

Creates branch-origin detail, holes, scars, or protrusions with local deformation and material blending. It follows parent growth.

### Shell

Creates an offset shell around parent geometry. Used for peeling bark, snow, moss layers, two-tone bark, or stylized outlines. Supports masks, thickness, breakup, and LOD.

### Mesh

Places an imported mesh as authoritative geometry. Supports transforms, materials, anchors, skinning metadata, mesh helpers, vertex edit, wind rigging, and descendants.

### Mesh Converter

Converts procedural output to an editable or optimized mesh representation while retaining semantic links. Used for freezing selected parts, reducing graph complexity, or preparing subdivision workflows.

### Mesh Detail

Places or wraps detail meshes on parent surfaces. Supports scatter masks, normals alignment, wrapping, seasons, and LOD.

### Stitch

Connects imported and procedural geometry through topology, texture, or vertex blending. Strategies are described in the photogrammetry specification.

### Target

Creates a point, orientation, region, or guide used by shape control, forces, hand drawing, vines, and camera tools. It emits no render geometry.

### Spine

Creates helper spine data without branch surface geometry. Used to rig hero meshes, drive descendants, or define guide curves.

### Subdivision

Applies controlled subdivision to selected geometry for VFX or high-detail outputs. It preserves creases, UVs, and semantic boundaries.

### Cage

Defines a control cage for subdivision or deformation. It is visible in edit modes and excluded from normal export.

### Proxy

Provides lightweight preview, collision, attachment, or evaluation geometry for expensive assets. It may be automatically replaced by final geometry in production profiles.

### Legacy Leaf

A compatibility generator for imported old Canopy documents only. New documents use Leaf Mesh or Batched Leaf. It may have reduced growth support and must provide a migration command.

## Shared generation properties

- Mode
- First and last eligible position
- Count, density, spacing, or internode length
- Phase and jitter
- Azimuth, elevation, roll, and alignment
- Parent-normal and world-up weighting
- Probability and pruning masks
- Grouping and cluster settings
- Parent and profile curves
- Random seeds and stream overrides
- Generation stage and force participation

## Attachment frames

The parent exposes a stable local frame at the attachment coordinate:

- Position
- Tangent
- Surface normal or radial direction
- Binormal
- Radius or local scale
- UV and material context
- Parent growth and season state

Children can blend between anatomical, surface, world, and target-based orientations.

## Generator extensibility

A third-party generator plugin must declare:

- Unique reverse-domain type ID
- Schema version and migration functions
- Parent/child compatibility
- Property descriptors
- Evaluation stage requirements
- Output semantics
- Thread-safety and determinism declaration
- Headless availability
- License and publisher metadata

Unknown generators remain preserved in documents as opaque extension objects. The modeler shows them as unavailable rather than deleting them.
