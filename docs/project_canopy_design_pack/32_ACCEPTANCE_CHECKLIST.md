# General-availability acceptance checklist

## Evidence rule

Every checked item must link in the project traceability system to at least one reproducible artifact: automated test, golden asset, benchmark, host-integration run, security result, signed audit record, executable example, user-workflow record or published documentation.

A checkbox in this file is a template. It is not evidence by itself.

## Release identity and scope

- [ ] Release version, commit, build provenance and baseline date are recorded.
- [ ] The public comparison baseline is SpeedTree Modeler 10.2.0 and Runtime SDK 10 as documented on 2026-07-19, or a later explicitly approved baseline.
- [ ] Every row in `02_PARITY_BASELINE.md` has an owner and evidence link.
- [ ] Deferred functionality is listed as a concrete known limitation.
- [ ] Marketing language states workflow/functional parity and does not claim unimplemented proprietary binary compatibility.
- [ ] Supported authoring platforms, runtime platforms, engines, DCC versions and export formats are explicit.
- [ ] Product name, package IDs, domains and marks have completed clearance.

## Clean-room, licensing, and provenance

- [ ] Contributor declarations are complete.
- [ ] AI-agent provenance logs are complete for release changes.
- [ ] No implementation source is derived from decompiled, leaked or unauthorized proprietary material.
- [ ] No unlicensed proprietary native format reader or writer is present.
- [ ] Any optional licensed adapter is isolated and its distribution rights are documented.
- [ ] Third-party dependency review is complete for exact shipped versions.
- [ ] SBOM and third-party notices are generated from the release build.
- [ ] Every distributed asset, scan, texture, mesh and preview has provenance and distribution rights.
- [ ] Test assets are original, synthetic, public-domain or appropriately licensed.
- [ ] Trademark attribution and comparative references are reviewed.
- [ ] Patent/freedom-to-operate review has addressed intended markets and core algorithm families.
- [ ] No clean-room incident remains unresolved.

## Repository, builds, and installation

- [ ] Dependency boundaries pass the architecture build check.
- [ ] Clean Windows build succeeds with the declared toolchain.
- [ ] Clean macOS build succeeds with the declared toolchain.
- [ ] Clean Linux build succeeds with the declared toolchain.
- [ ] Headless-only build succeeds without Qt or viewport dependencies.
- [ ] Runtime-only build succeeds without authoring dependencies.
- [ ] Debug, release and sanitizer configurations build.
- [ ] Dependencies are pinned by version and hash.
- [ ] Generated files reproduce exactly in CI.
- [ ] Release archives and installers are signed where the platform supports it.
- [ ] Installation succeeds on clean supported systems.
- [ ] Update preserves projects, settings, plugins and libraries.
- [ ] Uninstall removes application files without deleting user projects or assets.
- [ ] Build attestations and package checksums are published.

## Document and project lifecycle

- [ ] New project creation works from empty, template and library asset flows.
- [ ] `.canopyproj` directory projects load and save canonically.
- [ ] `.canopy` packages load and save safely.
- [ ] Package-with-assets workflow includes selected dependencies and adjusted textures.
- [ ] Save uses atomic replacement and survives injected write failures.
- [ ] Autosave journals unsaved transactions without overwriting the source.
- [ ] Crash recovery identifies source revision and recoverable edits.
- [ ] Save As changes project identity and path references correctly.
- [ ] Read-only projects can be inspected without accidental mutation.
- [ ] Unknown optional fields are preserved when feasible.
- [ ] Required unknown schema features fail with an actionable message.
- [ ] Every supported older schema fixture migrates forward.
- [ ] Migration preserves the original and records warnings.
- [ ] Canonical re-save is byte stable for unchanged content.
- [ ] Semantic project diff reports graph, property, asset and edit-layer changes.
- [ ] Project units and axis settings round-trip.
- [ ] User metadata and tree information round-trip and export.
- [ ] Dependency lockfiles reproduce exact package content offline.
- [ ] Missing dependencies are reported and never silently replaced.
- [ ] Hostile archives cannot escape extraction roots or exceed configured limits.

## Transactions, undo, and concurrency

