# Epics and implementation tasks

## Use

This is the master implementation map. Task identifiers are stable planning keys, not calendar commitments. Each task must be expanded into the work-unit template in `24_AGENT_IMPLEMENTATION_PLAYBOOK.md` before assignment.

Dependencies are logical predecessors. A dependency ending in `00`, such as `CAN-0500`, means the corresponding epic has reached its integration gate. Replace such aggregate references with concrete task IDs when scheduling.

## Epic sequence

- **E00 — Clean-room governance and product control**: 6 tasks
- **E01 — Repository, build, and platform foundation**: 7 tasks
- **E02 — Document, schema, assets, and graph**: 10 tasks
- **E03 — Evaluation, randomness, and caching**: 9 tasks
- **E04 — Branch skeleton and meshing foundation**: 10 tasks
- **E05 — Procedural generator catalog**: 15 tasks
- **E06 — Manual and freehand editing**: 11 tasks
- **E07 — Materials, textures, photogrammetry, and baking**: 12 tasks
- **E08 — Wind, growth, seasons, and timeline**: 11 tasks
- **E09 — LOD, billboards, collision, and optimization**: 9 tasks
- **E10 — Export, CLI, and pipeline SDK**: 12 tasks
- **E11 — Modeler UI and viewport**: 13 tasks
- **E12 — Runtime SDK and forest**: 14 tasks
- **E13 — Engine and DCC integrations**: 13 tasks
- **E14 — Rules, plugins, Python, library, and variants**: 11 tasks
- **E15 — Hardening, documentation, and general availability**: 10 tasks

## Task catalog

### E00 — Clean-room governance and product control

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0001` | Create contributor provenance declaration and restricted-material policy | `—` | Every contributor and agent run records allowed inputs; CI blocks missing declarations |
| `CAN-0002` | Create feature-parity traceability database | `CAN-0001` | Every row in `02_PARITY_BASELINE.md` has an owner, evidence type and release gate |
| `CAN-0003` | Adopt trademark-safe naming and placeholder package IDs | `CAN-0001` | No shipping identifier uses third-party marks; rename procedure is documented |
| `CAN-0004` | Create dependency intake and license-review workflow | `CAN-0001` | New dependencies require machine-readable license, provenance and approval metadata |
| `CAN-0005` | Create ADR and public-contract review process | `CAN-0002` | Schema, ABI and architecture changes cannot merge without linked ADR status |
| `CAN-0006` | Create parity evidence dashboard | `CAN-0002` | Dashboard derives status from tests and artifacts rather than manual percentage claims |

### E01 — Repository, build, and platform foundation

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0101` | Create monorepo module skeleton and dependency rules | `CAN-0005` | All modules configure; forbidden dependency edges fail a build check |
| `CAN-0102` | Add CMake presets for Windows, macOS and Linux | `CAN-0101` | Debug, release, sanitizer and headless presets configure on supported hosts |
| `CAN-0103` | Pin third-party dependencies with hashes and SBOM generation | `CAN-0004,CAN-0102` | Clean build resolves exact versions and emits SPDX or CycloneDX SBOM |
| `CAN-0104` | Implement diagnostics, status codes and structured logging | `CAN-0101` | C++, C ABI and CLI expose stable machine codes and nested diagnostics |
| `CAN-0105` | Implement allocator, task, cancellation and progress abstractions | `CAN-0101` | Fault injection, cancellation and caller-provided job-system tests pass |
| `CAN-0106` | Add sanitizers, static analysis and formatting checks | `CAN-0102` | CI runs compiler warnings, ASan/UBSan or platform equivalents and lint checks |
| `CAN-0107` | Create reproducible packaging and signing pipeline | `CAN-0103` | Two clean release builds produce matching declared artifacts and attestations |

