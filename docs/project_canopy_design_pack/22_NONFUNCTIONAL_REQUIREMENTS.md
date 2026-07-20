# Nonfunctional requirements

## Quality attributes

Functional parity is insufficient without deterministic generation, responsive editing, robust files, secure extensions and production-scale runtime performance. The following requirements are release-gated.

## Determinism

- Given the same canonical project, dependency lockfile, target profile, seed, plugin set and deterministic build mode, generated topology and semantic attributes are byte-identical on the same architecture.
- Cross-architecture output is semantically identical. Floating-point fields must meet documented tolerances when byte identity is not practical.
- Randomness uses named streams and stable sampling algorithms.
- Parallel scheduling cannot affect object identity, topology ordering or reductions.
- Hash maps whose iteration affects output use stable ordering.
- Timestamps, locale, file enumeration order and host paths do not enter content hashes.
- Determinism tests run across Windows, macOS and Linux.

## Modeler responsiveness

Interactive targets on the reference workstation:

| Operation | Target |
|---|---|
| UI input acknowledgement | Under 50 ms at the 95th percentile |
| Property edit to coarse preview update | Under 100 ms for standard assets |
| Property edit to full preview update | Under 500 ms for standard assets where cacheable |
| Undo or redo metadata operation | Under 100 ms excluding reevaluation |
| Viewport navigation | 60 frames per second at standard preview budgets |
| Cancellation acknowledgement | Under 100 ms for cooperative stages |
| Project open | Under 3 seconds for a 250 MB local project on reference SSD, excluding optional texture upload |

“Standard asset” is a checked-in benchmark with approximately 250,000 preview triangles, 50 generators and 25 materials. Targets are measured, not inferred.

## Evaluation performance

- Evaluation scales across available workers for independent subgraphs.
- A single-thread mode remains supported for diagnosis and determinism comparison.
- Cached reevaluation must skip unchanged stages verified by trace assertions.
- Peak transient memory is bounded by profile.
- Algorithms avoid quadratic behavior in ordinary branch and foliage counts.
- Cancellation checks occur at bounded intervals in expensive loops.

Performance baselines are recorded for small, standard, hero and forest-cluster assets.

## Runtime performance

Reference targets are specified per hardware class in benchmark manifests rather than marketing prose. Minimum release demonstrations include:

- One million placed instances in a population structure
- At least 100,000 simultaneously visible tree or plant instances in a synthetic worst-case view on desktop reference hardware, using GPU-driven culling
- CPU culling of representative mixed forests within the frame budget assigned by the benchmark profile
- Stable LOD and wind with no per-instance heap allocation per frame
- Bounded streaming and no unplanned GPU synchronization

Every result records hardware, driver, resolution, render features, asset set and commit.

## Scalability limits

Documented and tested minimum limits:

- 100,000 generators or explicit nodes in a document, subject to memory
- 2^32-1 vertices per export scene through chunking even where individual formats are smaller
- 64-bit file offsets and asset sizes
- 1,000 materials per document
- 256 named seasons or states, with practical UI presets
- 1,024 wind domains per forest
- Large-world positions beyond float precision through origin rebasing

The application reports configured limits before allocation failure.

## Reliability

- Save uses write-new, flush, checksum and atomic replace.
- Autosave never overwrites the primary project.
- Crash recovery identifies the exact source revision and unsaved transaction range.
- A failed plugin cannot corrupt committed document state.
- Import and export either commit all declared outputs or clearly mark a partial diagnostic directory.
- Schemas are validated before object construction.
- Unknown optional data is preserved when possible.
- Core operations have invariant checks in debug and hardened validation in release.

## Data integrity

- Canonical project files use stable serialization.
- Binary sections include checksums.
- Content-addressed assets verify hashes.
- Cache corruption triggers recomputation rather than project corruption.
- Migration is copy-on-write and preserves the original.
- Every migration has forward fixtures and rollback documentation.
- The CLI can validate an entire project or library recursively.

## Security

Threats include hostile archives, malformed meshes and images, extension code, untrusted scripts, path traversal, decompression bombs, integer overflow, shader compilation abuse and remote registry compromise.

Required controls:

- Input-size and nesting limits
- Checked arithmetic for offsets and counts
- Sandboxed rule scripts
- Permissioned plugins and optional process isolation
- Constrained archive extraction
- TLS and signature verification for configured registries
- OS secure storage for credentials
- Dependency lockfiles and hashes
- No implicit network access during project evaluation
- Fuzzing of file parsers, mesh import, image metadata, runtime loader and RPC
- Coordinated vulnerability-reporting process

## Privacy and telemetry

- Product functionality does not require telemetry.
- Telemetry is disabled unless explicitly enabled by product policy and user choice.
- Collected fields are documented and inspectable.
- Project names, paths, geometry, images, scripts and asset content are never included in diagnostics without explicit action.
- Crash uploads provide a local review and redaction path where feasible.
- Remote library requests contain only data necessary for the configured service.

## Accessibility

- All editor commands are keyboard reachable.
- Focus order is deterministic.
- Controls expose accessible names, roles and values.
- Color is not the only carrier of state.
- UI scaling supports at least 100–200 percent without clipped core controls.
- Themes meet contrast targets for essential text and selection.
- Animation can be reduced.
- Error lists and graph selection synchronize through accessible controls.

## Localization

- UI strings are externalized.
- Project identifiers and property paths remain invariant English-like tokens.
- Numeric parsing in files and CLI is locale independent.
- Display uses locale-aware formatting without changing saved values.
- Bidirectional text and long translations are tested in principal panels.

## Platform support

Authoring:

- 64-bit Windows
- Current supported macOS on Apple silicon; Intel support according to release policy
- Major 64-bit Linux distributions through an AppImage, Flatpak or supported package strategy

Runtime:

- Desktop platforms above
- Mobile and XR through renderer/engine integrations
- Console branches only in licensed build environments
- Headless server builds

Exact OS, compiler and GPU minimums belong in each release manifest and are tested in CI or controlled labs.

## Reproducible builds

- Dependencies are version pinned with hashes.
- Release builds run in declared toolchain containers or images where licensing permits.
- Generated source is reproducible.
- Archives contain a software bill of materials.
- Binaries and packages are signed.
- Build provenance follows a documented attestation format.

## Observability

Structured trace events cover:

- Document load and migration
- Graph invalidation and evaluation stages
- Cache hit and miss
- Geometry statistics
- Export stages
- Runtime load, streaming and culling
- Plugin calls

Tracing is off or low overhead by default. Users can capture a deterministic diagnostic bundle excluding asset content unless requested.

## Maintainability

- Core modules have explicit dependency boundaries enforced by build checks.
- Public headers do not include private UI or renderer dependencies.
- Cyclomatic complexity and duplication checks are advisory with reviewed exceptions.
- Every nontrivial algorithm has a design note, tests and benchmark.
- Ownership and thread-safety are documented on public types.
- Unsupported platform branches cannot silently diverge from schema conformance.

## Documentation

A release requires:

- User manual
- API reference
- File format specifications
- Integration guides
- Tutorial projects
- Migration notes
- Known limitations
- Performance methodology
- Security and extension model
- Third-party notices

Examples are compiled or executed in CI whenever possible.
