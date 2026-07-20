# Bootstrap backlog

## Objective

This is the first implementation sequence for converting the design pack into a working repository. It deliberately builds a narrow deterministic vertical slice before the full generator catalog or desktop interface.

Each entry references the stable task in `25_EPICS_AND_TASKS.md`. Expand it through the work-unit template before assignment.

## Stage 0 — Governance and repository rules

### B-001 — Contributor and agent provenance

**Task:** `CAN-0001`

**Deliverables:**

- `GOVERNANCE.md`
- contributor declaration template
- `agent-runs/README.md` and machine-readable run-record schema
- restricted-material policy
- pull-request provenance fields

**Acceptance:** A sample human contribution and a sample agent-assisted contribution validate through CI. Missing provenance fails with an actionable message.

### B-002 — ADR and contract review

**Task:** `CAN-0005`

**Deliverables:**

- `docs/adr/README.md`
- ADR template
- CODEOWNERS or equivalent ownership map
- check linking schema/API changes to an ADR

**Acceptance:** A deliberate public-schema change without an ADR fails the governance check.

### B-003 — Dependency intake

**Task:** `CAN-0004`

**Deliverables:**

- dependency manifest schema
- license/provenance review template
- initial records for build/test dependencies only
- generated notice/SBOM proof of concept

**Acceptance:** Introducing an unregistered dependency fails CI.

## Stage 1 — Buildable skeleton

### B-004 — Monorepo structure

**Task:** `CAN-0101`

**Deliverables:**

```text
apps/modeler/
apps/cli/
apps/runtime-viewer/
src/foundation/
src/document/
src/evaluation/
src/geometry/
src/materials/
src/export/
src/runtime/core/
src/runtime/forest/
src/plugins/
bindings/python/
integrations/
schemas/
tests/
benchmarks/
docs/
```

Create placeholder targets only where needed to verify dependency direction. Do not add feature stubs that pretend to implement behavior.

**Acceptance:** Build graph report shows no UI dependency below application/view-model layers and no authoring dependency in runtime libraries.

### B-005 — Cross-platform CMake presets

**Task:** `CAN-0102`

**Deliverables:** Debug, release, sanitizer, headless and runtime-only presets plus CI workflows.

**Acceptance:** A minimal test executable runs on Windows, macOS and Linux; headless configuration does not find Qt.

### B-006 — Diagnostics and status

**Task:** `CAN-0104`

**Deliverables:** Stable error categories/codes, source locations, nested diagnostics, human and JSON formatting, log sink interface.

**Acceptance:** C++, C and CLI examples report the same machine code for a synthetic invalid-argument case.

### B-007 — Allocator, cancellation, tasks and progress

**Task:** `CAN-0105`

**Deliverables:** Caller allocator, cancellation token/source, progress hierarchy, minimal job-system adapter and deterministic single-thread executor.

**Acceptance:** Allocation failure, cancellation and progress ordering tests pass without leaked tasks or callbacks under internal locks.

### B-008 — Tooling baseline

**Task:** `CAN-0106`

**Deliverables:** Warning policy, formatting, static analysis, ASan/UBSan or platform equivalents, test runner and coverage upload where configured.

**Acceptance:** A deliberate leak/out-of-bounds fixture is detected in a quarantined CI self-test and not present in normal suites.

## Stage 2 — Canonical values and document

### B-009 — Fundamental identifiers and units

**Task:** `CAN-0202`

**Deliverables:** UUID, content hash, semantic/schema version, property path, unit dimensions, typed values and canonical numeric formatting.

**Acceptance:** Cross-platform vectors round-trip and invalid unit/property combinations fail deterministically.

### B-010 — Authoring schema v1 skeleton

**Task:** `CAN-0201`

**Deliverables:** JSON schemas for document header, settings, graph, properties, assets and metadata; minimal/complete fixtures.

**Acceptance:** Official schema validator accepts fixtures and rejects unknown required fields, malformed IDs and invalid units.

### B-011 — Immutable state and transactions

**Task:** `CAN-0203`

**Deliverables:** Document state, revision, transaction, commit/rollback/savepoint and stale-handle behavior.

**Acceptance:** Property-based command sequences yield the same state as a simple reference model.

### B-012 — Typed generator DAG

**Task:** `CAN-0204`

**Deliverables:** Generator descriptor registry, IDs, parent/child links, type/cardinality validation and deterministic traversal.

**Acceptance:** Cycle/type/cardinality fixtures report stable codes and exact offending IDs.

### B-013 — Canonical serialization

**Task:** `CAN-0205`

**Deliverables:** Stable JSON writer/reader, semantic diff and canonical content hash.

**Acceptance:** Repeated save is byte identical across worker count and working directory; semantic diff finds a one-property edit.

### B-014 — Directory project I/O

**Task:** `CAN-0206`

**Deliverables:** `.canopyproj` layout, safe path resolution, atomic save and recovery marker.

**Acceptance:** Fault injection at every write/rename stage never destroys the last valid project.

### B-015 — Package I/O

**Task:** `CAN-0207`

**Deliverables:** `.canopy` safe ZIP package, checksums, extraction limits and virtual read path.

**Acceptance:** Traversal, duplicate path, overlap, truncation and decompression-limit tests fail safely; valid package matches directory project.

### B-016 — Migration framework

**Task:** `CAN-0208`

**Deliverables:** Version dispatcher, copy-on-write migration transaction, warning log and v0-to-v1 synthetic fixture.

**Acceptance:** Migration is repeatable and original bytes remain unchanged after success or injected failure.