### E02 — Document, schema, assets, and graph

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0201` | Define canonical authoring JSON schemas | `CAN-0005,CAN-0104` | Schemas validate minimal and complete fixtures; IDs and units are explicit |
| `CAN-0202` | Implement UUID, property path, units and reflected value types | `CAN-0101` | Round-trip and invalid-value tests cover every public value kind |
| `CAN-0203` | Implement immutable document state and transactions | `CAN-0201,CAN-0202` | Commit, rollback, savepoint and stale-revision conflict tests pass |
| `CAN-0204` | Implement typed generator DAG and validation | `CAN-0203` | Cycle, type, cardinality and missing-plugin errors are deterministic and source located |
| `CAN-0205` | Implement canonical serialization and source-control diff | `CAN-0203` | Repeated save is byte stable; semantic diff reports object/property changes |
| `CAN-0206` | Implement `.canopyproj` directory load/save | `CAN-0205` | Atomic save, dependency references and recovery fixtures pass |
| `CAN-0207` | Implement `.canopy` package load/save | `CAN-0206` | Safe ZIP packaging, embedded assets, checksums and hostile-path tests pass |
| `CAN-0208` | Implement schema migration framework | `CAN-0205` | Version fixtures migrate without silent loss and preserve originals |
| `CAN-0209` | Implement content-addressed asset resolver and lockfile | `CAN-0206` | Project-relative, package and registry-independent resolution is reproducible offline |
| `CAN-0210` | Implement command-based undo/redo and autosave journal | `CAN-0203` | Mixed graph/property/edit operations undo exactly and recover after simulated crash |

### E03 — Evaluation, randomness, and caching

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0301` | Implement stable named random streams | `CAN-0202` | Published vectors match on all platforms; adding an unrelated stream changes no existing output |
| `CAN-0302` | Implement property evaluation with scalar, variance and curves | `CAN-0202,CAN-0301` | Relative/absolute parent lookup, units and edge curves pass conformance tests |
| `CAN-0303` | Implement dependency extraction and invalidation | `CAN-0204,CAN-0302` | Trace tests prove only affected stages and descendants are invalidated |
| `CAN-0304` | Implement immutable evaluation snapshots | `CAN-0303` | Concurrent readers observe one revision and no mutable backing state |
| `CAN-0305` | Implement content-addressed stage cache | `CAN-0303` | Cold/warm output matches; corruption causes recomputation rather than bad output |
| `CAN-0306` | Implement deterministic parallel scheduler | `CAN-0105,CAN-0304` | Worker counts 1, 2 and maximum produce matching stable output |
| `CAN-0307` | Implement evaluation profiles and resolution controls | `CAN-0304` | Draft, preview and production profiles request documented products and budgets |
| `CAN-0308` | Implement node override and edit-layer merge | `CAN-0304,CAN-0210` | Overrides survive valid reevaluation and report orphaned targets explicitly |
| `CAN-0309` | Add evaluation trace and cache inspector | `CAN-0305` | CLI and editor can show stage timings, hashes, dependencies and cache outcomes |

