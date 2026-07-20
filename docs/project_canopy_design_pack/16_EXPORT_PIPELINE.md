# Export pipeline

## Export architecture

All exporters consume a renderer-neutral `ExportScene` intermediate representation. This prevents each format writer from reimplementing procedural evaluation, material resolution, LOD baking, or animation sampling.

```cpp
struct ExportScene {
    ExportMetadata metadata;
    CoordinateSystem coordinate_system;
    std::vector<ExportMaterial> materials;
    std::vector<ExportTexture> textures;
    std::vector<ExportLod> lods;
    std::vector<ExportSkeleton> skeletons;
    std::vector<ExportAnimation> animations;
    std::vector<ExportCollision> collisions;
    std::vector<ExportBillboardSet> billboards;
    SemanticChannelDictionary semantics;
};
```

## Export phases

1. Load and migrate document
2. Resolve preset and overrides
3. Validate target capabilities and budgets
4. Evaluate production snapshot or sampled animation frames
5. Bake LODs, collision, lightmap UVs, atlases, and billboards
6. Build `ExportScene`
7. Apply custom semantic packing
8. Write format files and sidecars
9. Run target validator or importer smoke test where available
10. Emit manifest and hashes

## Presets

Preset categories:

### Games

- Unity
- Unity one-draw-call
- Unreal
- Godot
- O3DE/generic engine
- Canopy Runtime SDK
- Generic static mesh

### VFX

- Maya
- 3ds Max
- Houdini
- Blender
- Cinema 4D
- Generic FBX
- Generic USD
- Generic Alembic
- Growth cache
- Wind animation

Presets are JSON documents with inheritance, schema validation, and studio namespaces.

## Built-in formats

### OBJ

Static geometry, materials, and texture references. No wind, bones, rich material semantics, or topology animation. Emit a sidecar for lost semantics.

### glTF/GLB

Static or skinned game assets, PBR materials, LOD extension policy, custom semantic attributes, and optional KTX2 textures. Project Canopy-specific extensions must be documented and optional.

### USD

Static and animated geometry, materials, variants, skeletons, time samples, metadata, and instancing. Use namespaces that do not collide with standard schemas.

### Alembic

Static-topology and topology-changing geometry caches for wind and growth. Material sidecars may be required depending on target DCC.

### FBX

Optional adapter built against the Autodesk FBX SDK. Supports static meshes, skeletons, skinning, materials, and compatible animation/cache workflows. The core distribution does not bundle the SDK unless licensed.

### Diagnostic JSON/XML

Human- and pipeline-readable raw mesh, semantic, material, and hierarchy data for debugging and custom conversion. It is not the authoring format.

### Canopy runtime

`.canopyrt` compact package described in `17_RUNTIME_SDK_AND_FOREST.md`.

## Game export options

- Axis, handedness, units, transform bake
- Number and policy of LODs
- Billboard inclusion and capture settings
- Material batching and one-draw-call mode
- Texture atlas size, format, compression, and channel packing
- Lightmap UV generation
- Collision inclusion
- Meshlets/clusters
- Wind data and quality profile
- Bone or vertex wind representation
- Custom semantic packing script
- Texture embedding or external files
- Metadata and source semantic map inclusion

## VFX export options

- Resolution profile and subdivision
- Static or animated
- Frame range, frame rate, subframes
- Wind, growth, or combined animation
- Skeleton/skinning, point cache, or geometry cache
- UV unwrap mode
- Material assignment and texture bake
- Instances versus expanded geometry
- Vertex colors and arbitrary attributes
- Per-frame topology policy
- Camera and light export

## Custom data packing

Packing scripts run once per vertex or element over documented semantic inputs. The default language is sandboxed Lua.

Available conceptual inputs include:

- Position represented as anchor plus offset
- LOD offset or morph target
- Normal, tangent, binormal
- Primary and lightmap UVs
- Vertex colors and alpha
- Geometry type
- Wind anchors, phases, weights, and hierarchy
- Branch/leaf normalized coordinates
- Material and generator IDs
- Custom painted channels

Outputs may set:

- Position, normal, tangent, binormal
- Named UV sets
- Named color sets
- Custom integer or float attributes where format supports them

The script has no file or network access. Instruction and memory limits prevent pathological scripts.

## Texture packing

Channel-pack recipes are data driven. Example:

```json
{
  "output": "ORM",
  "channels": {
    "r": {"source": "ambient_occlusion"},
    "g": {"source": "roughness"},
    "b": {"source": "metallic"},
    "a": {"constant": 1.0}
  }
}
```

Recipes specify color space, default values, inversion, range remap, compression, and mip policy.

## Command-line interface

```text
canopy-cli validate Oak.canopyproj --profile production
canopy-cli export Oak.canopyproj --preset presets/unreal.json --out build/oak
canopy-cli render Oak.canopyproj --camera turntable --frames 1:120 --out preview
canopy-cli package Oak.canopyproj --out Oak.canopy --embed-assets
canopy-cli compile-runtime Oak.canopyproj --preset presets/runtime-high.json --out Oak.canopyrt
canopy-cli inspect Oak.canopyrt --json
```

Commands return stable exit codes and can emit JSON Lines diagnostics.

## Export manifest

```json
{
  "format": "canopy-export-manifest",
  "tool_version": "1.0.0",
  "source_document_hash": "sha256:...",
  "preset_hash": "sha256:...",
  "schema_versions": {
    "authoring": "1.0.0",
    "runtime": "1.0.0"
  },
  "outputs": [
    {"path": "Oak_LOD0.glb", "sha256": "...", "bytes": 123456}
  ],
  "statistics": {
    "lods": 4,
    "triangles": [85000, 32000, 9000, 12],
    "draw_calls": [3, 3, 2, 1]
  },
  "warnings": []
}
```

## Custom exporter SDK

An exporter plugin receives validated `ExportScene` data and a restricted output service. It declares supported capabilities and target formats. It cannot mutate the source document. Out-of-process mode is preferred for untrusted plugins.

## Export correctness

- All formats pass independent parser validation
- Coordinate transforms are tested with asymmetric reference meshes
- Materials include explicit color-space metadata or sidecars
- Animation sampling uses exact rational frame times
- Output paths are sanitized
- Partial files are written to temporary paths and atomically committed
- Failed exports leave a manifest with diagnostics but no misleading completed outputs
