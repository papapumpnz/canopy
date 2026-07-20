# Product requirements

Requirements use stable IDs so agents can reference them in issues, code, tests, and release notes.

## Document and project management

- **DOC-001** The application shall create, open, save, package, unpack, validate, migrate, and recover Canopy documents.
- **DOC-002** The unpacked project form shall be canonical, deterministic, and suitable for version control.
- **DOC-003** Autosave and crash recovery shall never overwrite the last explicit save.
- **DOC-004** External assets shall be stored by relative URI plus content hash and may be embedded during packaging.
- **DOC-005** Schema migration shall be explicit, logged, testable, and reversible when no information is discarded.
- **DOC-006** Documents shall record units, up axis, handedness, color-management configuration, and seed policy.

## Procedural graph

- **GRF-001** A model shall contain a rooted typed generator graph with stable object IDs.
- **GRF-002** The graph shall reject cycles except within explicitly modeled iterative solvers that are not graph dependencies.
- **GRF-003** Each generator shall define input contracts, child-attachment rules, properties, compute passes, outputs, and diagnostics.
- **GRF-004** Property values shall support constants, curves, variance, expressions, rules, animation, and node overrides.
- **GRF-005** Evaluation shall be deterministic for a versioned engine build, model, seed, and target profile.
- **GRF-006** Dirty propagation shall recompute only affected outputs and dependent passes.
- **GRF-007** Long evaluations shall be cancellable and shall publish immutable preview snapshots.

## Geometry

- **GEO-001** Branch geometry shall be generated from editable spines and radial profiles with adaptive tessellation.
- **GEO-002** Junctions shall support collar, welded, overlap, and high-quality remesh strategies.
- **GEO-003** Leaves, cards, fronds, fins, decals, knots, shells, caps, and imported meshes shall preserve material and attachment metadata.
- **GEO-004** Geometry shall carry normals, tangents, UV sets, color sets, custom feature channels, wind data, LOD data, and stable semantic IDs.
- **GEO-005** The system shall generate collision primitives and custom collision meshes.
- **GEO-006** All destructive mesh operations shall operate on derived results, not the authoritative procedural model.

## Editing

- **EDT-001** Generator editing and per-node editing shall be separate, visible modes.
- **EDT-002** Manual edits shall be stored as ordered, non-destructive edit layers with deterministic replay.
- **EDT-003** Undo and redo shall cover graph, property, asset, manual edit, material, and export-preset changes.
- **EDT-004** Trim, hand draw, bend, displacement, vertex edit, vertex paint, click place, and shape controls shall be supported.
- **EDT-005** Lost references caused by regeneration shall be reported and offered deterministic remapping or explicit deletion.

## Materials and images

- **MAT-001** Materials shall support PBR maps, opacity, two-sided shading, subsurface/transmission approximations, displacement, and vertex blending.
- **MAT-002** Image adjustments shall remain non-destructive until bake or export.
- **MAT-003** The cutout editor shall create and optimize 2D meshes from texture regions.
- **MAT-004** The atlas builder shall pack multiple channels consistently and preserve gutters, alpha dilation, mips, and color space.
- **MAT-005** Material sets shall support weighted selection, seasons, variants, and export overrides.

## Animation

- **ANM-001** Game wind shall be evaluated primarily in vertex shaders from packed semantic data.
- **ANM-002** VFX wind shall export through bones, static-topology caches, or time-sampled geometry.
- **ANM-003** Growth shall support topology changes and parent-relative or in-place timing.
- **ANM-004** Seasons shall affect material choice, visibility, size, gravity, curl, fold, and drop timing where relevant.
- **ANM-005** The timeline shall preview wind, growth, scripted parameters, cameras, and render sequences.

## Optimization and export

- **OPT-001** Continuous LOD shall reduce geometry while preserving silhouette and suppressing visible popping.
- **OPT-002** Exporters shall bake discrete LODs and optional crossfade or morph data.
- **OPT-003** Billboards shall include configurable view counts and PBR pass atlases.
- **OPT-004** Shade pruning and leaf collision shall run as deterministic post processes.
- **EXP-001** Exports shall be preset-driven, reproducible, and available through GUI, CLI, C++, Python, and build-worker APIs.
- **EXP-002** Static export shall support OBJ, USD, glTF, FBX when the optional SDK is installed, and diagnostic JSON/XML.
- **EXP-003** Animated export shall support USD and Alembic, plus FBX skinning or cache modes where feasible.
- **EXP-004** Custom packing scripts shall access documented semantic inputs and emit named vertex attributes.
- **EXP-005** Every export shall emit a machine-readable manifest containing source hash, preset hash, tool version, dependencies, warnings, and output hashes.

## Runtime

- **RT-001** Runtime assets shall be memory-mappable and endian/version checked.
- **RT-002** The runtime shall expose a stable C ABI and an idiomatic C++ wrapper.
- **RT-003** Base models, instances, materials, textures, LODs, billboards, wind, collision, bones, and metadata shall be queryable.
- **RT-004** Forest culling shall support cell, frustum, distance, occlusion-hook, and per-instance LOD evaluation.
- **RT-005** The reference renderer shall support GPU-driven batching and indirect draws with CPU fallback.
- **RT-006** Applications shall be able to replace memory, logging, file, job-system, texture, and graphics callbacks.
- **RT-007** Runtime coordinate-system conversion shall be explicit and testable.

## Extensibility

- **EXT-001** Plugins shall declare ABI/API compatibility and capabilities in a manifest.
- **EXT-002** Generator, importer, exporter, image processor, validator, and UI extension points shall be documented.
- **EXT-003** Lua scripts shall be sandboxed and deterministic by default.
- **EXT-004** Python automation shall expose documents, properties, evaluation, validation, export, and asset-catalog functions.
- **EXT-005** Headless use shall not load Qt or viewport dependencies.

## Quality

- **QLT-001** All parsers that accept untrusted data shall have fuzz targets.
- **QLT-002** Numerical and image regressions shall be tracked through golden tests with versioned tolerances.
- **QLT-003** Core algorithms shall be free of undefined behavior under sanitizers.
- **QLT-004** The application shall provide accessible keyboard navigation, high-DPI scaling, and configurable contrast.
- **QLT-005** Telemetry shall be absent by default; any optional telemetry shall be explicit, minimal, documented, and disableable at build and runtime.