- [ ] Every mutation occurs in a named transaction.
- [ ] Commit, rollback and nested savepoint behavior pass.
- [ ] Stale-revision writes return conflict errors.
- [ ] Undo and redo cover graph, property, material, edit, asset and timeline operations.
- [ ] Mixed operation sequences undo to exact canonical state.
- [ ] Background evaluations cannot overwrite a newer UI revision.
- [ ] Cancellation leaves the last committed document and snapshot valid.
- [ ] Plugin failures cannot commit partial mutations.
- [ ] Read-only evaluation snapshots are safe for concurrent consumers.

## Procedural graph and properties

- [ ] Typed generator DAG creation, linking, unlinking, duplication and deletion work.
- [ ] Cycles are rejected with a source-located diagnostic.
- [ ] Parent/child type and cardinality rules are enforced.
- [ ] Generator lock, hide, solo/focus and tags work.
- [ ] Generator and generated-node selection synchronize with the viewport.
- [ ] Reflected property descriptors drive UI, scripting and serialization.
- [ ] Scalar, boolean, enum, color, asset, vector and transform values work.
- [ ] Length, angle, time and unitless values use explicit units.
- [ ] Curves support insertion, deletion, tangents, interpolation and extrapolation.
- [ ] Variance controls and distributions are deterministic.
- [ ] Relative and absolute parent-curve evaluation work.
- [ ] Generator randomization can target a selected subgraph.
- [ ] Named random streams do not change unrelated output when new streams are added.
- [ ] Per-generator random seeds and global seed controls work.
- [ ] Node overrides survive valid regeneration.
- [ ] Orphaned node edits are identified, preserved and optionally rebound.
- [ ] Multi-selection property editing follows documented mixed-value behavior.
- [ ] Expressions or rules cannot create dependency cycles without a diagnostic.

## Generation modes

- [ ] Interval mode passes placement and boundary fixtures.
- [ ] Phyllotaxy mode passes angular divergence and spacing fixtures.
- [ ] Bifurcation mode passes split-count and orientation fixtures.
- [ ] Proportional mode passes normalized parent-size fixtures.
- [ ] Proportional Steps mode passes discrete normalized placement fixtures.
- [ ] Absolute mode passes world-unit placement fixtures.
- [ ] Absolute Steps mode passes discrete world-unit placement fixtures.
- [ ] Flood mode fills eligible surfaces/regions deterministically.
- [ ] Parent mode derives or mirrors parent-generated locations as specified.
- [ ] Legacy/classic mode is clearly isolated and covered by compatibility fixtures.
- [ ] Shared first/last boundaries, frequency, spacing, orientation and jitter controls work.

## Generator catalog

- [ ] Tree generator defines scene extent, base radius and top-level anatomy.
- [ ] Branch generator supports trunks, boughs, branches, twigs and roots.
- [ ] Branch length and radius clamps work.
- [ ] Branch taper, flare, lobes, noise, straightness, curl, twist, zigzag and shape controls work.
- [ ] Branch splits, pruning and junctions produce valid geometry.
- [ ] Leaf Mesh generator supports cutout/imported meshes, orientation and per-leaf variation.
- [ ] Batched Leaf generator supports clustered leaves and shared post-processing.
- [ ] Legacy Leaf behavior required by the declared compatibility scope works.
- [ ] Frond generator supports ribbons, folds, segments, materials and wind data.
- [ ] Zone generator creates or constrains nodes within declared regions.
- [ ] Vine generator supports hanging, crawling, wrapping and twisting.
- [ ] Vines can follow guides and interact with ground/geometry.
- [ ] Manual vine placement and low-poly game presets work.
- [ ] Card generator produces oriented textured cards and clusters.
- [ ] Base generator composes base-level source geometry or hierarchy.
- [ ] Reference generator instantiates and edits reusable subhierarchies.
- [ ] Cap generator closes or decorates branch ends.
- [ ] Decal generator follows surfaces and supports seasonal state.
- [ ] Fin generator creates thin protruding detail with growth/LOD support.
- [ ] Knot generator places and blends knot/burl/scar details.
- [ ] Shell generator creates controlled offset surface layers.
- [ ] Mesh generator imports and preserves supported mesh semantics.
- [ ] Mesh Converter derives procedural controls from imported meshes.
- [ ] Mesh Detail creates wrapped or projected details with correct normals.
- [ ] Stitch generator joins imported and procedural geometry.
- [ ] Target generator locates children precisely on parent geometry.
- [ ] Spine generator creates hierarchy and deformation data without requiring visible tube geometry.
- [ ] Subdivision generator produces the declared surface refinement.
- [ ] Cage generator controls constrained mesh deformation.
- [ ] Proxy generator produces helper/optimization representations.
- [ ] All generator types pass the shared generator contract suite.
- [ ] Missing plugin generators preserve data through placeholders.