### E04 — Branch skeleton and meshing foundation

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0401` | Implement robust curve and spline library | `CAN-0202` | Arc length, closest point, subdivision and degenerate cases pass property tests |
| `CAN-0402` | Implement parallel-transport frames | `CAN-0401` | Frames avoid unintended roll and remain stable through near-collinear segments |
| `CAN-0403` | Implement branch spine sampling and adaptive tessellation | `CAN-0401,CAN-0307` | Error-bounded samples are stable and profile responsive |
| `CAN-0404` | Implement radius, taper, flare, lobes and cross sections | `CAN-0403` | Reference profiles produce valid rings with finite normals and bounds |
| `CAN-0405` | Implement ring stitching, tips and caps | `CAN-0404` | Changing ring counts creates no out-of-range indices or unintended holes |
| `CAN-0406` | Implement branch-parent junction blending | `CAN-0405` | Junction fixtures meet gap, normal and silhouette tolerances |
| `CAN-0407` | Implement branch UV tiling, patching and seam controls | `CAN-0405` | Texel density, seam and patch metadata match declared conventions |
| `CAN-0408` | Implement branch normals, tangents, AO hooks and semantic attributes | `CAN-0405` | All streams are finite, normalized and reflected to exporters |
| `CAN-0409` | Implement roots, ground clipping and base flares | `CAN-0404` | Root fixtures interact with ground and export correct bounds |
| `CAN-0410` | Benchmark and optimize branch meshing | `CAN-0408` | Standard and hero benchmarks meet agreed budgets with no output change |

### E05 — Procedural generator catalog

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0501` | Implement generation modes: interval, absolute and proportional families | `CAN-0302,CAN-0401` | Placement fixtures cover boundaries, steps and parent-relative lookup |
| `CAN-0502` | Implement phyllotaxy, bifurcation, flood, parent and legacy mode | `CAN-0501` | Deterministic distributions and compatibility fixtures pass |
| `CAN-0503` | Implement Tree and Branch generators | `CAN-0409,CAN-0502` | Trunks, boughs, twigs and roots satisfy shared generator contract |
| `CAN-0504` | Implement Leaf Mesh and legacy Leaf generators | `CAN-0502` | Orientation, mesh anchors, materials, collision fields and LOD semantics pass |
| `CAN-0505` | Implement Batched Leaf generator | `CAN-0504` | Clustered placement reduces draw/vertex overhead while preserving variation |
| `CAN-0506` | Implement Frond and Card generators | `CAN-0502,CAN-0407` | Ribbon/card topology, UVs, folds, materials and wind attributes pass |
| `CAN-0507` | Implement Zone, Base and Reference generators | `CAN-0502` | Volume placement, base composition and reusable hierarchy references work |
| `CAN-0508` | Implement Cap, Decal, Fin, Knot and Shell generators | `CAN-0503` | Detail geometry attaches, blends, LODs and seasons according to descriptors |
| `CAN-0509` | Implement Mesh, Subdivision, Cage and Proxy generators | `CAN-0209` | Imported geometry, modifiers and proxy outputs are deterministic |
| `CAN-0510` | Implement Target and Spine generators | `CAN-0401,CAN-0509` | Surface placement, hierarchy and skinning metadata pass hero-mesh fixtures |
| `CAN-0511` | Implement Mesh Converter, Mesh Detail and Stitch generators | `CAN-0503,CAN-0509` | Synthetic scan converts, receives details and joins native geometry by three strategies |
| `CAN-0512` | Implement procedural and guided Vine generator | `CAN-0502,CAN-0602` | Hang, crawl, wrap, guide-follow and low-poly presets are deterministic |
| `CAN-0513` | Implement geometry forces and containers | `CAN-0503` | Attract, avoid, align, crawl, obstruct, prune, stop and ground behaviors pass fixtures |
| `CAN-0514` | Implement collision, clusters, splits and shape control | `CAN-0503,CAN-0504` | Post stages preserve stable IDs and meet silhouette/collision expectations |
| `CAN-0515` | Implement leaf shade pruning | `CAN-0504,CAN-0708` | Interior/exterior modes react to completed texture inputs and preserve silhouette target |

