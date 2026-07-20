# Architecture decision records

## ADR-001 — Headless core with GUI client

**Status:** Accepted

**Decision:** Document, evaluation, geometry, material, animation and export behavior resides in headless libraries. The Qt Modeler invokes commands and consumes immutable snapshots.

**Rationale:** Deterministic CI, scripting, farm operation and testability require all substantive behavior without a GUI.

**Consequences:** UI prototypes cannot bypass transaction or command APIs. More explicit view-model work is required.

## ADR-002 — C++20 core and stable C ABI

**Status:** Accepted

**Decision:** Implement performance-sensitive authoring and runtime code in C++20. Expose runtime and plugin compatibility through C ABIs with opaque handles and sized structures.

**Rationale:** C++ provides control over geometry, memory and cross-platform deployment; C is a practical long-lived binary boundary.

**Consequences:** Public C++ surface remains deliberately small. ABI conformance tooling is mandatory.

## ADR-003 — Qt 6 Widgets for the desktop Modeler

**Status:** Accepted, subject to final license intake

**Decision:** Use Qt 6 Widgets for dockable professional desktop UI. Do not make Qt a dependency of core or runtime libraries.

**Rationale:** Mature cross-platform docking, accessibility, input and native integration fit a dense authoring application.

**Consequences:** License/distribution strategy must be selected before commercialization. UI extensions use host abstractions rather than raw Qt ABI.

## ADR-004 — WebGPU viewport through `wgpu-native`

**Status:** Accepted, with backend feasibility gate

**Decision:** Build the reference Modeler viewport and runtime sample on WebGPU through a native implementation. Engine integrations use engine-native shaders and rendering.

**Rationale:** One modern explicit graphics abstraction reduces authoring-platform backend duplication and supports compute-driven features.

**Consequences:** Platform and driver conformance must be measured early. Renderer-neutral core interfaces prevent lock-in.

## ADR-005 — Canonical JSON project and ZIP package

**Status:** Accepted

**Decision:** `.canopyproj` is an unpacked directory with canonical JSON and external assets. `.canopy` packages the same logical content in a safe ZIP container.

**Rationale:** Text review and source control are critical; packaging is needed for transfer and embedded dependencies.

**Consequences:** Large numeric arrays and images remain binary payloads. Canonical serialization and archive hardening are required.

## ADR-006 — Independent compact runtime format

**Status:** Accepted

**Decision:** `.canopyrt` is a sectioned, memory-mappable, renderer-neutral format derived from a normalized export scene.

**Rationale:** Runtime use needs fast safe access and custom vertex layouts without authoring dependencies.

**Consequences:** A compiler and schema conformance suite are first-class products. Proprietary runtime formats are not parsed.

## ADR-007 — Deterministic named random streams

**Status:** Accepted

**Decision:** Random samples are keyed by document seed, object identity, property/sample domain and stable stream name.

**Rationale:** Adding a feature must not reshuffle unrelated vegetation, and parallel order must not affect results.

**Consequences:** Algorithms cannot consume a shared sequence casually. Sampling versions must be explicit.

## ADR-008 — Immutable evaluation snapshots

**Status:** Accepted

**Decision:** Evaluation produces immutable, revision-tagged snapshots; editing creates new document revisions.

**Rationale:** This simplifies viewport concurrency, export consistency, cancellation and cache reuse.

**Consequences:** Snapshot memory must be shared efficiently. UI must discard stale asynchronous results.

## ADR-009 — Typed DAG with anatomical semantics

**Status:** Accepted

**Decision:** The generator structure is a validated typed DAG, while generated plant anatomy is represented by stable node/spine entities.

**Rationale:** Reusable references require DAG behavior; generator links still need clear parent-child biological semantics.

**Consequences:** Cycle detection, type contracts and instance-context IDs are mandatory.

## ADR-010 — Non-destructive edit layers

**Status:** Accepted

**Decision:** Bend, trim, displacement, paint, hand drawing and vertex edits are explicit layers keyed to stable entities and merged during evaluation.

**Rationale:** Procedural regeneration and manual art direction must coexist with undo and diagnostics.

**Consequences:** Orphan/rebind behavior is part of the data model. Some topology-changing edits require explicit conversion.

## ADR-011 — Open formats are canonical interchange

**Status:** Accepted

**Decision:** glTF, OpenUSD, Alembic and OBJ cover primary real-time, VFX and diagnostic exchange. FBX is optional and isolated.