### B-017 — Undo and autosave journal

**Task:** `CAN-0210`

**Deliverables:** Command record, undo/redo stack, autosave journal and recovery replay.

**Acceptance:** Graph/property mixed sequence recovers to exact canonical state after simulated process termination.

## Stage 3 — Deterministic evaluation kernel

### B-018 — Named random streams

**Task:** `CAN-0301`

**Deliverables:** Versioned PRNG/sampling API keyed by seed, object, domain and stream name; cross-platform vectors.

**Acceptance:** Adding a new unused stream or changing worker count leaves existing vectors and generated IDs unchanged.

### B-019 — Curves and variance

**Task:** `CAN-0302`

**Deliverables:** Curve keys/tangents/interpolation, relative and absolute lookup, variance distributions and reflected evaluation.

**Acceptance:** Edge, repeated-key, extrapolation, unit and distribution tests pass with documented tolerances.

### B-020 — Dependency and invalidation graph

**Task:** `CAN-0303`

**Deliverables:** Stage dependency declaration, fingerprints, dirty propagation and trace events.

**Acceptance:** A property edit invalidates only its stage and real descendants in the synthetic graph fixture.

### B-021 — Immutable snapshots

**Task:** `CAN-0304`

**Deliverables:** Snapshot lifetime, revision tags, typed products and concurrent read API.

**Acceptance:** A viewport-like reader and exporter-like reader safely consume the same snapshot while a newer revision evaluates.

### B-022 — Stage cache

**Task:** `CAN-0305`

**Deliverables:** Memory/disk content-addressed cache, version namespace, integrity checks and eviction interface.

**Acceptance:** Cold and warm results match; corrupt entry is quarantined and recomputed.

### B-023 — Deterministic scheduler

**Task:** `CAN-0306`

**Deliverables:** Dependency scheduling over caller job system, stable reductions and cancellation.

**Acceptance:** Worker counts 1, 2 and maximum produce matching snapshot hashes and diagnostics ordering.

### B-024 — Evaluation profiles

**Task:** `CAN-0307`

**Deliverables:** Draft, preview, production and export profile schema with requested products and resource budgets.

**Acceptance:** Trace proves draft omits unrequested expensive products while retaining stable design identity.

### B-025 — Node/edit override merge

**Task:** `CAN-0308`

**Deliverables:** Stable generated target keys, override records, merge order, orphan diagnostics and rebind interface.

**Acceptance:** A surviving node retains its override after unrelated randomization; deleted target becomes a preserved orphan.

## Stage 4 — First geometric vertical slice

### B-026 — Curve/spline foundation

**Task:** `CAN-0401`

**Deliverables:** Piecewise curve API, arc length, resampling, closest point, derivatives and robust degenerate behavior.

**Acceptance:** Property-based tests cover straight, curved, repeated and near-zero segments; benchmark baseline is stored.

### B-027 — Parallel-transport frames

**Task:** `CAN-0402`

**Deliverables:** Stable frame initialization/transport, optional twist and frame diagnostics.

**Acceptance:** Near-collinear and reversing fixtures avoid NaNs and minimize unintended roll.

### B-028 — Branch spine sampling

**Task:** `CAN-0403`

**Deliverables:** Generator-independent sampled spine with adaptive error criteria and stable sample IDs.

**Acceptance:** Draft and production profiles change density while source/spine identity and envelope remain consistent.

### B-029 — Branch sweep mesh

**Tasks:** `CAN-0404`, `CAN-0405`, `CAN-0408`

Split into separate work units after interfaces are fixed.

**Deliverables:** Radius/taper profiles, cross-section rings, ring stitching, tip/cap, normals/tangents, material and semantic streams.

**Acceptance:** MinimalTrunk mesh has valid indices, finite attributes, containing bounds, deterministic topology and independently inspectable OBJ export data.

### B-030 — Tree and Branch minimal generator

**Task:** first vertical slice of `CAN-0503`

**Deliverables:** One Tree root, one Branch child generator, interval/absolute placement subset, one bark material reference and one evaluated render mesh.

**Acceptance:** The CLI can create or open `MinimalTrunk.canopyproj`, evaluate it, print stable statistics, save it and export a valid OBJ with matching manifest.

## Bootstrap integration gate

The bootstrap is complete when a clean checkout can execute:

```text
cmake --preset headless-dev
cmake --build --preset headless-dev
ctest --preset headless-dev
canopy-cli validate tests/assets/MinimalTrunk.canopyproj
canopy-cli evaluate tests/assets/MinimalTrunk.canopyproj --profile production --json
canopy-cli export tests/assets/MinimalTrunk.canopyproj --preset tests/presets/obj-diagnostic.json --out build/minimal-trunk
```

Required evidence:

- Cross-platform matching canonical document hash
- Cross-worker matching topology and manifest hash
- Valid OBJ read by an independent parser
- Cold/warm cache equivalence
- Atomic save and recovery tests
- Sanitizer-clean parser/evaluation run
- Provenance records for all source, dependencies and test assets

## Next sequence after bootstrap

Proceed in this order unless an ADR changes dependencies:

1. Branch junctions, UVs and roots
2. Leaf Mesh, Frond and material/cutout vertical slices
3. WebGPU viewport and Qt command/view-model shell
4. Full generation modes and remaining core generators
5. Manual edit layers and tool interaction
6. Wind, growth and seasons
7. LOD, impostors and export formats
8. Runtime compiler, loader and CPU forest
9. GPU forest and integrations
10. Plugin/library ecosystem and final hardening