### E06 — Manual and freehand editing

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0601` | Implement screen ray, picking and stable selection | `CAN-1103` | Graph and viewport selection agree under LOD and hidden-state changes |
| `CAN-0602` | Implement collision query and surface projection service | `CAN-0405,CAN-0509` | Nearest, sweep and signed-distance queries handle degenerate inputs |
| `CAN-0603` | Implement bend edit layer | `CAN-0308,CAN-0401` | Brush and control-point bends are non-destructive and undoable |
| `CAN-0604` | Implement displacement painting | `CAN-0308,CAN-0601` | Airbrush, erase, fill and smoothing work on branches and meshes |
| `CAN-0605` | Implement trim modes | `CAN-0308,CAN-0601` | Cut, shrink, grow, remove and selective reset remain valid through lower LODs |
| `CAN-0606` | Implement hand-drawn branches and spline editing | `CAN-0401,CAN-0601` | Draw, resample, edit points, assign target generator and convert procedural branches |
| `CAN-0607` | Implement click-place for branches, targets and details | `CAN-0602` | Placement snaps, orients and records stable surface anchors |
| `CAN-0608` | Implement topology-preserving vertex edit | `CAN-0308,CAN-0601` | Soft selection and constraints preserve stream validity and undo |
| `CAN-0609` | Implement vertex feature and color painting | `CAN-0604` | Named channels export with deterministic brush accumulation |
| `CAN-0610` | Implement mesh helpers and point-sequence tools | `CAN-0510,CAN-0602` | Curves extracted on flat and curved hero surfaces create correct spine hierarchies |
| `CAN-0611` | Implement Art Director gizmos and bulk edits | `CAN-0601,CAN-0308` | Spatial global edits are transactional, previewable and repeatable |

### E07 — Materials, textures, photogrammetry, and baking

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0701` | Implement material graph/value model and material sets | `CAN-0209,CAN-0302` | PBR semantics, weighted sets, seasons and serialization pass |
| `CAN-0702` | Implement image loading, color management and texture cache | `CAN-0104` | Supported images decode with explicit color space and bounded resources |
| `CAN-0703` | Implement non-destructive map adjustments | `CAN-0702` | Levels, hue, normal strength, alpha and channel operations are deterministic |
| `CAN-0704` | Implement UV area editor | `CAN-0701` | Named regions, pivots, anchors and transforms serialize and preview |
| `CAN-0705` | Implement cutout editor and triangulation | `CAN-0702,CAN-0405` | Polygon editing, alpha cleanup and robust triangulation pass pathological masks |
| `CAN-0706` | Implement deterministic texture atlas packer | `CAN-0703,CAN-0704` | Padding, rotation, separate and combined styles reproduce exact manifests |
| `CAN-0707` | Implement lightmap UV generation | `CAN-0408` | Charts are nonoverlapping within tolerance and validate in target exporters |
| `CAN-0708` | Implement AO, thickness and shade-exposure baking | `CAN-0408,CAN-1104` | CPU reference and accelerated backend meet comparison tolerance |
| `CAN-0709` | Implement synthetic-scan cleanup and spine extraction | `CAN-0509,CAN-0602` | Noisy trunk fixture yields editable axis and feature vertices |
| `CAN-0710` | Implement scan texture projection and tiling-map creation | `CAN-0702,CAN-0709` | Projection handles seams and emits validated PBR map set |
| `CAN-0711` | Implement bake stitch, texture blend and vertex blend | `CAN-0511,CAN-0710` | All three scan-to-procedural extension workflows pass visual and geometry tests |
| `CAN-0712` | Implement branch/twig mesh preparation workflow | `CAN-0704,CAN-0607` | Pivot, anchors, orientation, cutout and clustering metadata are reusable |

