# Data model and file formats

## Authoring representations

Project Canopy supports two equivalent authoring representations.

### Directory form: `.canopyproj`

A directory intended for source control and AI-agent manipulation.

```text
Oak.canopyproj/
  manifest.json
  graph.json
  properties.json
  edits.json
  materials.json
  animation.json
  export_presets.json
  metadata.json
  assets/
    textures/
    meshes/
    masks/
  rules/
  packing/
  thumbnails/
```

### Packaged form: `.canopy`

A ZIP64-compatible package containing the same canonical paths, optional compression, and a package index. Packaging never changes semantic content. Unpack then repack with the same options must preserve canonical hashes except for the outer archive container.

## Runtime representation: `.canopyrt`

A compact, versioned, memory-mappable binary. It contains no authoring graph or manual edit history unless optional debug metadata is selected. It is generated from an immutable production evaluation snapshot.

## Stable identity

Every persistent object uses a 128-bit ID:

- Document
- Generator
- Scene object
- Material and material set
- Mesh, image, mask, displacement source
- Manual edit layer and edit operation
- Rule and export preset
- Camera and timeline track

Generated nodes use semantic IDs derived from:

```text
hash(document_seed,
     generator_id,
     parent_semantic_id,
     generation_mode,
     stable_ordinal_key,
     algorithm_version)
```

The stable ordinal key is based on sorted generation candidates and not container iteration order. It must remain unchanged when unrelated siblings are added.

## Property model

A property definition contains:

- Stable key such as `spine.length.absolute`
- Display label and group
- Type and units
- Default and legal range
- Whether variance, parent curve, profile curve, animation, rules, or node overrides are permitted
- Recompute scope
- Serialization and migration hooks
- Documentation URI

A property value is layered in this order:

1. Type default
2. Generator base value
3. Material or profile contribution where applicable
4. Rule or expression output
5. Parent-curve multiplier or offset
6. Profile-curve multiplier or offset
7. Deterministic variance
8. Node override
9. Manual edit-layer result
10. Animation/timeline value for the sampled frame

The precise combination mode is property metadata, not an implicit convention.

## Curves

Curves use normalized or physical-domain X coordinates and support:

- Linear, smoothstep, cubic Hermite, constant, and monotone cubic interpolation
- Clamped, repeated, mirrored, or extrapolated boundaries
- Named presets
- Point IDs to support merge and collaboration tools
- Absolute and relative parent evaluation
- Deterministic sampling and optional adaptive lookup tables

## Variance

Supported distributions:

- Uniform
- Normal with explicit clamp
- Triangular
- Log-normal with explicit clamp
- Discrete weighted choices
- Correlated vector or color variation

Every variance source names a random stream. A change to one property must not reshuffle unrelated random decisions.

## Node overrides

A node override refers to a semantic node ID and contains property offsets or replacement values. It may include remapping hints:

- Generator ID
- Parent semantic path
- Normalized attachment coordinate
- Azimuth/elevation
- Local neighborhood signature

If regeneration removes a node, the system attempts deterministic remapping only when confidence exceeds a configurable threshold. Otherwise the override becomes orphaned and is shown to the user.

## Manual edit layers

An edit layer is an ordered list of typed operations:

```json
{
  "id": "2fd2c0f2-9a23-4f68-9f47-8b6ce77e1653",
  "name": "Hero silhouette pass",
  "enabled": true,
  "target_query": {
    "generator": "8d9745d0-4551-45f1-a91c-c48901e410fe",
    "semantic_ids": ["b6d..."],
    "selection_fallback": "spatial_signature"
  },
  "operations": [
    {
      "type": "spine_control_delta",
      "control_id": "cp:4",
      "translation_m": [0.03, -0.01, 0.12]
    }
  ]
}
```

Brush strokes store resampled points in object space, pressure, radius, falloff, and tool parameters. They do not store raw screen coordinates as the authoritative edit.

## Asset references

An asset reference contains:

- Relative or package URI
- SHA-256 or stronger content hash
- Media type
- Color space for images
- Import settings and importer version
- Optional source URI and license metadata
- Last-known dimensions and bounds

The evaluator resolves by content hash first, then URI. Missing or changed assets produce explicit diagnostics.

## Canonical JSON

Canonical serialization requirements:

- UTF-8
- Sorted object keys
- Stable array order by semantic order or explicit ID
- Decimal floating-point encoding with enough precision for round trip
- No locale-sensitive formatting
- No comments in canonical files
- Unknown extension fields preserved under namespaced keys

Human-authored comments may live in adjacent Markdown or metadata fields, not in canonical JSON.

## Manifest example

```json
{
  "format": "canopy-authoring",
  "schema_version": "1.0.0",
  "document_id": "37f7d1ee-c8a8-4d3e-8ba9-3c3a77e40533",
  "name": "Coastal Pohutukawa",
  "units": "meter",
  "up_axis": "Y",
  "handedness": "right",
  "seed": 182736451,
  "color_management": {
    "config": "ocio://studio/default",
    "working_space": "ACEScg"
  },
  "engine_algorithm_set": "canopy-1",
  "files": {
    "graph": "graph.json",
    "properties": "properties.json",
    "edits": "edits.json",
    "materials": "materials.json",
    "animation": "animation.json"
  }
}
```

## Runtime file sections

- Header, version, endianness, feature flags, content hash
- String and metadata tables
- Coordinate-system and bounds data
- Material and texture descriptors
- LOD descriptors
- Vertex and index buffer views
- Meshlet or cluster data where enabled
- Billboard geometry and atlas descriptors
- Wind constants and per-vertex semantic layouts
- Skeleton and bone data
- Collision objects
- Species variation ranges
- Optional debug semantic-ID map

Sections are individually aligned and checksummed. Readers validate all offsets and lengths before use.

## Schema evolution

- Major version: incompatible reader change
- Minor version: additive fields or sections with defaults
- Patch version: documentation or validation clarification
- Readers must ignore unknown optional sections and reject unknown required feature flags
- Migration tests load every checked-in historical fixture
- Runtime writers may target selected older minor versions when no required feature is lost