## Branch geometry and procedural techniques

- [ ] Adaptive spine sampling meets declared error bounds.
- [ ] Parallel-transport frames avoid unintended roll.
- [ ] Cross sections, ring stitching and ring-count transitions are valid.
- [ ] Tips and caps contain no unintended holes.
- [ ] Parent-child junctions meet crack, normal and silhouette tolerances.
- [ ] Bark UV tiling, scale, seam and patch controls work.
- [ ] Mesh anchors retain stable surface and orientation data.
- [ ] Clusters and grouped placement work.
- [ ] Shape envelopes, attractors and crown controls work.
- [ ] Geometry forces support attract, repel/avoid, align, obstruct, crawl, prune, stop and ground interaction.
- [ ] Force attenuation, containers and generator enable/weight controls work.
- [ ] Leaf collision removes or displaces intersecting foliage according to policy.
- [ ] Shade pruning supports interior and inverse/exterior removal.
- [ ] Shade pruning recomputes when required texture inputs finish loading.
- [ ] Mesh section manipulation and UV patching work.
- [ ] Per-vertex semantic attributes are finite, counted and reflected.
- [ ] Ground clipping and root/base behavior work in arbitrary project units.

## Manual and freehand editing

- [ ] Generator, node and freehand modes are distinct and discoverable.
- [ ] Bend brush and control-point editing create non-destructive layers.
- [ ] Displacement paint supports airbrush, erase, fill, smoothing and reset.
- [ ] Trim supports cut, shrink, grow, remove and selective reset.
- [ ] Trim results remain correct at lower LODs.
- [ ] Hand-drawn branches can be created, edited, resampled and assigned to target generators.
- [ ] Procedural branches can be converted to hand-drawn form with explicit consequences.
- [ ] Hand-drawn targets and locks behave deterministically.
- [ ] Click-place adds branches, targets and details with snapping/orientation.
- [ ] Vertex edit supports selection, soft selection and constrained deformation.
- [ ] Vertex edits maintain valid topology and attribute streams.
- [ ] Vertex Feature painting supports named scalar/vector channels.
- [ ] Vertex Color painting supports named color sets.
- [ ] Brush pressure/modifiers are normalized across supported input devices.
- [ ] Mesh helpers select robust point sequences on flat and curved surfaces.
- [ ] Mesh-helper curves create correct spine hierarchies and assignments.
- [ ] Art Director/global shape controls are transactional and undoable.
- [ ] Manual edits remain selectable and diagnosable after reevaluation.

## Materials, maps, and texture workflows

- [ ] PBR material model covers base color, opacity, normal, roughness, metallic/specular, AO, emissive, subsurface/transmission and displacement semantics.
- [ ] Material map color spaces are explicit.
- [ ] Front/back or two-sided leaf material policy works.
- [ ] Material sets support names, weights, rules, seasons and deterministic selection.
- [ ] Texture reload and dependency change detection work.
- [ ] Non-destructive image adjustments round-trip.
- [ ] UV Area editor supports named regions, transforms, pivots and anchors.
- [ ] Cutout editor supports polygon creation, editing, holes or declared limitation, simplification and alpha cleanup.
- [ ] Cutout triangulation remains valid for pathological masks.
- [ ] Texture atlas packing is deterministic.
- [ ] Atlas padding, rotation, resolution and separate/combined styles work.
- [ ] Multiple models can share a combined atlas with correct UV transforms.
- [ ] Channel-packing recipes control defaults, inversion, range and color space.
- [ ] Lightmap UVs are unique and nonoverlapping within tolerance.
- [ ] Texture and vertex AO options work.
- [ ] Thickness/shade-exposure bake outputs are validated.
- [ ] Per-material and model statistics report texture memory and atlas use.

## Photogrammetry and hero meshes

