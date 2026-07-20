# Release gates

## Gate philosophy

Releases advance on evidence, not elapsed time or feature counts. A feature is complete only when its workflow, failure behavior, deterministic tests, documentation and required integration evidence are present.

## Foundation gate

Purpose: prove the project can safely evolve.

Required evidence:

- Clean-room contributor and agent provenance process is active.
- Repository dependency boundaries are enforced.
- Windows, macOS and Linux configure and run unit tests.
- Diagnostics, allocation, tasks, cancellation and progress contracts exist.
- Canonical schema, transactions, save and migration prototypes pass data-integrity tests.
- Reproducible dependency lock and SBOM generation work.
- Public API and ADR review workflow is operational.

Exit condition: an intentionally small document can be created, modified, saved, reopened, migrated and validated identically on all authoring platforms.

## Headless Alpha gate

Purpose: prove the procedural product core without relying on UI polish.

Required capabilities:

- Generator DAG and reflected properties
- Curves, variance, generation modes and named random streams
- Immutable deterministic evaluation snapshots
- Incremental cache and invalidation trace
- Core branch, leaf, frond, card and material generation
- Open-format static export through CLI
- Minimal Python automation
- Original golden assets and cross-platform determinism checks

Quality bars:

- No unresolved data-loss defect in supported project operations.
- No critical memory-safety defect in parsers exercised by the alpha.
- Same-architecture deterministic outputs are byte stable for declared fields.
- Every incomplete parity row is visibly marked; alpha is not advertised as full parity.

Exit condition: CI can generate and export a representative tree from source files on every authoring platform with matching manifest and topology hashes.

## Modeler Beta gate

Purpose: complete end-to-end artist workflows.

Required capabilities:

- Full generator catalog
- Generator, node and freehand editing modes
- Bend, displacement, trim, hand draw, click place, vertex edit and painting
- Cutout, UV area, atlas and material-set workflows
- Photogrammetry conversion and scan extension strategies
- Vines, mesh helpers and hero-mesh spines
- Game and VFX wind
- Growth, timeline and seasonal transitions
- Dynamic LOD, collision generation and impostor baking
- Games and VFX export presets
- Rules, custom data packing and command-line batch operation
- Recoverable dockable desktop UI

Quality bars:

- Core workflows pass scripted UI tests.
- Standard asset editing meets responsiveness targets.
- Golden render differences are reviewed.
- Project and package parsers have active fuzz coverage.
- Documentation covers every visible feature.

Exit condition: designated artists complete a broadleaf, conifer, vine, photogrammetry and hero-mesh workflow without source-code changes or manual file repair.

## Runtime Beta gate

Purpose: prove engine-independent runtime use.

Required capabilities:

- Normative `.canopyrt` schema and compiler
- Hardened zero-copy loader
- Stable C ABI
- Base assets and instances
- Sparse cell population
- CPU culling and LOD
- Wind domains
- Draw packets and shader reflection
- Impostors, collision and metadata
- Memory budgets, streaming and large-world support
- WebGPU reference viewer

Quality bars:

- Loader fuzzing finds no unresolved release-blocking defect.
- CPU culling matches brute force for the reference corpus.
- ABI conformance passes on supported platforms.
- One million-instance population stress test succeeds.

Exit condition: an external starter application installed from release packages renders and animates the reference forest without repository-private code.

## Release Candidate gate

Purpose: complete production integrations, scale and compatibility.

Required capabilities:

- GPU-driven culling and indirect drawing
- Shadow helpers and optional terrain module
- Unity, Unreal, Godot and O3DE packages according to declared matrix
- Blender, Maya, Houdini, 3ds Max and Cinema 4D workflows according to declared matrix
- Custom engine starter kit
- Local library, dependency lockfile, variations and provenance
- Optional remote registry
- Plugin trust, signing and isolated extension host
- Complete API, schema and integration documentation

Quality bars:

- Performance profiles meet approved budgets.
- Host import/reimport/build or render tests pass.
- No unresolved blocker or critical security finding.
- All schema migrations and public ABI checks pass.
- Installation, update and uninstall are tested.
- Crash recovery and fault-injection suites pass.

Exit condition: every parity matrix row is either evidenced as complete or recorded as a specific known limitation approved for the candidate.

## General Availability gate

GA requires all items in `32_ACCEPTANCE_CHECKLIST.md` to be signed off with evidence.

Mandatory release evidence:

- Feature-parity traceability report
- Cross-platform test report
- Determinism matrix
- Performance and memory report
- Fuzz and security report
- Public API and ABI compatibility report
- File-schema and migration report
- Engine and DCC compatibility report
- Accessibility report
- SBOM, third-party notices and provenance audit
- Original-content provenance bundle
- Known limitations and migration notes
- Signed installers/packages and build attestations

GA cannot be declared when a missing proprietary file adapter is the only gap; binary compatibility is outside the parity definition. GA can be declared only when equivalent documented workflows and outputs are available through Canopy and open/standard interchange.

## Patch gate

A patch release may contain fixes and compatible performance improvements.

Required checks:

- No public ABI break
- No incompatible schema change
- Regression test for every defect
- Affected golden and performance checks
- Targeted security review for parser or extension changes
- Updated changelog and known issues

## Minor release gate

A minor release may add compatible features.

Additional checks:

- New feature traceability and user documentation
- API additions reviewed for long-term compatibility
- New schema fields optional to older readers or covered by compatible version rules
- Integration packages state minimum core version
- Added dependencies pass full intake review

## Major release gate

A major release may change contracts only with:

- Approved ADRs
- Published migration tools
- Side-by-side compatibility strategy where feasible
- Deprecation history
- Full old-to-new fixture suite
- Engine, DCC and plugin transition guide

## Stop-ship conditions

- Project data loss or silent corruption
- Remote code execution or sandbox escape in default configuration
- Runtime loader memory-safety flaw reachable from an asset
- Nondeterministic topology in deterministic production mode
- Incorrect license/provenance packaging for distributed content
- Installer or updater deleting user projects or assets
- A release artifact not reproducible from the declared source and toolchain
- False claim of proprietary format compatibility
