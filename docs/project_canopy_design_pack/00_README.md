# Project Canopy

## Clean-room vegetation authoring, animation, export, and runtime platform

Project Canopy is a working codename for an implementation-ready alternative to SpeedTree. The target is functional parity with the publicly documented SpeedTree Modeler 10.2.0, its game and VFX export workflows, and the core capabilities of the SpeedTree Runtime SDK 10.

This pack is written for a team of software-engineering AI agents supervised by human maintainers. It defines product scope, architecture, data contracts, implementation epics, acceptance tests, and clean-room constraints. It is not source code and does not contain copied proprietary algorithms, assets, schemas, shaders, file-format details, or user-interface artwork.

## Parity definition

“Parity” means that a user can complete the same classes of workflow and obtain equivalent open or industry-standard outputs:

- Procedural and manual vegetation modeling
- Branches, roots, leaves, fronds, vines, cards, caps, knots, shells, decals, stitches, imported meshes, and related detail geometry
- Curves, variance, randomization, node overrides, generation modes, geometry forces, collision handling, shape control, and scripting rules
- PBR materials, texture processing, cutout meshes, UV regions, atlases, photogrammetry workflows, material sets, and seasonal states
- Real-time and VFX wind
- Animated growth and dropped foliage
- Continuous and baked LOD, impostor billboards, shade pruning, collision proxies, and optimization
- Game and VFX export, custom vertex packing, command-line batch operation, custom exporters, and pipeline integration
- A runtime SDK for loading, instancing, culling, LOD selection, wind, billboards, forest rendering, metadata, collision, and optional terrain streaming
- Integration packages for major engines and DCC applications
- A searchable, license-aware vegetation asset library system

Parity does not require binary compatibility with proprietary `.spm`, `.st`, `.st9`, `.stsdk`, or `.ste` files. Project Canopy uses its own documented formats and industry-standard exports. Proprietary format support may be added only through a separately licensed, vendor-approved adapter.

## Pack map

Read these files first:

1. `01_EXECUTIVE_SPEC.md`
2. `02_PARITY_BASELINE.md`
3. `03_PRODUCT_REQUIREMENTS.md`
4. `04_SYSTEM_ARCHITECTURE.md`
5. `05_REPOSITORY_AND_BUILD.md`
6. `06_DATA_MODEL_AND_FILE_FORMATS.md`
7. `24_AGENT_IMPLEMENTATION_PLAYBOOK.md`
8. `25_EPICS_AND_TASKS.md`
9. `32_ACCEPTANCE_CHECKLIST.md`

Core technical specifications:

- `07_EVALUATION_ENGINE.md`
- `08_GENERATORS.md`
- `09_BRANCH_MESHING.md`
- `10_FOLIAGE_VINES_AND_DETAILS.md`
- `11_MANUAL_EDITING.md`
- `12_MATERIALS_TEXTURES_AND_PHOTOGRAMMETRY.md`
- `13_WIND_GROWTH_AND_SEASONS.md`
- `14_LOD_BILLBOARDS_AND_OPTIMIZATION.md`
- `15_VIEWPORT_AND_EDITOR_UX.md`
- `16_EXPORT_PIPELINE.md`
- `17_RUNTIME_SDK_AND_FOREST.md`
- `18_ENGINE_AND_DCC_INTEGRATIONS.md`
- `19_SCRIPTING_PLUGINS_AND_AUTOMATION.md`
- `20_ASSET_LIBRARY_AND_VARIATION.md`
- `21_PUBLIC_API_CONTRACTS.md`

Governance and delivery:

- `22_NONFUNCTIONAL_REQUIREMENTS.md`
- `23_TEST_STRATEGY.md`
- `26_RELEASE_GATES.md`
- `27_RISK_REGISTER.md`
- `28_CLEAN_ROOM_AND_LICENSING.md`
- `29_ARCHITECTURE_DECISIONS.md`
- `30_GLOSSARY.md`
- `31_OFFICIAL_SOURCES.md`
- `32_ACCEPTANCE_CHECKLIST.md`
- `33_AGENT_PROMPT_TEMPLATES.md`
- `34_BOOTSTRAP_BACKLOG.md`
- `35_PACK_MANIFEST.md`

## Product components

The repository is expected to produce six installable components:

- **Canopy Modeler**: cross-platform desktop authoring application
- **Canopy CLI**: deterministic headless generation, validation, rendering, and export
- **Canopy Core SDK**: model graph, evaluation, geometry, material, animation, and export APIs
- **Canopy Runtime SDK**: compact runtime format and forest-management API
- **Canopy Integrations**: engine importers, shaders, DCC add-ons, and examples
- **Canopy Library**: local or hosted catalog and asset packaging services

## Architectural principles

- Deterministic output for a given document, seed, dependency set, and target profile
- Non-destructive procedural modeling with explicit edit layers
- Stable identifiers and source-control-friendly project files
- Headless-first core with the GUI as a client
- Open formats by default and proprietary adapters as optional modules
- Renderer-independent runtime data
- Strict separation between authoring geometry and compact runtime geometry
- Versioned schemas with forward migration and no silent data loss
- Testable numerical algorithms with golden assets and perceptual image baselines
- Secure scripting with no filesystem or network capability by default

## Implementation language and platform baseline

- C++20 for core, modeler, exporters, and the public C++ SDK
- Stable C ABI for runtime and plugin interoperability
- Python bindings through nanobind or an equivalent thin binding layer
- Lua 5.4-compatible embedded scripting for rules and data packing
- CMake presets and a manifest-based dependency manager
- Qt 6 Widgets for the professional dockable desktop interface
- WebGPU through `wgpu-native` for the modeler viewport, with native engine shaders for integrations
- Windows, macOS, and Linux authoring builds
- Windows, macOS, Linux, current console targets through licensed platform branches, mobile, and XR runtime targets

## Naming

“Project Canopy” is a codename only. Complete trademark, domain, package-name, and app-store searches before public release.