- [ ] Dense imported scan meshes load within configured resource limits.
- [ ] Scan cleanup supports scale, orientation, holes/noise policy and decimation hooks.
- [ ] Spine/axis extraction produces editable results for the reference corpus.
- [ ] Feature vertices preserve important ridges, seams and branch sites.
- [ ] Tiling PBR maps can be derived or projected from scan data.
- [ ] Texel-density tools report and correct mismatches.
- [ ] Bake-stitch extension joins scan and procedural geometry.
- [ ] Texture-blend extension joins scan and procedural geometry.
- [ ] Vertex-blend extension joins scan and procedural geometry.
- [ ] Branch/twig meshes can be prepared with pivots, anchors, orientation and cutouts.
- [ ] Prepared mesh assets can be clustered and reused procedurally.
- [ ] Hero meshes accept mesh-helper spine hierarchies.
- [ ] Procedural branches can be built from hero-mesh spines.
- [ ] Wind rigging and anchors export from hero meshes.

## Wind

- [ ] Game wind supports at least lightweight/mobile and full quality profiles.
- [ ] Hierarchical branch wind deforms trunks, branches and twigs coherently.
- [ ] Leaf ripple/tumble and frond motion work.
- [ ] Gust timing, strength, frequency and transitions work.
- [ ] Direction and velocity changes interpolate without discontinuity.
- [ ] Wind preview uses explicit time and can be paused/reset.
- [ ] Timeline wind is visibly indicated in the fan/wind UI.
- [ ] Only geometry with wind enabled contributes to wind behavior.
- [ ] VFX wind supports bone and point-cache outputs.
- [ ] VFX high-frequency/chaos controls work according to the declared model.
- [ ] Viewport and engine shader conformance vectors match.
- [ ] No-wind runtime state fully initializes all shader control data.
- [ ] Multiple forest wind domains work.
- [ ] Motion vectors account for wind deformation where supported.

## Growth and seasons

- [ ] Timeline supports exact frame rate, start/end and rational sample time.
- [ ] Generator growth start, duration and timing modes work.
- [ ] Parent-relative and in-place growth work.
- [ ] Growth wizard initializes editable schedules.
- [ ] Topology-stable growth previews and exports correctly.
- [ ] Topology-varying growth exports through USD/Alembic with exact frames.
- [ ] Empty growth frames do not crash exporters.
- [ ] Combined growth and wind cache frame counts are correct.
- [ ] Season selector supports named states and continuous transition value.
- [ ] Material seasonal curves blend correctly.
- [ ] Branch seasonal gravity works.
- [ ] Leaf size, drop and color controls work.
- [ ] Frond drop, curl, fold and gravity controls work.
- [ ] Decal and Mesh Detail seasonal timing works.
- [ ] Dropped foliage is deposited on configured surfaces.
- [ ] Dropped foliage defaults to neutral wind after landing.
- [ ] Seasonal topology changes use stable IDs or explicit swap metadata.

## Viewport and Modeler UX

- [ ] Dockable workspaces persist and restore.
- [ ] Default layouts can be reset without deleting user presets.
- [ ] Generation Editor supports all graph operations.
- [ ] Property, Curve and Variance editors are reflection driven.
- [ ] Asset, Material Set, Mesh, Displacement and Mask panels work.
- [ ] Rules, Season, Timeline, Fan and message panels work.
- [ ] PBR viewport supports HDR environment/direct light, shadows and AO.
- [ ] Leaf/frond two-sided and transmission preview works.
- [ ] Standard, wire/scribed, AO, albedo, saturation, subsurface, lightmap, normal, UV, wind and LOD diagnostics are available or mapped to equivalent modes.
- [ ] User-defined render/debug mode is supported through a documented extension.
- [ ] Viewport resolution controls change preview cost predictably.
- [ ] Camera orbit, pan, zoom, focus, orthographic/perspective and reset work.
- [ ] Named scene cameras work.
- [ ] Still, tiled high-resolution and sequence rendering work.
- [ ] Selection outline/wireframe remains visible across render modes.
- [ ] Visibility toggles exist by geometry/helper class.
- [ ] Statistics include nodes, triangles, vertices, materials, draw calls and memory estimates.
- [ ] Background computing never blocks basic UI interaction beyond the response target.
- [ ] Progress is cancelable and cancellation is acknowledged promptly.
- [ ] Diagnostics navigate to relevant graph object, asset or property.
- [ ] High-DPI layouts work at 100–200 percent.
- [ ] Keyboard access, focus order, accessible roles/names and noncolor state cues pass.
- [ ] Localization does not alter saved identifiers or numeric parsing.