### E08 — Wind, growth, seasons, and timeline

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0801` | Define wind hierarchy and vertex semantic contract | `CAN-0408,CAN-0202` | Reference vectors and shader reflection are versioned |
| `CAN-0802` | Implement game wind authoring controller | `CAN-0801` | Quality tiers, gusts, branch hierarchy, leaf ripple and frond motion preview |
| `CAN-0803` | Implement reference wind shader functions | `CAN-0802,CAN-1104` | Fixed-time conformance captures pass on WebGPU backends |
| `CAN-0804` | Implement VFX wind deformation and cache sampling | `CAN-0802` | Bone and point-cache outputs preserve timing and topology contracts |
| `CAN-0805` | Implement timeline, fan and wind domains | `CAN-0802` | Absolute-time playback, transitions and no-wind initialization are deterministic |
| `CAN-0806` | Implement growth property model and scheduling | `CAN-0302` | Parent/in-place timing, start/duration and generator channels evaluate correctly |
| `CAN-0807` | Implement growth wizard | `CAN-0806` | Rule-based initialization creates editable, plausible non-overlapping schedules |
| `CAN-0808` | Implement topology-stable growth preview | `CAN-0806,CAN-1104` | Fixed-age captures and bounds are correct |
| `CAN-0809` | Implement topology-changing VFX growth export | `CAN-0806,CAN-1005` | USD/Alembic sequences handle empty frames and exact frame counts |
| `CAN-0810` | Implement seasonal material and generator channels | `CAN-0701,CAN-0302` | Branch, leaf, batched leaf, frond, decal and mesh detail channels blend |
| `CAN-0811` | Implement leaf/frond dropping and deposition | `CAN-0810,CAN-0602` | Dropped elements land on configured surface and default to neutral wind |

### E09 — LOD, billboards, collision, and optimization

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-0901` | Define stable LOD identity and error metrics | `CAN-0408,CAN-0504` | Every source element has deterministic survival/remap metadata |
| `CAN-0902` | Implement continuous branch LOD | `CAN-0901` | Segment/ring reduction and smooth radius shrink preserve junction validity |
| `CAN-0903` | Implement foliage/frond/detail dropout and compensation | `CAN-0901` | Silhouette and density metrics remain within target through LOD |
| `CAN-0904` | Implement discrete LOD baker | `CAN-0902,CAN-0903` | Preset thresholds produce validated immutable meshes and bounds |
| `CAN-0905` | Implement multi-view billboard/impostor baker | `CAN-0706,CAN-1105` | Color, normal, depth, AO and top views align with proxy geometry |
| `CAN-0906` | Implement geometry-to-impostor transition metadata | `CAN-0905` | Dither/crossfade tests show no threshold chatter or phase jump |
| `CAN-0907` | Implement collision-object generation | `CAN-0405,CAN-0509` | Primitive, convex and custom proxy outputs are accessible in export/runtime |
| `CAN-0908` | Implement overdraw, draw-call and triangle budget optimizer | `CAN-0706,CAN-0904` | Optimizer respects hard constraints and reports tradeoffs |
| `CAN-0909` | Implement LOD editor and diagnostics | `CAN-0904,CAN-1102` | Artists can inspect per-class curves, counts, bounds and transitions |

