# Test strategy

## Principles

Testing must establish behavior, determinism, numerical robustness, interoperability and usability. Image snapshots alone are insufficient; topology, semantic attributes, bounds, manifests, performance traces and failure paths are also asserted.

## Test layers

| Layer | Purpose | Typical cadence |
|---|---|---|
| Unit | Math, curves, distributions, identifiers, parsers and local algorithms | Every change |
| Property-based | Invariants over broad generated inputs | Every change or nightly by cost |
| Component | Generator, mesher, material, exporter, runtime subsystem | Every change |
| Integration | Full document evaluation, GUI workflows, engine and DCC imports | Continuous and scheduled host runs |
| Golden | Stable topology, metadata and reference renders | Every change for selected corpus |
| Fuzz | Parsers, schemas, archives, mesh and image import, RPC | Continuous corpus and nightly campaigns |
| Performance | Time, memory, cache and scalability regressions | Per merge and scheduled hardware lab |
| Usability | Artist workflows and accessibility | Release milestones |

## Test corpus

The repository contains original test assets with narrow purposes:

- `MinimalTrunk`: one branch generator and one material
- `BranchHierarchy`: deep and broad generator topology
- `RootsAndForces`: ground interactions and force fields
- `Broadleaf`: branch, leaf mesh, batched leaf and seasons
- `Conifer`: fronds, needles and high instance counts
- `Palm`: trunk, frond, leaf bases and wind
- `VineWall`: procedural and guided vines with collision
- `ScannedBase`: synthetic photogrammetry mesh, conversion and stitch
- `HeroMesh`: spine rigging, anchors, vertex edits and details
- `GrowthPlant`: topology-stable and topology-changing growth cases
- `RuntimeForest`: mixed species, impostors, wind domains and large coordinates
- `Pathological`: near-zero radii, extreme curves, coincident points and malformed inputs

Each asset has a provenance record and expected statistics.

## Unit tests

Required math coverage:

- Vector, matrix, quaternion and frame conversions
- Robust predicates used by meshing
- Curve interpolation and extrapolation
- Arc-length parameterization
- Parallel transport frames
- Random distributions and named streams
- Unit conversion
- Bounds and projected-size calculations
- Hash and canonical serialization

Tests use exact comparisons for discrete behavior and explicit tolerances for floating-point behavior. Tolerances derive from algorithm scale, not arbitrary broad epsilon values.

## Generator contract tests

Every generator type runs a shared suite:

- Valid empty/default evaluation
- Stable object identifiers
- Deterministic output across worker counts
- Correct parent-type acceptance and rejection
- Property descriptor completeness
- Minimum and maximum values
- Curves and variance
- Node override survival
- Season channels
- LOD response
- Serialization and migration
- Copy, paste, duplicate and delete
- Missing dependency diagnostics
- Cancellation

Type-specific tests then assert geometry and semantics.

## Geometry invariants

For each generated mesh:

- Indices are in range.
- Positions and attributes are finite.
- Degenerate triangles remain below the allowed threshold or are intentionally tagged.
- Winding is consistent within manifold regions.
- Normals and tangents are normalized within tolerance.
- UVs meet declared range and packing constraints.
- Material ranges cover indices exactly once where required.
- Vertex semantic streams have matching counts.
- Bounds contain all positions.
- LOD mappings reference valid source entities.
- Skin weights sum correctly.

Manifoldness is asserted only for geometry classes that promise it.

## Property-based testing

Generated random documents exercise:

- Graph creation and deletion sequences
- Undo and redo equivalence
- Serialization round trips
- Curves with repeated or extreme keys
- Branch skeletons and taper profiles
- Collision and force combinations
- LOD simplification
- Atlas packing
- Runtime section layouts

Shrinking produces a minimal failing document stored in the regression corpus.

## Determinism suite

A matrix runs every golden asset with:

- Worker counts 1, 2 and maximum
- Debug and release builds where applicable
- Windows, macOS and Linux
- Supported x86-64 and ARM64 systems
- Cold and warm caches
- Different process working directories

The suite compares canonical graph data, stable IDs, topology, integer attributes, manifests and hashes. Floating data uses field-specific tolerances and reports maximum error with object identity.

## Golden data

Golden artifacts contain compact declarative expectations rather than opaque binary dumps when practical:

```yaml
asset: Broadleaf
profile: production
expected:
  generators: 18
  nodes_range: [1200, 1300]
  lod_triangles:
    - [78000, 82000]
    - [30000, 34000]
    - [7000, 9500]
  material_slots: 3
  has_wind: true
  has_impostor: true
```

Exact topology hashes are used for algorithms declared stable. Intentional changes require a reviewed golden update explaining why.

## Render tests

Reference rendering covers:

- Standard PBR views
- Albedo, roughness, normals, ambient occlusion, subsurface and UV diagnostics
- Alpha-tested leaves
- Two-sided lighting
- Wind at fixed times
- Growth at fixed ages
- Seasonal states
- LOD transitions
- Billboards and impostors
- Motion vectors

Captures use fixed camera, lighting, renderer version and exposure. Comparison combines structural similarity, absolute pixel thresholds and semantic buffers. A changed image is never accepted solely by raising tolerance.

## Editor tests

Automated UI tests cover:

- New/open/save/save-as/recovery
- Generator graph edits
- Property, curve and variance editing
- Selection synchronization between graph and viewport
- Freehand bend, trim, draw, click-place and vertex edit
- Cutout and atlas editors
- Season and timeline controls
- Undo/redo across mixed operations
- Plugin failure recovery
- High-DPI layout and keyboard navigation

Core behavior is tested below the UI so GUI automation focuses on wiring and workflow.

## Import/export tests

For each format:

- Export validates with an independent official or reference parser.
- Imported scene statistics and attributes match the manifest.
- Axes, units, winding, UV origin and tangents use asymmetric fixtures.
- Textures have expected channels and color spaces.
- Animation frame ranges and times match.
- Names are unique and stable.
- Repeated export is deterministic.
- Cancellation leaves no false completed outputs.

USD tests use official OpenUSD validation. glTF uses Khronos schemas and validators. Alembic uses its reference library. Optional FBX tests run only in licensed CI environments.

## Engine and DCC tests

Host integrations run scripted smoke projects. They import a fixed export package, render a reference view, save, reopen, reimport and build or batch-render where supported.

Failures record host version, package version, logs, screenshots and imported asset metadata. Tests never rely only on the host reporting a successful import.

## Runtime tests

- Loader conformance and malformed data
- Memory mapping and copied data modes
- Species and instance lifetime
- Cell insertion, movement and removal
- Camera convention conversion
- CPU culling against brute-force visibility
- LOD threshold, hysteresis and fade
- Shadow views
- Wind state progression and no-wind initialization
- Billboard selection
- Large-world origin shifts
- Streaming cancellation and eviction
- Renderer-neutral draw packet contents
- Reference backend render captures

GPU culling is compared to CPU results using captured buffers.

## Fuzzing

Targets include:

- Canonical JSON project parser
- ZIP and asset package extraction
- `.canopyrt` loader
- OBJ, glTF, USD adapter boundaries and custom mesh import
- Image headers and metadata
- Rule parser and bytecode interface
- Plugin manifest and RPC messages
- Curves and geometry builder inputs

Fuzzers use sanitizers, integer-overflow checks and memory limits. Every unique crash becomes a minimized regression input.

## Fault injection

Tests inject:

- Allocation failure
- Disk full and permission errors
- Truncated writes
- Corrupt cache
- Plugin crash or hang
- Lost registry connection
- Cancel at every major stage
- GPU resource creation failure
- Worker task failure

The expected result is a structured error and preserved last committed state.

## Performance tests

Benchmarks report distributions, not one sample. Metrics include:

- Cold and warm document open
- Full and incremental evaluation
- Peak and retained memory
- Geometry generation by class
- Atlas and impostor build
- Export throughput
- Runtime asset load
- Population mutation
- CPU and GPU culling
- Draw packet generation
- Frame time and GPU memory

A regression budget is attached to each benchmark. Changes exceeding it require review and a recorded reason.

## Accessibility tests

Automated checks inspect accessible names, roles, focus order and contrast. Manual release checks cover full keyboard workflows, screen-reader announcement of diagnostics and scaled layouts.

## Release evidence

The test system emits a signed release evidence bundle containing:

- Commit and toolchain IDs
- Supported platform matrix
- Unit, integration and host-test results
- Golden changes
- Fuzz status and known open findings
- Performance tables
- Schema and ABI conformance
- Dependency and license scans
- Security checks

A release gate consumes this bundle rather than relying on an informal statement that tests passed.