## LOD, billboards, and optimization

- [ ] Continuous LOD reduces branch segments/rings without invalid junctions.
- [ ] Foliage, fronds and details remove according to class-specific curves.
- [ ] Shrink/grow compensation avoids abrupt disappearance.
- [ ] Stable source identity and LOD remaps are present.
- [ ] Discrete LOD baking produces validated meshes and per-LOD bounds.
- [ ] LOD thresholds can use distance or projected screen size.
- [ ] Hysteresis prevents threshold chatter.
- [ ] Crossfade/dither metadata supports smooth transitions.
- [ ] Independent shadow LOD works.
- [ ] Multi-view impostor baking produces color, opacity, normal and depth data according to preset.
- [ ] Top-down impostor view is supported when requested.
- [ ] Impostor geometry and atlas metadata align.
- [ ] Runtime impostor frame selection matches the camera sweep tests.
- [ ] Wind phase remains coherent through geometry/impostor transition.
- [ ] Collision primitives, convex approximations and custom proxies export.
- [ ] Shade pruning and collision can be used as optimization stages.
- [ ] Triangle, draw-call, texture and overdraw budgets are reported.
- [ ] Optimizer never violates declared hard constraints silently.

## Export and pipeline

- [ ] Export preset system separates games, VFX and custom profiles.
- [ ] User presets save, load, validate and migrate.
- [ ] ExportScene normalizes axes, units, winding, UV origin, tangents and time basis once.
- [ ] Export is deterministic for identical snapshot and preset.
- [ ] Export manifest records source/preset/schema hashes and output statistics.
- [ ] Output transaction commits atomically.
- [ ] OBJ export passes an independent parser.
- [ ] glTF 2.0 export passes Khronos validation.
- [ ] OpenUSD export passes official OpenUSD validation.
- [ ] Alembic export reads through the reference library.
- [ ] Optional FBX adapter works only in approved licensed builds and is absent cleanly elsewhere.
- [ ] Static mesh export works.
- [ ] Bone/skeleton export works for supported formats.
- [ ] Point-cache export works.
- [ ] Topology-varying cache export works.
- [ ] LODs, billboards, collisions, anchors and metadata export.
- [ ] Texture atlases and channel-packed textures export.
- [ ] Custom vertex/mesh packing can write documented semantics to target channels.
- [ ] Packing scripts are sandboxed, bounded and reflected.
- [ ] Custom texture packing recipes are validated.
- [ ] Command-line validate, export, render, package, migrate, inspect and compile-runtime commands work.
- [ ] CLI supports human, JSON and JSON Lines diagnostics.
- [ ] CLI exit codes are stable and documented.
- [ ] CLI cancellation and atomic-output behavior pass.
- [ ] Custom exporter SDK sample works in-process.
- [ ] Isolated custom exporter sample survives crash and quota tests.
- [ ] Package output contains only declared files beneath its root.

## Runtime SDK and forest

- [ ] `.canopyrt` format specification and conformance vectors are published.
- [ ] Runtime compiler produces deterministic section ordering and checksums.
- [ ] Runtime loader validates header, section directory, offsets, counts and compression sizes.
- [ ] Unknown optional sections are skipped; unknown required sections fail.
- [ ] Zero-copy/memory-mapped and copied load modes work.
- [ ] Runtime Core and Forest have no graphics-API dependency.
- [ ] C ABI headers compile as C and C++.
- [ ] ABI layout and old-client compatibility tests pass.
- [ ] Caller allocator, logging, file and job-system hooks work.
- [ ] Base asset geometry is shared across instances.
- [ ] One million-instance population insertion/query/removal stress test passes.
- [ ] Sparse cells stream and cull correctly.
- [ ] Camera handedness, depth range, reversed-Z, matrix layout and origin are explicit.
- [ ] CPU culling matches brute-force reference.
- [ ] Projected-size LOD, bias, hysteresis and crossfade work.
- [ ] Renderer-neutral draw packets expose complete stream semantics.
- [ ] Shader-layout mismatch is detected before unsafe rendering.
- [ ] Runtime wind domains and fixed-step controller work.
- [ ] Growth age and seasonal instance state work when present.
- [ ] Billboards/impostors select and fade correctly.
- [ ] Collision, anchors, metadata and optional skeleton data are accessible.
- [ ] Shadow views and independent shadow LOD work.
- [ ] Large-world origin rebasing preserves instance identity and visible position.
- [ ] Streaming priority, cancellation, residency and eviction work.
- [ ] CPU/GPU memory budgets expose current and peak usage.
- [ ] GPU-driven culling visibility agrees with CPU within policy.
- [ ] Indirect rendering path meets the approved desktop benchmark profile.
- [ ] Optional terrain module supports tiles, masks, queries and deterministic placement.
- [ ] Reference viewer demonstrates one asset and mixed forest workflows.
- [ ] Runtime loader and RPC fuzz campaigns have no unresolved stop-ship finding.