### E10 — Export, CLI, and pipeline SDK

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-1001` | Implement normalized immutable ExportScene | `CAN-0304,CAN-0701,CAN-0904` | All exporters consume one validated coordinate/material/animation model |
| `CAN-1002` | Implement export presets, manifest and atomic output transaction | `CAN-1001` | Repeated export emits stable manifest and no false partial success |
| `CAN-1003` | Implement OBJ diagnostic exporter | `CAN-1002` | Independent parser validates meshes, groups, materials and axes |
| `CAN-1004` | Implement glTF 2.0 exporter | `CAN-1002` | Khronos validation passes for static, LOD metadata and supported animation |
| `CAN-1005` | Implement OpenUSD exporter | `CAN-1002` | OpenUSD validation passes for meshes, variants, point instances, materials and animation |
| `CAN-1006` | Implement Alembic exporter | `CAN-1002` | Reference library reads static and cache sequences with exact sampling |
| `CAN-1007` | Implement optional FBX adapter | `CAN-0004,CAN-1002` | Build is isolated behind licensed SDK and absent adapter degrades clearly |
| `CAN-1008` | Implement diagnostic JSON/XML exporter | `CAN-1002` | All semantic streams and hierarchy are inspectable and schema validated |
| `CAN-1009` | Implement custom vertex and texture packing | `CAN-1401,CAN-1002` | Sandbox recipes pass reflection, conformance vectors and runtime-layout checks |
| `CAN-1010` | Implement full Canopy CLI | `CAN-1002,CAN-0208` | All specified commands, exit codes, JSON output and cancellation behavior pass |
| `CAN-1011` | Implement image, material and sequence rendering export | `CAN-1105,CAN-0804` | Still and sequence outputs use exact camera/time and validated metadata |
| `CAN-1012` | Implement custom exporter SDK | `CAN-1403,CAN-1001` | Sample in-process and isolated exporters pass ownership and failure tests |

### E11 — Modeler UI and viewport

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-1101` | Create Qt application shell, workspaces and docking | `CAN-0102` | Layouts persist, reset, scale and remain keyboard accessible |
| `CAN-1102` | Implement command/view-model layer | `CAN-0203,CAN-0210` | UI contains no direct document mutation and async state is revision safe |
| `CAN-1103` | Implement WebGPU viewport foundation | `CAN-0102,CAN-0304` | Meshes, camera, picking IDs and resize render on all authoring platforms |
| `CAN-1104` | Implement PBR, debug, overlay and selection passes | `CAN-1103,CAN-0701` | Standard and diagnostic modes meet golden captures |
| `CAN-1105` | Implement offscreen render, cameras and turntables | `CAN-1104` | Named cameras, high-resolution tiled stills and sequences export |
| `CAN-1106` | Implement Generation Editor | `CAN-1102,CAN-0204` | Create/link/duplicate/lock/hide/query operations are undoable |
| `CAN-1107` | Implement reflected Property, Curve and Variance editors | `CAN-1102,CAN-0302` | All descriptor types edit with validation and multi-selection rules |
| `CAN-1108` | Implement Assets, Materials, Meshes and Sets panels | `CAN-1102,CAN-0701` | Import, reload, reference tracking and thumbnails operate asynchronously |
| `CAN-1109` | Implement Cutout and UV Area editors | `CAN-0704,CAN-0705` | 2D editing is precise, undoable and synchronized with preview |
| `CAN-1110` | Implement Rules, Season, Timeline and Fan UI | `CAN-1401,CAN-0805,CAN-0810` | Controls invoke transactions and display source-located errors |
| `CAN-1111` | Implement messages, stats, preferences and recovery UI | `CAN-0104,CAN-0206` | Diagnostics navigate to objects and recovery never overwrites source |
| `CAN-1112` | Implement freehand tool interaction framework | `CAN-0601,CAN-1103` | Brushes, gizmos, pressure, modifiers and overlays route to edit commands |
| `CAN-1113` | Implement accessibility, localization and high-DPI verification | `CAN-1101` | Release UI checklist passes keyboard, roles, focus and 200% scale |

