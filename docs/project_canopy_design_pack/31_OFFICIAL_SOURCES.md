# Official requirements sources

## Scope and method

This source index records public documentation used to establish the functional baseline and standards strategy. Access date: **2026-07-19**.

These sources define observable capabilities, terminology and interchange requirements. They are not implementation source material. Project Canopy's algorithms, schemas, UI expression and native formats must remain independently designed.

## SpeedTree baseline

### Product and release state

- [ST-001 — Official SpeedTree product page](https://unity.com/products/speedtree)
  - Establishes the product grouping of Modeler, Library and Runtime SDK.
  - Describes procedural/manual modeling, adaptable assets, seasons, wind, dynamic LOD, photogrammetry, cutouts, game/VFX export and runtime platform scope.

- [ST-002 — SpeedTree Modeler release notes](https://docs.unity3d.com/speedtree-modeler/changelog/CHANGELOG.html)
  - Establishes **SpeedTree Modeler 10.2.0**, released **2026-04-30**, as the current public comparison point on the access date.
  - Also records 10.1 expanded painting/season work and 10.0 mesh helpers, vines, trim, shade pruning, custom packing, reference generators, Rules and package workflow.

- [ST-003 — What's new in SpeedTree 10.x](https://docs.unity3d.com/speedtree-modeler/manual/whats-new.html)
  - Describes major 10.x workflows: hero-mesh helpers, physics-based vines, trim brush, unified games/VFX modeler, shade pruning, custom mesh packing, reference generators and Lua Rules.

### Complete Modeler capability map

- [ST-010 — SpeedTree Modeler 10 documentation map](https://docs.unity3d.com/speedtree-modeler/manual/doc-map.html)
  - Primary inventory for UI, generators, nodes, properties, procedural techniques, freehand tools, vines, forces, seasons, collision, materials, photogrammetry, mesh helpers, Rules, lighting, animation, rendering, export and integrations.

- [ST-011 — SpeedTree Modeler 10 manual](https://docs.unity3d.com/speedtree-modeler/manual/)
  - Top-level official manual and version family.

- [ST-012 — Modeling approach](https://docs.unity3d.com/speedtree-modeler/manual/modeling-approach.html)
  - Requirements input for hierarchical procedural modeling combined with node-level manual editing.

- [ST-013 — Generation properties](https://docs.unity3d.com/speedtree-modeler/manual/generation-properties.html)
  - Requirements input for placement modes and shared generation controls.

- [ST-014 — Randomization](https://docs.unity3d.com/speedtree-modeler/manual/randomization.html)
  - Requirements input for generator-level randomization, seeds and variance behavior.

- [ST-015 — Branch generator properties](https://docs.unity3d.com/speedtree-modeler/manual/branch-generator-properties.html)
  - Requirements input for trunk/branch/root geometry, forces, shape and resolution controls.

- [ST-016 — Photogrammetry in SpeedTree](https://docs.unity3d.com/speedtree-modeler/manual/photogrammetry-in-speedtree.html)
  - Requirements input for Mesh, Mesh Converter, Mesh Detail, Stitch and Target workflows.

- [ST-017 — Hand-drawn generators](https://docs.unity3d.com/speedtree-modeler/manual/work-with-hand-drawn-generators-in-the-generation-editor.html)
  - Requirements input for hand-drawn targets, hierarchy behavior and template workflows.

- [ST-018 — Modeler hotkeys](https://docs.unity3d.com/speedtree-modeler/manual/hotkeys.html)
  - Requirements input for keyboard-accessible selection, visibility, editing, viewport and undo operations. Project Canopy does not copy the exact key map by default.

### Runtime SDK

- [ST-020 — SpeedTree Runtime SDK 10 manual](https://docs.unity3d.com/speedtree-runtime-sdk/manual/index.html)
  - Establishes custom-engine runtime integration as a separate SDK concern.

- [ST-021 — Runtime SDK organization](https://docs.unity3d.com/speedtree-runtime-sdk/manual/sdk-organization.html)
  - Requirements input for separation between Core, Forest, render interface and renderer libraries.

- [ST-022 — Modeler-to-Runtime workflow](https://docs.unity3d.com/speedtree-runtime-sdk/manual/sdk-workflow.html)
  - Requirements input for compiled runtime assets, customizable vertex/texture packing and shader-layout agreement.

- [ST-023 — Forest library introduction](https://docs.unity3d.com/speedtree-runtime-sdk/manual/forest-library-introduction.html)
  - Requirements input for dynamic forests, large populations, culling, LOD, wind and shadows without graphics-API dependence.

- [ST-024 — Culling and population structures](https://docs.unity3d.com/speedtree-runtime-sdk/manual/culling-and-population-structures.html)
  - Requirements input for cell-based forest culling, base assets, instances and per-instance transform state.

- [ST-025 — Wind overview](https://docs.unity3d.com/speedtree-runtime-sdk/manual/wind-overview.html)
  - Requirements input for CPU wind state plus shader deformation and packed per-vertex wind data.

- [ST-026 — Accessing metadata](https://docs.unity3d.com/speedtree-runtime-sdk/manual/accessing-metadata.html)
  - Requirements input for runtime metadata access.

- [ST-027 — Render Interface library](https://docs.unity3d.com/speedtree-runtime-sdk/manual/render-interface-library-introduction.html)
  - Requirements input for portable render abstraction while preserving custom-engine control.

## Open interchange standards

- [STD-001 — Khronos glTF 2.0 specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html)
  - Normative basis for portable real-time asset export.

- [STD-002 — OpenUSD documentation](https://openusd.org/release/index.html)
  - Normative implementation source for USD export and validation.

- [STD-003 — Introduction to USD](https://openusd.org/release/intro.html)
  - Establishes USD's role in scalable scene interchange and composition.

- [STD-004 — Alembic computer graphics project](https://github.com/alembic/alembic)
  - Reference implementation for non-procedural animated geometry interchange.

- [STD-005 — MaterialX](https://materialx.org/)
  - Open standard for portable material/look-development descriptions.

## Proposed implementation technologies

These sources validate that proposed technologies provide the required class of capability. Exact version and license approval remains a release task.

- [TECH-001 — Qt Widgets module](https://doc.qt.io/qt-6/qtwidgets-module.html)
- [TECH-002 — QMainWindow and dock widgets](https://doc.qt.io/qt-6/qmainwindow.html)
- [TECH-003 — `wgpu-native`](https://github.com/gfx-rs/wgpu-native)
- [TECH-004 — Lua 5.4 reference manual](https://www.lua.org/manual/5.4/)
- [TECH-005 — CMake documentation](https://cmake.org/cmake/help/latest/)
- [TECH-006 — nanobind documentation](https://nanobind.readthedocs.io/)

## Botanical and algorithmic research policy

Implementation agents should prefer primary literature and standards for:

- Plant architecture and phyllotaxis
- Allometric scaling
- Curve frames and sweep surfaces
- Robust triangulation and mesh processing
- Position-based or constrained-rod simulation
- Geometry simplification and perceptual LOD
- Image-based impostors
- Wind response and procedural turbulence
- Color management and PBR material semantics

Each nontrivial algorithm task records the specific papers or standards used. This pack does not prescribe one proprietary algorithm for any SpeedTree-observed feature.

## Baseline change control

When Unity publishes a new SpeedTree release:

1. Record the release number and exact release date.
2. Diff the official release notes and documentation map.
3. Add new or changed capabilities to the parity traceability database.
4. Classify them as required for the next Canopy parity release, deferred or outside scope.
5. Preserve the previous baseline so historical parity claims remain auditable.