## Engine integrations

- [ ] Shared import manifest and shader semantic vectors are versioned.
- [ ] Unity importer creates correct assets, materials, LODGroup, collision and metadata.
- [ ] Unity Built-in, URP and HDRP declared versions pass import/reimport/build tests.
- [ ] Unity wind, season, growth and impostor behavior pass reference scenes.
- [ ] Unity indirect population path meets its benchmark profile.
- [ ] Unreal asset factory, reimport, materials, LOD, impostors, collision and metadata work.
- [ ] Unreal packaged build works without editor-only dependencies.
- [ ] Unreal wind and population component pass the reference map.
- [ ] Godot declared versions pass import, MultiMesh, wind and LOD scene tests.
- [ ] O3DE declared versions pass Asset Builder, material and vegetation tests.
- [ ] Custom-engine starter kit builds against installed SDK packages only.
- [ ] Every integration states exact supported host and core/schema versions.
- [ ] Import diagnostics identify missing maps, semantics and incompatible profiles.
- [ ] Engine uninstall does not delete imported user assets.

## DCC integrations

- [ ] Blender GUI and background import tests pass for declared versions.
- [ ] Blender materials, attributes, LOD collections and animation map correctly.
- [ ] Maya GUI/standalone import, materials and animation tests pass.
- [ ] Houdini SOP/Solaris, point instancer, MaterialX and PDG tests pass.
- [ ] 3ds Max declared static/cache workflow and batch smoke test pass.
- [ ] Cinema 4D declared material, MoGraph, LOD and cache workflow pass.
- [ ] USD is used for scene/variant/instancing workflows where appropriate.
- [ ] Alembic is used for dense or topology-varying cache workflows where appropriate.
- [ ] Unit, axis, tangent, UV and frame-time fixtures pass in every host.
- [ ] DCC packages do not claim to round-trip the procedural graph through a lossy interchange format.

## Rules, plugins, and automation

- [ ] Rules support float, integer, boolean, enum, color, curve, text, action and asset controls as declared.
- [ ] Rules read/write only documented property APIs inside transactions.
- [ ] Rule errors roll back changes and report source location.
- [ ] Rule filesystem, network, process and native loading are unavailable by default.
- [ ] Rule instruction and memory limits are enforced.
- [ ] Python open, transaction, evaluate, export and inspect examples pass.
- [ ] Python ownership, exception and GIL/thread behavior pass.
- [ ] Native plugin ABI negotiates versions and sized structures.
- [ ] Plugin allocators and ownership pass cross-module tests.
- [ ] Unknown project plugins never auto-execute.
- [ ] Plugin trust, signer and permission decisions are recorded.
- [ ] Missing plugins preserve serialized data in placeholders.
- [ ] Isolated extension crash, hang, cancellation and quota tests pass.
- [ ] Plugin discovery order is deterministic.
- [ ] Hot reload follows the documented safety constraints.

## Asset library and variants

- [ ] `.canopyasset` manifest validates IDs, version, content hashes, entry points and license fields.
- [ ] Local index can be deleted and rebuilt from packages.
- [ ] Search supports name, scientific taxonomy, tags and numeric statistics.
- [ ] Install, update, pin, vendor and remove work offline.
- [ ] Corrupt packages are quarantined.
- [ ] Duplicate content is detected by hash.
- [ ] Remote registry is optional and product operation continues without it.
- [ ] Remote downloads verify hashes and configured signatures.
- [ ] Credentials use OS secure storage.
- [ ] Variation schema exposes bounded editable controls.
- [ ] Variant recipes support deterministic distributions, correlations and rejection constraints.
- [ ] Locking and rerolling dimensions do not perturb locked samples.
- [ ] Variation browser thumbnails cancel and update asynchronously.
- [ ] Batch export of selected variants produces unique stable manifests.
- [ ] License/provenance metadata survives package, project and export.
- [ ] Organization policy decisions are auditable and show recorded source terms.