### E12 — Runtime SDK and forest

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-1201` | Specify `.canopyrt` binary schema and conformance vectors | `CAN-0005,CAN-1001` | Header, section rules and malformed fixtures are normative |
| `CAN-1202` | Implement runtime compiler | `CAN-1201,CAN-1009` | Authoring export produces deterministic, validated, memory-mappable package |
| `CAN-1203` | Implement hardened runtime loader and asset views | `CAN-1201,CAN-0104` | Zero-copy and copied modes pass fuzz, size and checksum tests |
| `CAN-1204` | Implement C runtime ABI | `CAN-1203,CAN-0104` | ABI layout and C examples pass on supported platforms |
| `CAN-1205` | Implement base assets, instances and sparse cell population | `CAN-1203` | Million-instance insert/query/remove stress test passes without geometry copies |
| `CAN-1206` | Implement camera conventions and CPU culling | `CAN-1205` | Results match brute force and asymmetric camera fixtures |
| `CAN-1207` | Implement runtime LOD, hysteresis and draw packets | `CAN-1206,CAN-0906` | Stable classifications and renderer-neutral packets pass capture tests |
| `CAN-1208` | Implement runtime wind domain manager | `CAN-0805,CAN-1205` | Fixed-step states and no-wind vectors match reference |
| `CAN-1209` | Implement billboard/impostor runtime selection | `CAN-0905,CAN-1207` | Atlas frame and crossfade selection pass camera sweeps |
| `CAN-1210` | Implement streaming, budgets and large-world origin shifts | `CAN-1205` | Eviction, cancellation and rebasing preserve instance identity |
| `CAN-1211` | Implement GPU-driven culling and indirect reference path | `CAN-1207,CAN-1103` | GPU visibility agrees with CPU and benchmark evidence meets profile |
| `CAN-1212` | Implement shadow-view and cascaded-shadow helpers | `CAN-1206` | Stable cascade fixtures and independent shadow LOD pass |
| `CAN-1213` | Implement optional terrain/population module | `CAN-1210` | Tile streaming, masks and deterministic placement integrate with forest cells |
| `CAN-1214` | Build runtime reference applications and viewer | `CAN-1211,CAN-1212` | Examples cover one asset, mixed forest, wind, LOD, metadata, collision and terrain |

### E13 — Engine and DCC integrations

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-1301` | Define shared integration manifest and shader test vectors | `CAN-1002,CAN-0803` | Every host consumes the same versioned semantics |
| `CAN-1302` | Implement Unity importer and materials | `CAN-1301,CAN-1004` | Built-in, URP and HDRP imports, reimports and builds pass |
| `CAN-1303` | Implement Unity runtime/terrain and indirect path | `CAN-1204,CAN-1302` | Wind, seasons, LOD and stress scene run outside editor |
| `CAN-1304` | Implement Unreal importer and material functions | `CAN-1301,CAN-1005` | Asset factory, reimport, LOD, impostor, collision and packaged build pass |
| `CAN-1305` | Implement Unreal population runtime component | `CAN-1204,CAN-1304` | Forest, wind and large-world map pass automated test |
| `CAN-1306` | Implement Godot add-on | `CAN-1301,CAN-1004` | glTF postprocess, materials, MultiMesh, wind and LOD scene pass |
| `CAN-1307` | Implement O3DE integration | `CAN-1301,CAN-1204` | Asset Builder, Atom materials and vegetation component pass |
| `CAN-1308` | Implement Blender add-on | `CAN-1004,CAN-1005,CAN-1006` | GUI/background import, materials, attributes, LOD and animation pass |
| `CAN-1309` | Implement Maya package | `CAN-1005,CAN-1006` | USD/Alembic import, materials and batch test pass |
| `CAN-1310` | Implement Houdini package | `CAN-1005,CAN-1006` | SOP/Solaris, point instances, MaterialX and PDG test pass |
| `CAN-1311` | Implement 3ds Max package | `CAN-1006,CAN-1007` | Supported static/cache workflow and batch smoke test pass |
| `CAN-1312` | Implement Cinema 4D package | `CAN-1005,CAN-1006` | Materials, MoGraph instances, LOD and cache smoke test pass |
| `CAN-1313` | Publish custom-engine starter kit | `CAN-1214,CAN-1301` | External sample consumes SDK via installed package with no repository-private headers |

