# System architecture

## Overview

Project Canopy uses a headless domain core surrounded by thin clients and adapters. The desktop modeler, CLI, Python module, exporters, and runtime compiler all consume the same public core services.

```text
Canopy Modeler       Canopy CLI       Python API       Build Worker
      |                   |                |                 |
      +-------------------+----------------+-----------------+
                          |
                    Canopy Application API
                          |
  +-----------------------+------------------------+
  | Document | Graph | Evaluation | Geometry | Assets |
  | Materials | Animation | LOD | Validation | Export |
  +-----------------------+------------------------+
          |              |              |
       Plugins       Cache store      Job system
          |              |              |
  Importers/exporters   Content hashes  Thread pool/distributed queue
                          |
                    Runtime compiler
                          |
                   .canopyrt packages
                          |
             Runtime SDK / engine integrations
```

## Process architecture

### Modeler process

The modeler contains Qt UI code, viewport rendering, input tools, and a local application service. Heavy evaluation and image tasks run through the shared job system. Initially these jobs may run in-process; the architecture must permit an optional worker process for crash isolation and farm execution.

### Headless process

`canopy-cli` loads the same document and plugin registry without linking Qt. It performs validation, deterministic evaluation, image rendering, packaging, and exports. CI and asset farms use this executable.

### Runtime

The runtime SDK is independent of Qt, Lua, OpenUSD, Alembic, FBX, and authoring dependencies. It reads only `.canopyrt` and optional external texture payloads. It provides resource decoding, culling, LOD, wind state, billboards, collision, and renderer-facing command generation.

## Module boundaries

### `canopy_base`

- Fixed-width types, result/error model, logging interface
- UUID and content hash types
- Math primitives and coordinate systems
- Serialization helpers and semantic versioning
- Allocator and job-system abstractions

### `canopy_document`

- Document object model
- Transactions, undo/redo, autosave journal
- External asset references
- Schema migration
- Canonical JSON serialization

### `canopy_graph`

- Generator registry and typed graph
- Property metadata and property paths
- Curves, variance, expressions, rule bindings
- Stable random stream service
- Dependency and dirty-region analysis

### `canopy_eval`

- Immutable evaluation snapshots
- Pass scheduler and cancellation
- Node generation and attachment
- Geometry-force passes and post processing
- Diagnostics and profiling

### `canopy_geometry`

- Spines, frames, sweeps, junctions, cutouts, fronds
- Mesh attributes, topology, simplification, UVs, tangents
- Collision and spatial acceleration structures
- Mesh repair and photogrammetry helpers

### `canopy_material`

- Material graph subset and map semantics
- Image adjustments, masks, material sets
- Texture atlas and channel packer
- Color management and baking

### `canopy_animation`

- Wind rigs and shader semantic generation
- Growth evaluation
- Seasons and leaf drop
- Timeline and cache sampling

### `canopy_lod`

- Continuous LOD policy
- Discrete LOD baking
- Shade pruning and leaf collision
- Billboard/impostor generation

### `canopy_export`

- Export scene intermediate representation
- Presets, validation, manifests
- Built-in open-format writers
- Optional proprietary adapters
- Custom packing and exporter plugins

### `canopy_runtime_compiler`

- Runtime batching and buffer layout
- Texture and billboard packaging
- Flat binary schema writer
- Validation and compression

### `canopy_runtime`

- C ABI and C++ wrapper
- Memory-mapped resources
- Forest cells, culling, LOD, wind
- Draw packet generation and callback interfaces

### `canopy_viewport`

- WebGPU device and resource layer
- PBR and debug render modes
- Picking, overlays, shadows, AO, impostor preview
- No domain-state ownership

### `canopy_modeler`

- Qt main window, dock panels, property editors
- Graph editor, curve editor, cutout editor, timeline
- Tool controllers and command construction
- Session, preferences, layouts, and onboarding

## Data flow

1. A UI command starts a document transaction.
2. The transaction changes graph, property, asset, or edit-layer data.
3. Dirty analysis computes affected generators and passes.
4. The evaluation scheduler creates immutable jobs using a document revision.
5. Jobs publish partial preview snapshots and a final snapshot.
6. The viewport swaps snapshots at frame boundaries.
7. Export obtains or requests a production-profile snapshot, converts it into the export scene IR, and invokes a writer.
8. Runtime compilation consumes the same export scene IR with stricter layout and validation rules.

## Concurrency model

- The authoritative document is mutated only on the application command thread.
- Evaluation reads immutable revision snapshots.
- Geometry and image tasks use a work-stealing job system.
- Publishing uses reference-counted immutable results.
- Cancellation is cooperative and checked at generator, mesh chunk, image tile, and frame boundaries.
- Plugins may not retain references to transient buffers beyond the documented callback scope.
- GPU uploads occur on the viewport render thread from immutable staging packages.

## Error model

Every fallible public operation returns a structured result containing:

- Stable error code
- Human-readable message
- Object, property, file, frame, or generator context
- Severity: info, warning, recoverable error, fatal error
- Suggested actions
- Nested causes

No core library function may present UI or terminate the process.

## Determinism model

Deterministic output requires:

- Named random streams derived from document seed, generator ID, node semantic key, and algorithm version
- Canonical iteration order independent of hash-map layout
- Versioned numerical tolerances and meshing policies
- Explicit coordinate and unit conversion
- Fixed color-management configuration
- Dependency versions captured in export manifests
- No use of wall-clock time, thread scheduling, pointer values, or platform locale in model evaluation

Bitwise identity across CPU architectures is desirable but not required for all floating-point mesh data. Topology, semantic IDs, material assignment, random decisions, and quantized runtime buffers must be identical. Float differences must remain within the tolerance profile and produce the same golden classification.

## Plugin isolation

In-process plugins are allowed only for trusted studio deployments. Public marketplace plugins should support an out-of-process host using a versioned RPC protocol. Importers parsing untrusted formats should be isolatable in helper processes.