**Rationale:** Open specifications improve portability, validation and clean-room independence.

**Consequences:** Target-specific metadata may use documented extensions/sidecars. Some legacy pipelines require the optional adapter.

## ADR-012 — One normalized ExportScene

**Status:** Accepted

**Decision:** All exporters consume an immutable `ExportScene` after coordinate, material, animation, LOD and packing normalization.

**Rationale:** Format plugins should encode, not reinterpret modeling rules.

**Consequences:** The normalization stage is large and heavily tested, but format behavior becomes consistent.

## ADR-013 — Sandboxed Lua for Rules and packing

**Status:** Accepted

**Decision:** Use a Lua 5.4-compatible sandbox for artist Rules and bounded data packing. Filesystem, network, process and native loading are absent by default.

**Rationale:** Lua is compact, embeddable and suitable for deterministic control scripts.

**Consequences:** Host API and resource limits require strong design. Rules are not general pipeline automation.

## ADR-014 — Python for trusted automation

**Status:** Accepted

**Decision:** Provide Python bindings over the public authoring SDK and use CLI as the lowest common automation interface.

**Rationale:** Studios and DCC tools broadly use Python.

**Consequences:** Python cannot be a runtime or project-open dependency. Ownership and threading rules need binding tests.

## ADR-015 — Native plugins use C ABI and optional isolation

**Status:** Accepted

**Decision:** Native capabilities negotiate through a stable C ABI. Untrusted or crash-prone extensions run in a separate host process.

**Rationale:** Extensibility is necessary, but in-process C++ ABI and project-triggered code are unsafe compatibility boundaries.

**Consequences:** RPC and shared-memory transfer add complexity. Direct Qt extension is not stable.

## ADR-016 — Sparse hashed grid as reference forest structure

**Status:** Accepted

**Decision:** The default forest population uses sparse cells with optional dense subcells. Alternative structures implement the same interface.

**Rationale:** Cell-based culling and streaming are predictable, simple to integrate and effective for large outdoor populations.

**Consequences:** Cell size tuning and worst-case dense views require profiling. The API cannot expose grid internals as the only option.

## ADR-017 — Renderer-neutral draw packets

**Status:** Accepted

**Decision:** Core and Forest produce visibility and draw packets, not graphics objects. A reference render layer maps packets to WebGPU.

**Rationale:** Custom engines need control over resources, shaders, frame graphs and synchronization.

**Consequences:** Reflection must precisely describe packed streams. Host integration work is explicit.

## ADR-018 — CPU reference for offline-sensitive baking

**Status:** Accepted

**Decision:** Determinism-sensitive atlas/impostor/bake operations have a CPU reference or deterministic mode. Accelerated paths must compare against it.

**Rationale:** GPU and driver variation can make source assets and CI unreproducible.

**Consequences:** Production-quality deterministic baking may be slower. Users can opt into faster preview paths.

## ADR-019 — Stable source identity through LOD

**Status:** Accepted

**Decision:** LOD outputs retain source entity identity, survivor maps and remap metadata.

**Rationale:** Wind, edits, selection, seasons, runtime transitions and diagnostics depend on continuity.

**Consequences:** Simplifiers cannot be black boxes that discard provenance.

## ADR-020 — Offline-first asset library

**Status:** Accepted

**Decision:** Local package catalogs and lockfiles are complete without network services. Remote registries are optional providers.

**Rationale:** Authoring and reproducible builds cannot depend on service availability.

**Consequences:** Local indexing, package validation and cache management are product responsibilities.

## ADR-021 — No proprietary binary compatibility in core scope

**Status:** Accepted

**Decision:** Do not parse or emit proprietary SpeedTree native formats in the core product.

**Rationale:** Functional parity can be achieved with independent formats and standard interchange while preserving clean-room boundaries.

**Consequences:** Existing native projects require neutral export from licensed software or a separately licensed adapter.

## ADR-022 — Evidence-derived parity and release status

**Status:** Accepted

**Decision:** Parity and release dashboards derive from linked tests, benchmarks, docs and integration artifacts.

**Rationale:** A large product can otherwise accumulate subjective “done” claims.

**Consequences:** Every feature task must produce traceable evidence; dashboard tooling is part of governance.

## ADR process

New ADRs use: context, decision, alternatives, consequences, compatibility, security, licensing and evidence. Accepted ADRs are immutable except for status and supersession links. A new ADR supersedes a decision rather than silently rewriting its history.