### E14 — Rules, plugins, Python, library, and variants

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-1401` | Embed sandboxed Lua and implement Rules API | `CAN-0203,CAN-0302` | Permissions, instruction/memory limits and rollback tests pass |
| `CAN-1402` | Implement Rules editor controls and samples | `CAN-1401,CAN-1110` | Slider, enum, color, curve and action samples drive documented properties |
| `CAN-1403` | Define and implement native plugin C ABI | `CAN-0104,CAN-0105` | ABI negotiation, allocators, errors and sample capability load pass |
| `CAN-1404` | Implement isolated extension host | `CAN-1403` | Crash, hang, quota and permission tests preserve Modeler state |
| `CAN-1405` | Implement Python bindings and type stubs | `CAN-0203,CAN-0304,CAN-1002` | Examples and ownership/threading tests pass on supported Python versions |
| `CAN-1406` | Implement plugin discovery, trust and signing | `CAN-1403,CAN-0004` | Unknown project plugins never auto-execute; signer replacement is detected |
| `CAN-1407` | Implement local asset library and package format | `CAN-0209,CAN-0702` | Index rebuild, install, update, remove and quarantine work offline |
| `CAN-1408` | Implement dependency resolver and project lockfile UX | `CAN-1407,CAN-0209` | Exact versions reproduce and missing packages never silently substitute |
| `CAN-1409` | Implement optional remote registry protocol and client | `CAN-1407` | Search/download/signature/auth tests pass against reference server |
| `CAN-1410` | Implement variation schema, recipes and browser | `CAN-0301,CAN-1105` | Locked dimensions, reroll, correlation and batch export are deterministic |
| `CAN-1411` | Implement license/provenance records and policy hooks | `CAN-1407,CAN-0004` | Metadata survives package/project/export and organization rules are auditable |

### E15 — Hardening, documentation, and general availability

| ID | Deliverable | Depends on | Acceptance evidence |
|---|---|---|---|
| `CAN-1501` | Build full original golden asset corpus | `CAN-0500,CAN-0700,CAN-0800,CAN-0900` | Every major workflow has provenance, expectations and render baselines |
| `CAN-1502` | Complete parser and RPC fuzz campaigns | `CAN-0207,CAN-1203,CAN-1404` | No unresolved reproducible critical memory-safety finding at release cut |
| `CAN-1503` | Complete cross-platform determinism matrix | `CAN-0306` | All declared deterministic outputs meet exact or documented tolerance rules |
| `CAN-1504` | Complete performance and memory qualification | `CAN-0410,CAN-1211` | Reference benchmark manifests meet approved budgets with reproducible evidence |
| `CAN-1505` | Complete schema, API and ABI compatibility qualification | `CAN-0208,CAN-1204,CAN-1403` | Migration fixtures, ABI dumps and examples pass for release candidates |
| `CAN-1506` | Complete security review and threat-model verification | `CAN-1502,CAN-1406` | All high-risk controls have evidence and release-blocking findings are closed |
| `CAN-1507` | Publish user, API, file-format and integration documentation | `CAN-1000,CAN-1200,CAN-1300` | Examples execute and known limitations are explicit |
| `CAN-1508` | Run artist workflow validation | `CAN-1100,CAN-1300` | Representative game, VFX, scan and hero-mesh workflows complete without developer intervention |
| `CAN-1509` | Complete clean-room and third-party release audit | `CAN-0001,CAN-0004` | Provenance, licenses, notices, names and content are approved for distribution |
| `CAN-1510` | Generate release evidence and parity sign-off | `CAN-0006,CAN-1501,CAN-1509` | Every GA checklist item links to passing evidence or an explicitly declared limitation |

## Cross-epic integration gates

- `CAN-0200`: canonical document can load, mutate transactionally, undo, save, package, migrate and resolve locked assets.
- `CAN-0300`: deterministic headless evaluation produces immutable snapshots with incremental caching.
- `CAN-0500`: all documented generator classes satisfy shared contract tests and produce inspectable geometry.
- `CAN-0700`: material, texture, cutout, atlas, scan and baking workflows are integrated.
- `CAN-0800`: wind, growth, season and timeline data preview and export correctly.
- `CAN-0900`: continuous/discrete LOD, impostors, collision and optimization are integrated.
- `CAN-1000`: open-format games and VFX export, CLI and custom exporter paths are complete.
- `CAN-1100`: the desktop Modeler supports all authoring workflows without direct core-state mutation.
- `CAN-1200`: runtime compiler, loader, forest, culling, LOD, wind and reference renderer are complete.
- `CAN-1300`: engine and DCC compatibility matrix has passing automated evidence.

## Task completion rules

- A task must have production code, focused tests, diagnostics, documentation and a structured handoff.
- A geometry or runtime hot-path task must include a benchmark or justify why it is not performance sensitive.
- A parser, archive, RPC or script task must add negative tests and fuzz seeds.
- A public API or schema task must update compatibility baselines and examples.
- A UI task must exercise the same operation through the command/view-model layer in a headless test.
- A claimed parity task must attach evidence to the traceability database.