## Determinism and data integrity

- [ ] Worker counts 1, 2 and maximum produce matching declared deterministic output.
- [ ] Cold and warm cache output match.
- [ ] Different working directories and file enumeration order do not affect content.
- [ ] Windows, macOS and Linux determinism matrix passes.
- [ ] x86-64 and ARM64 meet exact or field-specific tolerance policy.
- [ ] Random vectors and canonical hashes are versioned.
- [ ] Cache corruption triggers recomputation.
- [ ] Fault-injected allocation failures return structured errors.
- [ ] Disk-full, permission and truncated-write tests preserve committed data.
- [ ] Every geometry output passes index, finite-value, bounds and stream-count invariants.

## Performance and scalability

- [ ] Standard Modeler asset meets input and preview responsiveness targets.
- [ ] Viewport meets the declared frame-rate target under standard preview budget.
- [ ] Full and incremental evaluation benchmarks meet approved budgets.
- [ ] Cache hit/miss and invalidation traces explain performance.
- [ ] Hero and pathological assets remain bounded and cancelable.
- [ ] Runtime load and population benchmarks meet approved budgets.
- [ ] CPU and GPU culling reports include hardware, driver, resolution and assets.
- [ ] Per-frame runtime path performs no unplanned per-instance heap allocations.
- [ ] Streaming avoids unplanned GPU synchronization according to captured traces.
- [ ] Peak memory remains within profile budgets.
- [ ] Performance regressions above budget have reviewed explanations.

## Security and privacy

- [ ] Threat model covers archives, parsers, images, scripts, plugins, RPC, registries and shader compilation.
- [ ] Checked arithmetic protects offsets, counts and sizes.
- [ ] Decompression and nesting limits are enforced.
- [ ] Path normalization prevents traversal and unsafe absolute paths.
- [ ] Parser and runtime fuzz corpora run under sanitizers.
- [ ] Extension permissions and isolation controls pass adversarial tests.
- [ ] Registry transport and signature policy pass.
- [ ] No hidden network access occurs during document evaluation.
- [ ] Telemetry is off unless explicitly enabled by product policy/user choice.
- [ ] Project content and paths are excluded from telemetry and default crash payloads.
- [ ] Credentials are not stored in project files or logs.
- [ ] Security reporting and patch process is published.
- [ ] No unresolved stop-ship security condition remains.

## Documentation and supportability

- [ ] User manual covers all Modeler workflows.
- [ ] C++ authoring API reference is published.
- [ ] C runtime and plugin ABI references are published.
- [ ] Python and Lua references are published.
- [ ] CLI commands, flags, schemas and exit codes are published.
- [ ] Authoring, package, asset and runtime formats are documented.
- [ ] Export-format and packing semantics are documented.
- [ ] Engine and DCC integration guides state exact supported versions.
- [ ] Custom-engine integration tutorial builds from installed packages.
- [ ] Migration and known-limitation notes are published.
- [ ] Performance methodology is published.
- [ ] Security, plugin trust and privacy behavior are published.
- [ ] Third-party notices and source-offer obligations, if any, are fulfilled.
- [ ] Executable documentation examples pass CI.

## Final sign-off

- [ ] Product owner signs functional scope.
- [ ] Architecture owner signs public contracts and dependency boundaries.
- [ ] Geometry owner signs generator/manual/LOD evidence.
- [ ] Rendering owner signs viewport, wind, impostor and runtime-render evidence.
- [ ] Pipeline owner signs export and DCC evidence.
- [ ] Runtime owner signs loader, culling, forest and ABI evidence.
- [ ] Security owner signs threat-model and test evidence.
- [ ] Clean-room/licensing owner signs provenance and distribution audit.
- [ ] Accessibility owner signs Modeler accessibility evidence.
- [ ] Release owner signs build, package, test and known-limitation evidence.
