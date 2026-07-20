# Executive specification

## Product statement

Project Canopy is a professional vegetation system that combines procedural generation, direct art tools, photogrammetry, animation, optimization, export, and runtime forest rendering in one clean-room platform.

The product must serve both real-time and offline/VFX pipelines without separate modeler editions. A single source document must be able to produce high-detail hero vegetation, optimized game assets, animated caches, runtime packages, billboards, texture atlases, collision proxies, and randomized variants.

## Primary users

- Environment artists building game-ready vegetation
- VFX artists building high-detail or animated plants
- Technical artists defining studio export and shader conventions
- Pipeline engineers automating asset builds
- Engine programmers integrating forest rendering
- Asset-library teams producing reusable species and variant packs

## Critical workflows

### Procedural-first authoring

An artist creates a generator hierarchy, assigns materials and meshes, selects generation modes, edits property curves and variance, and regenerates a family of deterministic variants.

### Non-destructive art direction

An artist selects generated components and applies node offsets, spine edits, trim strokes, bend/displacement brushes, vertex colors, vertex feature data, click-placement, shape controls, and mesh-helper spines without destroying the underlying generator setup.

### Scanned and sculpted hero meshes

An artist imports a photogrammetry or sculpted trunk, aligns and repairs it, creates anchors and helper spines, blends or stitches procedural extensions, transfers or bakes textures, rigs the result for wind, and grows procedural descendants from it.

### Games pipeline

An artist selects a game preset and exports batched materials, texture atlases, several LODs, billboard impostors, collision data, lightmap UVs, wind data, and custom-packed vertex attributes. Engine importers create materials, LOD groups, wind controls, and prefabs/assets.

### VFX pipeline

An artist exports static or animated high-resolution geometry to USD, Alembic, FBX, or OBJ. Wind may use bones, point caches, or time-sampled geometry. Growth may change topology and therefore must export through topology-varying formats.

### Runtime forest pipeline

A technical artist compiles one or more species to the compact runtime format. The application loads base models, instances millions of transforms, performs cell and instance culling, selects LOD, batches draws, animates wind, renders billboards and shadows, and exposes metadata and collision data.

## Product boundaries

### Included

- Complete authoring application and headless CLI
- Equivalent user-visible functionality documented for SpeedTree Modeler 10.2.0
- Equivalent runtime categories documented for SpeedTree Runtime SDK 10
- Open, documented source and runtime formats
- Export to industry-standard formats and engine-specific packages
- Plugin and scripting systems
- Reference shaders and integrations
- An original sample library sufficient for testing and onboarding

### Excluded from clean-room core

- Import or export of proprietary SpeedTree native formats without authorization
- Reuse of SpeedTree sample assets, textures, shaders, UI artwork, names, source, or private documentation
- Claims of pixel-identical output
- Redistribution of third-party SDKs whose licenses prohibit bundling
- A copied commercial vegetation library

## Success measures

The project is accepted only when all release-gate evidence exists:

- Every capability in `02_PARITY_BASELINE.md` has a passing acceptance test or an approved equivalence statement
- Identical seeds produce identical topology and stable asset hashes on all supported authoring platforms, within explicitly versioned floating-point tolerances
- A standard game tree exports with LODs, atlases, billboards, collision, and wind, then imports into Unity, Unreal, and Godot reference projects
- A hero tree exports static USD and animated Alembic/USD wind and growth caches
- A forest reference application renders at least one million instances in a representative synthetic world using GPU-driven culling on supported desktop hardware, with scalable fallback paths
- The modeler remains interactive while background evaluation and texture processing run
- All public formats and APIs are documented and versioned
- Security fuzzing covers all untrusted import and package parsers
- License provenance is recorded for every dependency and sample asset

## Competitive differentiation permitted beyond parity

Parity is the minimum. The design deliberately adds:

- Diffable directory projects alongside packaged documents
- glTF/GLB and Godot support
- A stable C ABI and Python automation API
- Optional GPU compute evaluation for selected operations
- Continuous seasonal animation as an extension, while still supporting single-state export
- Distributed headless build workers
- Content-addressed caches and reproducible asset builds
- A plugin SDK based on documented contracts rather than private headers
- Explicit clean-room provenance and license manifests
